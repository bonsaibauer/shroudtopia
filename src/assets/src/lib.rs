mod transaction;

use std::{
    collections::{BTreeMap, HashMap, HashSet},
    ffi::{c_char, c_void},
    fs,
    path::{Path, PathBuf},
    ptr,
    slice,
    str,
    sync::{Arc, Mutex, OnceLock},
};

use kfc::{
    container::{KFCFile, KFCReader, KFCReaderOptions, KFCWriteOptions, KFCWriter},
    guid::{Guid, ResourceId},
    reflection::{LookupKey, TypeRegistry},
    resource::value::Value,
};

const OK: i32 = 0;
const INVALID_ARGUMENT: i32 = 1;
const NOT_FOUND: i32 = 3;
const CALLBACK_FAILED: i32 = 7;
const INTERNAL_ERROR: i32 = 8;

#[repr(C)]
pub struct StringView {
    pub data: *const c_char,
    pub size: usize,
}

#[repr(C)]
pub struct ResourceKey {
    pub struct_size: usize,
    pub guid: StringView,
    pub type_name: StringView,
    pub part: u32,
}

pub type Visitor = unsafe extern "C" fn(*const ResourceKey, *mut c_void) -> i32;

struct Engine {
    directory: PathBuf,
    stem: String,
    registry: Arc<TypeRegistry>,
    reference: Arc<KFCFile>,
    reader: Mutex<kfc::container::KFCCursor<KFCReader>>,
    overlays: BTreeMap<String, HashMap<ResourceId, Value>>,
    fields: BTreeMap<String, HashMap<ResourceId, BTreeMap<String, serde_json::Value>>>,
    _lease: fs::File,
}

static ENGINE: OnceLock<Mutex<Option<Engine>>> = OnceLock::new();

fn engine() -> &'static Mutex<Option<Engine>> {
    ENGINE.get_or_init(|| Mutex::new(None))
}

unsafe fn text(view: StringView) -> Result<String, i32> {
    if view.data.is_null() || view.size == 0 {
        return Err(INVALID_ARGUMENT);
    }
    let bytes = unsafe { slice::from_raw_parts(view.data.cast::<u8>(), view.size) };
    str::from_utf8(bytes).map(str::to_owned).map_err(|_| INVALID_ARGUMENT)
}

fn backup_path(path: &Path) -> PathBuf {
    PathBuf::from(format!("{}.shroudtopia.bak", path.display()))
}

fn prepare_backups(directory: &Path, stem: &str) -> Result<(PathBuf, PathBuf), String> {
    let kfc = directory.join(format!("{stem}.kfc"));
    let resources = directory.join(format!("{stem}.kfc_resources"));
    let kfc_backup = backup_path(&kfc);
    let resources_backup = backup_path(&resources);
    if !kfc.is_file() || !resources.is_file() {
        return Err("game KFC files are missing".into());
    }

    let current_version = KFCFile::get_version_tag(&kfc).map_err(|e| e.to_string())?;
    let backup_valid = kfc_backup.is_file()
        && resources_backup.is_file()
        && KFCFile::get_version_tag(&kfc_backup).ok().as_deref() == Some(current_version.as_str());
    if !backup_valid {
        fs::copy(&kfc, &kfc_backup).map_err(|e| e.to_string())?;
        fs::copy(&resources, &resources_backup).map_err(|e| e.to_string())?;
    }
    Ok((kfc_backup, resources_backup))
}

fn resource_id(key: &ResourceKey) -> Result<ResourceId, i32> {
    if key.struct_size < size_of::<ResourceKey>() {
        return Err(INVALID_ARGUMENT);
    }
    let guid = unsafe { text(StringView { data: key.guid.data, size: key.guid.size }) }?;
    let type_name = unsafe { text(StringView { data: key.type_name.data, size: key.type_name.size }) }?;
    ResourceId::parse(&guid, kfc::hash::fnv(&type_name), key.part).ok_or(INVALID_ARGUMENT)
}

fn base_value(state: &mut Engine, owner: &str, id: &ResourceId) -> Result<Option<Value>, String> {
    if let Some(value) = state.overlays.get(owner).and_then(|values| values.get(id)) {
        return Ok(Some(value.clone()));
    }
    for values in state.overlays.values().rev() {
        if let Some(value) = values.get(id) {
            return Ok(Some(value.clone()));
        }
    }
    let bytes = match state.reader.get_mut().map_err(|_| "asset reader lock poisoned")?
        .read_resource(id).map_err(|e| e.to_string())? {
        Some(bytes) => bytes,
        None => return Ok(None),
    };
    let ty = state.registry.get_by_hash(LookupKey::Qualified(id.type_hash()))
        .ok_or_else(|| format!("unknown resource type {:08x}", id.type_hash()))?;
    Value::from_bytes(state.registry.as_ref(), ty, &bytes)
        .map(Some).map_err(|e| e.to_string())
}

fn effective_value(state: &mut Engine, owner: &str, id: &ResourceId) -> Result<Option<Value>, String> {
    let Some(value) = base_value(state, owner, id)? else { return Ok(None); };
    let mut document = serde_json::to_value(value).map_err(|e| e.to_string())?;
    for resources in state.fields.values() {
        if let Some(fields) = resources.get(id) {
            for (path, value) in fields {
                let field = document.pointer_mut(path).ok_or("asset field no longer exists")?;
                *field = value.clone();
            }
        }
    }
    serde_json::from_value(document).map(Some).map_err(|e| e.to_string())
}

fn paths_overlap(a: &str, b: &str) -> bool {
    a == b || a.strip_prefix(b).is_some_and(|v| v.starts_with('/')) ||
        b.strip_prefix(a).is_some_and(|v| v.starts_with('/'))
}

fn copy_output(data: &[u8], buffer: *mut c_char, capacity: usize, required: *mut usize) -> i32 {
    if required.is_null() {
        return INVALID_ARGUMENT;
    }
    unsafe { *required = data.len() };
    if capacity == 0 {
        return OK;
    }
    if buffer.is_null() || capacity < data.len() {
        return INVALID_ARGUMENT;
    }
    unsafe { ptr::copy_nonoverlapping(data.as_ptr(), buffer.cast::<u8>(), data.len()) };
    OK
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn ShroudtopiaAssetsOpen(directory: StringView, stem: StringView) -> i32 {
    let directory = match unsafe { text(directory) } { Ok(v) => PathBuf::from(v), Err(e) => return e };
    let stem = match unsafe { text(stem) } { Ok(v) => v, Err(e) => return e };
    if stem.is_empty() || !stem.bytes().all(|b| b.is_ascii_alphanumeric() || b == b'_' || b == b'-') {
        return INVALID_ARGUMENT;
    }
    let mut slot = match engine().lock() { Ok(v) => v, Err(_) => return INTERNAL_ERROR };
    if slot.is_some() { return 2; }
    // Prevent another Shroudtopia process from publishing/recovering this set.
    use std::os::windows::fs::OpenOptionsExt;
    let lease = match fs::OpenOptions::new().create(true).truncate(false).read(true).write(true)
        .share_mode(0).open(directory.join(format!(".{stem}.shroudtopia.lock"))) {
        Ok(v) => v, Err(_) => return INTERNAL_ERROR,
    };
    // Recovery must precede reading either member of the file set.
    if transaction::recover(&directory, &stem).is_err() { return INTERNAL_ERROR; }
    let (kfc_backup, _) = match prepare_backups(&directory, &stem) { Ok(v) => v, Err(_) => return NOT_FOUND };
    let executable = directory.join(format!("{stem}.exe"));
    let mut registry = match TypeRegistry::load_from_executable(executable) { Ok(v) => v, Err(_) => return INTERNAL_ERROR };
    registry.version = match KFCFile::get_version_tag(&kfc_backup) { Ok(v) => v, Err(_) => return INTERNAL_ERROR };
    let reference = match KFCFile::from_path(&kfc_backup, false) { Ok(v) => Arc::new(v), Err(_) => return INTERNAL_ERROR };
    let reader = match KFCReader::new_with_options(&directory, &stem, KFCReaderOptions {
        kfc_extension: "kfc.shroudtopia.bak",
        resource_extension: "kfc_resources.shroudtopia.bak",
        ..Default::default()
    }).and_then(KFCReader::into_cursor) { Ok(v) => v, Err(_) => return INTERNAL_ERROR };
    *slot = Some(Engine {
        directory, stem, registry: Arc::new(registry), reference,
        reader: Mutex::new(reader), overlays: BTreeMap::new(), fields: BTreeMap::new(), _lease: lease,
    });
    OK
}

#[unsafe(no_mangle)]
pub extern "C" fn ShroudtopiaAssetsClose() {
    if let Ok(mut slot) = engine().lock() { *slot = None; }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn ShroudtopiaListAssets(
    type_name: StringView, visitor: Option<Visitor>, user_data: *mut c_void,
) -> i32 {
    let type_name = match unsafe { text(type_name) } { Ok(v) => v, Err(e) => return e };
    let visitor = match visitor { Some(v) => v, None => return INVALID_ARGUMENT };
    let slot = match engine().lock() { Ok(v) => v, Err(_) => return INTERNAL_ERROR };
    let state = match slot.as_ref() { Some(v) => v, None => return NOT_FOUND };
    let type_hash = kfc::hash::fnv(&type_name);
    let mut ids: HashSet<ResourceId> = state.reference.resources_by_type(type_hash).copied().collect();
    for overlay in state.overlays.values() {
        ids.extend(overlay.keys().filter(|id| id.type_hash() == type_hash).copied());
    }
    let mut ids: Vec<_> = ids.into_iter().collect();
    ids.sort_by_key(|id| (id.guid().to_string(), id.part_index()));
    // Never call mod code while holding the engine lock. Visitors may read,
    // replace, create or enumerate resources through this same API.
    drop(slot);
    for id in ids {
        let guid = id.guid().to_string();
        let key = ResourceKey {
            struct_size: size_of::<ResourceKey>(),
            guid: StringView { data: guid.as_ptr().cast(), size: guid.len() },
            type_name: StringView { data: type_name.as_ptr().cast(), size: type_name.len() },
            part: id.part_index(),
        };
        if unsafe { visitor(&key, user_data) } != OK { return CALLBACK_FAILED; }
    }
    OK
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn ShroudtopiaGetAssetJson(
    owner: StringView, key: *const ResourceKey, buffer: *mut c_char,
    capacity: usize, required: *mut usize,
) -> i32 {
    let owner = match unsafe { text(owner) } { Ok(v) => v, Err(e) => return e };
    if key.is_null() { return INVALID_ARGUMENT; }
    let id = match resource_id(unsafe { &*key }) { Ok(v) => v, Err(e) => return e };
    let mut slot = match engine().lock() { Ok(v) => v, Err(_) => return INTERNAL_ERROR };
    let state = match slot.as_mut() { Some(v) => v, None => return NOT_FOUND };
    let value = match effective_value(state, &owner, &id) { Ok(Some(v)) => v, Ok(None) => return NOT_FOUND, Err(_) => return INTERNAL_ERROR };
    let json = match serde_json::to_vec(&value) { Ok(v) => v, Err(_) => return INTERNAL_ERROR };
    copy_output(&json, buffer, capacity, required)
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn ShroudtopiaUpdateAssetJson(
    owner: StringView, key: *const ResourceKey, json: StringView,
) -> i32 {
    let owner = match unsafe { text(owner) } { Ok(v) => v, Err(e) => return e };
    if key.is_null() { return INVALID_ARGUMENT; }
    let id = match resource_id(unsafe { &*key }) { Ok(v) => v, Err(e) => return e };
    let json = match unsafe { text(json) } { Ok(v) => v, Err(e) => return e };
    let value: Value = match serde_json::from_str(&json) { Ok(v) => v, Err(_) => return INVALID_ARGUMENT };
    let mut slot = match engine().lock() { Ok(v) => v, Err(_) => return INTERNAL_ERROR };
    let state = match slot.as_mut() { Some(v) => v, None => return NOT_FOUND };
    if state.reference.resources().get(&id).is_none()
        && !state.overlays.values().any(|values| values.contains_key(&id)) { return NOT_FOUND; }
    let ty = match state.registry.get_by_hash(LookupKey::Qualified(id.type_hash())) {
        Some(v) => v, None => return NOT_FOUND,
    };
    if value.to_bytes(state.registry.as_ref(), ty).is_err() { return INVALID_ARGUMENT; }
    // Full-document replacement cannot safely merge independent owners.
    // Reject conflicts instead of silently discarding another mod's edits.
    if state.overlays.iter().any(|(other, values)| other != &owner && values.contains_key(&id))
        || state.fields.iter().any(|(other, values)| other != &owner && values.contains_key(&id)) {
        return 2; // RESULT_CONFLICT
    }
    if let Some(fields) = state.fields.get_mut(&owner) { fields.remove(&id); }
    state.overlays.entry(owner).or_default().insert(id, value);
    OK
}

/// Set an existing JSON-pointer field; no implicit creation or array append.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ShroudtopiaSetAssetFieldJson(
    owner: StringView, key: *const ResourceKey, path: StringView, json: StringView,
) -> i32 {
    let owner = match unsafe { text(owner) } { Ok(v) => v, Err(e) => return e };
    if key.is_null() { return INVALID_ARGUMENT; }
    let id = match resource_id(unsafe { &*key }) { Ok(v) => v, Err(e) => return e };
    let path = match unsafe { text(path) } { Ok(v) => v, Err(e) => return e };
    if !path.starts_with('/') { return INVALID_ARGUMENT; }
    let json = match unsafe { text(json) } { Ok(v) => v, Err(e) => return e };
    let value: serde_json::Value = match serde_json::from_str(&json) { Ok(v) => v, Err(_) => return INVALID_ARGUMENT };
    let mut slot = match engine().lock() { Ok(v) => v, Err(_) => return INTERNAL_ERROR };
    let state = match slot.as_mut() { Some(v) => v, None => return NOT_FOUND };
    if state.overlays.iter().any(|(other, values)| other != &owner && values.contains_key(&id)) {
        return 2;
    }
    for (other, resources) in &state.fields {
        if let Some(fields) = resources.get(&id) {
            if fields.keys().any(|existing| paths_overlap(existing, &path) && (other != &owner || existing != &path)) {
                return 2;
            }
        }
    }
    let current = match effective_value(state, &owner, &id) { Ok(Some(v)) => v, Ok(None) => return NOT_FOUND, Err(_) => return INTERNAL_ERROR };
    let mut document = match serde_json::to_value(current) { Ok(v) => v, Err(_) => return INTERNAL_ERROR };
    let field = match document.pointer_mut(&path) { Some(v) => v, None => return NOT_FOUND };
    *field = value.clone();
    let typed: Value = match serde_json::from_value(document) { Ok(v) => v, Err(_) => return INVALID_ARGUMENT };
    let ty = match state.registry.get_by_hash(LookupKey::Qualified(id.type_hash())) { Some(v) => v, None => return NOT_FOUND };
    if typed.to_bytes(state.registry.as_ref(), ty).is_err() { return INVALID_ARGUMENT; }
    state.fields.entry(owner).or_default().entry(id).or_default().insert(path, value);
    OK
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn ShroudtopiaCreateAssetJson(
    owner: StringView, type_name: StringView, json: StringView,
    visitor: Option<Visitor>, user_data: *mut c_void,
) -> i32 {
    let owner = match unsafe { text(owner) } { Ok(v) => v, Err(e) => return e };
    let type_name = match unsafe { text(type_name) } { Ok(v) => v, Err(e) => return e };
    let json = match unsafe { text(json) } { Ok(v) => v, Err(e) => return e };
    let visitor = match visitor { Some(v) => v, None => return INVALID_ARGUMENT };
    let value: Value = match serde_json::from_str(&json) { Ok(v) => v, Err(_) => return INVALID_ARGUMENT };
    let id = ResourceId::new(Guid::new(uuid::Uuid::new_v4().into_bytes()), kfc::hash::fnv(&type_name), 0);
    let guid = id.guid().to_string();
    let mut slot = match engine().lock() { Ok(v) => v, Err(_) => return INTERNAL_ERROR };
    let state = match slot.as_mut() { Some(v) => v, None => return NOT_FOUND };
    let ty = match state.registry.get_by_hash(LookupKey::Qualified(id.type_hash())) {
        Some(v) => v, None => return NOT_FOUND,
    };
    if value.to_bytes(state.registry.as_ref(), ty).is_err() { return INVALID_ARGUMENT; }
    state.overlays.entry(owner.clone()).or_default().insert(id, value);
    drop(slot);
    let key = ResourceKey {
        struct_size: size_of::<ResourceKey>(),
        guid: StringView { data: guid.as_ptr().cast(), size: guid.len() },
        type_name: StringView { data: type_name.as_ptr().cast(), size: type_name.len() },
        part: 0,
    };
    if unsafe { visitor(&key, user_data) } == OK { return OK; }
    // A failed creation callback must not leave an unacknowledged resource queued.
    if let Ok(mut slot) = engine().lock() {
        if let Some(state) = slot.as_mut() {
            if let Some(values) = state.overlays.get_mut(&owner) { values.remove(&id); }
            if let Some(values) = state.fields.get_mut(&owner) { values.remove(&id); }
        }
    }
    CALLBACK_FAILED
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn ShroudtopiaResetAssets(owner: StringView) -> i32 {
    let owner = match unsafe { text(owner) } { Ok(v) => v, Err(e) => return e };
    let mut slot = match engine().lock() { Ok(v) => v, Err(_) => return INTERNAL_ERROR };
    let state = match slot.as_mut() { Some(v) => v, None => return NOT_FOUND };
    let full = state.overlays.remove(&owner).is_some();
    let fields = state.fields.remove(&owner).is_some();
    if full || fields { OK } else { NOT_FOUND }
}

#[unsafe(no_mangle)]
pub extern "C" fn ShroudtopiaSaveAssets() -> i32 {
    let mut slot = match engine().lock() { Ok(v) => v, Err(_) => return INTERNAL_ERROR };
    let state = match slot.as_mut() { Some(v) => v, None => return NOT_FOUND };
    // Validate every resource before touching any active game file.
    let mut ids = HashSet::new();
    for values in state.overlays.values() { ids.extend(values.keys().copied()); }
    for values in state.fields.values() { ids.extend(values.keys().copied()); }
    let mut encoded = Vec::new();
    for id in ids {
        let value = match effective_value(state, "", &id) {
            Ok(Some(v)) => v, Ok(None) => return NOT_FOUND, Err(_) => return INTERNAL_ERROR,
        };
        let ty = match state.registry.get_by_hash(LookupKey::Qualified(id.type_hash())) {
            Some(v) => v, None => return NOT_FOUND,
        };
        let bytes = match value.to_bytes(state.registry.as_ref(), ty) { Ok(v) => v, Err(_) => return INVALID_ARGUMENT };
        encoded.push((id, bytes));
    }
    encoded.sort_by_key(|(id, _)| (id.type_hash(), id.guid().to_string(), id.part_index()));
    let stage = match transaction::begin(&state.directory, &state.stem) {
        Ok(v) => v, Err(_) => return INTERNAL_ERROR,
    };
    for suffix in ["kfc", "kfc_resources"] {
        let name = format!("{}.{}", state.stem, suffix);
        if fs::copy(backup_path(&state.directory.join(&name)), stage.join(&name)).is_err() {
            return INTERNAL_ERROR;
        }
    }
    let mut writer = match KFCWriter::new_incremental_with_options(
        &stage, &state.stem, state.reference.clone(), state.registry.clone(),
        KFCWriteOptions { overwrite_containers: true, ..Default::default() },
    ) { Ok(v) => v, Err(_) => return INTERNAL_ERROR };
    for (id, bytes) in encoded {
        if writer.write_resource(&id, &bytes).is_err() { return INTERNAL_ERROR; }
    }
    if writer.finalize().is_err() { return INTERNAL_ERROR; }
    if KFCFile::from_path(stage.join(format!("{}.kfc", state.stem)), false).is_err() {
        return INTERNAL_ERROR;
    }
    match transaction::commit(&state.directory, &state.stem) { Ok(()) => OK, Err(_) => INTERNAL_ERROR }
}

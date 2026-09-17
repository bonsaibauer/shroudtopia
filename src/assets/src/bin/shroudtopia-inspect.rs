use serde_json::{json, Value};
use shroudtopia::{
    ResourceKey, ShroudtopiaAssetsClose, ShroudtopiaAssetsOpen, ShroudtopiaCreateAssetJson,
    ShroudtopiaGetAssetJson, ShroudtopiaListAssets, ShroudtopiaResetAssets,
    ShroudtopiaSaveAssets, ShroudtopiaSetAssetFieldJson, ShroudtopiaUpdateAssetJson, StringView,
};
use std::{env, ffi::{c_char, c_void}, io::{self, BufRead, Write}, ptr, slice};

const RESULT_OK: i32 = 0;

#[derive(Clone)]
struct OwnedKey { guid: String, type_name: String, part: u32 }

fn view(value: &str) -> StringView {
    StringView { data: value.as_ptr().cast::<c_char>(), size: value.len() }
}

unsafe fn view_text(value: &StringView) -> String {
    if value.data.is_null() { return String::new(); }
    String::from_utf8_lossy(unsafe { slice::from_raw_parts(value.data.cast::<u8>(), value.size) }).into_owned()
}

unsafe extern "C" fn collect_key(key: *const ResourceKey, data: *mut c_void) -> i32 {
    if key.is_null() || data.is_null() { return 1; }
    let key = unsafe { &*key };
    let keys = unsafe { &mut *data.cast::<Vec<OwnedKey>>() };
    keys.push(OwnedKey {
        guid: unsafe { view_text(&key.guid) },
        type_name: unsafe { view_text(&key.type_name) },
        part: key.part,
    });
    RESULT_OK
}

fn with_key<T>(args: &Value, action: impl FnOnce(*const ResourceKey) -> T) -> Result<T, String> {
    let key = args.get("asset").unwrap_or(args);
    let guid = key.get("guid").and_then(Value::as_str).ok_or("asset.guid is required")?;
    let type_name = key.get("type_name").and_then(Value::as_str).ok_or("asset.type_name is required")?;
    let part = key.get("part").and_then(Value::as_u64).unwrap_or(0) as u32;
    let native = ResourceKey { struct_size: size_of::<ResourceKey>(), guid: view(guid), type_name: view(type_name), part };
    Ok(action(&native))
}

fn owner(args: &Value) -> &str {
    args.get("owner_id").and_then(Value::as_str).unwrap_or("sandbox")
}

fn text_arg<'a>(args: &'a Value, name: &str) -> Result<&'a str, String> {
    args.get(name).and_then(Value::as_str).ok_or_else(|| format!("{name} is required"))
}

fn json_arg(args: &Value) -> Result<String, String> {
    let value = args.get("json").ok_or("json is required")?;
    Ok(match value { Value::String(text) => text.clone(), other => serde_json::to_string(other).map_err(|e| e.to_string())? })
}

fn result_name(code: i32) -> &'static str {
    match code { 0 => "RESULT_OK", 1 => "RESULT_INVALID_ARGUMENT", 2 => "RESULT_CONFLICT", 3 => "RESULT_NOT_FOUND", 4 => "RESULT_VERSION_MISMATCH", 5 => "RESULT_PERMISSION_DENIED", 6 => "RESULT_NOT_AVAILABLE", 7 => "RESULT_CALLBACK_FAILED", _ => "RESULT_INTERNAL_ERROR" }
}

fn response(code: i32, outputs: Value) -> Value {
    json!({ "result": result_name(code), "result_code": code, "outputs": outputs })
}

fn invoke(function: &str, args: &Value, allow_write: bool) -> Result<Value, String> {
    let write = matches!(function, "update_asset" | "set_asset_field" | "create_asset" | "reset_assets" | "save_assets");
    if write && !allow_write { return Ok(response(5, json!({ "message": "Restart the local host with --allow-write." }))); }
    let value = unsafe { match function {
        "list_assets" => {
            let type_name = text_arg(args, "type_name")?;
            let mut keys = Vec::<OwnedKey>::new();
            let code = ShroudtopiaListAssets(view(type_name), Some(collect_key), (&mut keys as *mut Vec<OwnedKey>).cast());
            response(code, json!({ "assets": keys.into_iter().map(|key| json!({ "guid": key.guid, "type_name": key.type_name, "part": key.part })).collect::<Vec<_>>() }))
        }
        "get_asset" => {
            let owner = owner(args);
            let (code, document) = with_key(args, |key| {
                let mut required = 0usize;
                let first = ShroudtopiaGetAssetJson(view(owner), key, ptr::null_mut(), 0, &mut required);
                if first != RESULT_OK { return (first, Value::Null); }
                let mut bytes = vec![0u8; required];
                let second = ShroudtopiaGetAssetJson(view(owner), key, bytes.as_mut_ptr().cast(), bytes.len(), &mut required);
                let document = if second == RESULT_OK { serde_json::from_slice(&bytes).unwrap_or(Value::Null) } else { Value::Null };
                (second, document)
            })?;
            response(code, json!({ "json": document }))
        }
        "update_asset" => {
            let owner = owner(args); let document = json_arg(args)?;
            let code = with_key(args, |key| ShroudtopiaUpdateAssetJson(view(owner), key, view(&document)))?;
            response(code, json!({}))
        }
        "set_asset_field" => {
            let owner = owner(args); let path = text_arg(args, "path")?; let document = json_arg(args)?;
            let code = with_key(args, |key| ShroudtopiaSetAssetFieldJson(view(owner), key, view(path), view(&document)))?;
            response(code, json!({}))
        }
        "create_asset" => {
            let owner = owner(args); let type_name = text_arg(args, "type_name")?; let document = json_arg(args)?;
            let mut keys = Vec::<OwnedKey>::new();
            let code = ShroudtopiaCreateAssetJson(view(owner), view(type_name), view(&document), Some(collect_key), (&mut keys as *mut Vec<OwnedKey>).cast());
            response(code, json!({ "asset": keys.into_iter().next().map(|key| json!({ "guid": key.guid, "type_name": key.type_name, "part": key.part })) }))
        }
        "reset_assets" => response(ShroudtopiaResetAssets(view(owner(args))), json!({})),
        "save_assets" => {
            let code = ShroudtopiaSaveAssets();
            response(code, json!({ "saved": code == RESULT_OK }))
        }
        _ => response(6, json!({ "message": "This function requires the in-game runtime." })),
    }};
    Ok(value)
}

fn argument(name: &str) -> Result<String, String> {
    let position = env::args().position(|item| item == name).ok_or_else(|| format!("{name} is required"))?;
    env::args().nth(position + 1).ok_or_else(|| format!("{name} requires a value"))
}

fn main() -> Result<(), String> {
    let directory = argument("--game-dir")?;
    let stem = env::args().position(|item| item == "--stem").and_then(|index| env::args().nth(index + 1)).unwrap_or_else(|| "enshrouded".into());
    let allow_write = env::args().any(|item| item == "--allow-write");
    let opened = unsafe { ShroudtopiaAssetsOpen(view(&directory), view(&stem)) };
    if opened != RESULT_OK { return Err(format!("Could not open {stem}.exe/KFC data in {directory}: {}", result_name(opened))); }
    let stdin = io::stdin();
    let mut stdout = io::stdout().lock();
    for line in stdin.lock().lines() {
        let output = match line {
            Ok(line) => match serde_json::from_str::<Value>(&line) {
                Ok(request) => {
                    let function = request.get("function").and_then(Value::as_str).unwrap_or("");
                    let args = request.get("arguments").unwrap_or(&Value::Null);
                    invoke(function, args, allow_write).unwrap_or_else(|error| json!({ "result": "RESULT_INVALID_ARGUMENT", "result_code": 1, "error": error }))
                }
                Err(error) => json!({ "result": "RESULT_INVALID_ARGUMENT", "result_code": 1, "error": error.to_string() }),
            },
            Err(error) => json!({ "result": "RESULT_INTERNAL_ERROR", "result_code": 8, "error": error.to_string() }),
        };
        serde_json::to_writer(&mut stdout, &output).map_err(|e| e.to_string())?;
        stdout.write_all(b"\n").map_err(|e| e.to_string())?;
        stdout.flush().map_err(|e| e.to_string())?;
    }
    ShroudtopiaAssetsClose();
    Ok(())
}

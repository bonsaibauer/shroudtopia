//! Recoverable publication of a generated KFC file set. The caller must keep
//! the game's resource reader gated; this is not an atomic hot-reload mechanism.
use std::{fs, io, path::{Path, PathBuf}};

fn sync(path: &Path) -> io::Result<()> {
    fs::OpenOptions::new().write(true).open(path)?.sync_all()
}

pub fn directory(root: &Path, stem: &str) -> PathBuf {
    root.join(format!(".{stem}.shroudtopia-stage"))
}

fn valid_name(stem: &str, name: &str) -> bool {
    name == format!("{stem}.kfc") || name == format!("{stem}.kfc_resources") ||
        name.strip_prefix(&format!("{stem}_")).and_then(|v| v.strip_suffix(".dat"))
            .is_some_and(|v| !v.is_empty() && v.bytes().all(|b| b.is_ascii_digit()))
}

fn invalid() -> io::Error { io::Error::other("invalid asset transaction journal") }

fn entries(stage: &Path, stem: &str) -> io::Result<Vec<(String, bool)>> {
    let entries: Vec<(String, bool)> = serde_json::from_slice(&fs::read(stage.join("pending.json"))?)
        .map_err(|_| invalid())?;
    if entries.iter().any(|(name, _)| !valid_name(stem, name)) { return Err(invalid()); }
    Ok(entries)
}

// Only remove flat files inside our dedicated workspace; reject unexpected folders.
fn clean(stage: &Path) -> io::Result<()> {
    if !stage.exists() { return Ok(()); }
    for entry in fs::read_dir(stage)? {
        let entry = entry?;
        if !entry.file_type()?.is_file() { return Err(invalid()); }
        fs::remove_file(entry.path())?;
    }
    fs::remove_dir(stage)
}

pub fn recover(root: &Path, stem: &str) -> io::Result<()> {
    let stage = directory(root, stem);
    if stage.join("pending.json").exists() {
        for (name, existed) in entries(&stage, stem)? {
            let target = root.join(&name);
            if existed {
                let restore = stage.join(format!("{name}.restore"));
                fs::copy(stage.join(format!("{name}.rollback")), &restore)?;
                sync(&restore)?;
                fs::rename(restore, target)?;
            } else if target.exists() {
                fs::remove_file(target)?;
            }
        }
        // A completed rollback must not require backups after cleanup starts.
        fs::rename(stage.join("pending.json"), stage.join("completed.json"))?;
    }
    clean(&stage)
}

pub fn begin(root: &Path, stem: &str) -> io::Result<PathBuf> {
    recover(root, stem)?;
    let stage = directory(root, stem);
    fs::create_dir(&stage)?;
    Ok(stage)
}

fn prepare(root: &Path, stem: &str) -> io::Result<Vec<(String, bool)>> {
    let stage = directory(root, stem);
    let mut files = Vec::new();
    for entry in fs::read_dir(&stage)? {
        let entry = entry?;
        let name = entry.file_name().into_string().map_err(|_| invalid())?;
        if !entry.file_type()?.is_file() || !valid_name(stem, &name) { return Err(invalid()); }
        files.push(name);
    }
    // Publish the index last. Readers must still be gated for the whole set.
    files.sort_by_key(|name| (name.ends_with(".kfc"), name.clone()));
    let mut records = Vec::new();
    for name in files {
        sync(&stage.join(&name))?;
        let target = root.join(&name);
        let existed = target.exists();
        if existed {
            let backup = stage.join(format!("{name}.rollback"));
            fs::copy(&target, &backup)?;
            sync(&backup)?;
        }
        records.push((name, existed));
    }
    let journal = stage.join("journal.tmp");
    fs::write(&journal, serde_json::to_vec(&records).map_err(|_| invalid())?)?;
    sync(&journal)?;
    fs::rename(journal, stage.join("pending.json"))?;
    Ok(records)
}

pub fn commit(root: &Path, stem: &str) -> io::Result<()> {
    let stage = directory(root, stem);
    let records = prepare(root, stem)?;
    let publish = (|| {
        for (name, _) in records { fs::rename(stage.join(&name), root.join(&name))?; }
        fs::rename(stage.join("pending.json"), stage.join("completed.json"))?;
        Ok(())
    })();
    if let Err(error) = publish {
        // Keep the journal if recovery also fails, so the next open retries it.
        recover(root, stem)?;
        return Err(error);
    }
    // Publication succeeded. A leftover completed workspace can be cleaned on open.
    let _ = clean(&stage);
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    fn fixture() -> PathBuf {
        let root = std::env::temp_dir().join(format!("shroudtopia-{}", uuid::Uuid::new_v4()));
        fs::create_dir(&root).unwrap();
        fs::write(root.join("game.kfc"), b"old-index").unwrap();
        fs::write(root.join("game.kfc_resources"), b"old-data").unwrap();
        root
    }
    #[test]
    fn interrupted_publish_restores_complete_previous_set() {
        let root = fixture();
        let stage = begin(&root, "game").unwrap();
        fs::write(stage.join("game.kfc"), b"new-index").unwrap();
        fs::write(stage.join("game.kfc_resources"), b"new-data").unwrap();
        fs::write(stage.join("game_123.dat"), b"new-content").unwrap();
        prepare(&root, "game").unwrap();
        fs::rename(stage.join("game.kfc_resources"), root.join("game.kfc_resources")).unwrap();
        fs::rename(stage.join("game_123.dat"), root.join("game_123.dat")).unwrap();
        recover(&root, "game").unwrap();
        assert_eq!(fs::read(root.join("game.kfc")).unwrap(), b"old-index");
        assert_eq!(fs::read(root.join("game.kfc_resources")).unwrap(), b"old-data");
        assert!(!root.join("game_123.dat").exists());
        fs::remove_file(root.join("game.kfc")).unwrap();
        fs::remove_file(root.join("game.kfc_resources")).unwrap();
        fs::remove_dir(root).unwrap();
    }
    #[test]
    fn commit_publishes_and_removes_journal() {
        let root = fixture();
        let stage = begin(&root, "game").unwrap();
        fs::write(stage.join("game.kfc"), b"new-index").unwrap();
        fs::write(stage.join("game.kfc_resources"), b"new-data").unwrap();
        commit(&root, "game").unwrap();
        assert_eq!(fs::read(root.join("game.kfc")).unwrap(), b"new-index");
        assert_eq!(fs::read(root.join("game.kfc_resources")).unwrap(), b"new-data");
        assert!(!stage.exists());
        fs::remove_file(root.join("game.kfc")).unwrap();
        fs::remove_file(root.join("game.kfc_resources")).unwrap();
        fs::remove_dir(root).unwrap();
    }
}

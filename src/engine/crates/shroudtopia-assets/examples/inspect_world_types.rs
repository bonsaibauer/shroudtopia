//! Offline developer diagnostic: extract reflection to stdout; no game writes.
use kfc::reflection::TypeRegistry;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let executable = std::env::args_os().nth(1).ok_or("Expected game EXE path")?;
    let registry = TypeRegistry::load_from_executable(executable)?;
    serde_json::to_writer_pretty(std::io::stdout().lock(), &registry)?;
    Ok(())
}

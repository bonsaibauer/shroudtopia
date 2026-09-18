use kfc::reflection::{LookupKey, TypeRegistry};
use std::{env, path::PathBuf};

fn main() {
    let mut args=env::args_os();
    let _=args.next();
    let executable=PathBuf::from(args.next().expect("executable path"));
    let needle=args.next().expect("qualified type name").to_string_lossy().into_owned();
    let registry=TypeRegistry::load_from_executable(executable).expect("reflection parse");
    let ty=registry.get_by_name(LookupKey::Qualified(&needle)).expect("type not found");
    println!("name={} impact={} size={} align={} impact_hash={:08x} qualified_hash={:08x} internal_hash={:08x}",
        ty.qualified_name,ty.impact_name,ty.size,ty.alignment,ty.impact_hash,ty.qualified_hash,ty.internal_hash);
    if env::var_os("BRIEF").is_some() { return; }
    for field in ty.struct_fields.values() {
        let field_type=registry.get(field.r#type).expect("field type");
        println!("field {} +0x{:x}: {} size={} primitive={:?}",field.name,field.data_offset,
            field_type.qualified_name,field_type.size,field_type.primitive_type);
    }
}

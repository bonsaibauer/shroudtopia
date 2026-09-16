import argparse
import ctypes
import json
from pathlib import Path


class StringView(ctypes.Structure):
    _fields_ = [("data", ctypes.c_char_p), ("size", ctypes.c_size_t)]


class ResourceKey(ctypes.Structure):
    _fields_ = [
        ("struct_size", ctypes.c_size_t),
        ("guid", StringView),
        ("type_name", StringView),
        ("part", ctypes.c_uint32),
    ]


def view(value: str) -> tuple[StringView, bytes]:
    encoded = value.encode("utf-8")
    return StringView(encoded, len(encoded)), encoded


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("engine", type=Path)
    parser.add_argument("game_directory", type=Path)
    parser.add_argument("stem")
    parser.add_argument("--roundtrip", action="store_true")
    args = parser.parse_args()
    if args.roundtrip and not args.game_directory.resolve().is_relative_to(Path(__file__).resolve().parents[1] / "build"):
        parser.error("--roundtrip requires an isolated fixture under this repository's build directory")

    library = ctypes.CDLL(str(args.engine.resolve()))
    callback_type = ctypes.CFUNCTYPE(ctypes.c_int32, ctypes.POINTER(ResourceKey), ctypes.c_void_p)
    library.ShroudtopiaAssetsOpen.argtypes = [StringView, StringView]
    library.ShroudtopiaAssetsOpen.restype = ctypes.c_int32
    library.ShroudtopiaAssetsVisit.argtypes = [StringView, callback_type, ctypes.c_void_p]
    library.ShroudtopiaAssetsVisit.restype = ctypes.c_int32
    library.ShroudtopiaAssetsReadJson.argtypes = [
        StringView, ctypes.POINTER(ResourceKey), ctypes.c_void_p, ctypes.c_size_t,
        ctypes.POINTER(ctypes.c_size_t),
    ]
    library.ShroudtopiaAssetsReadJson.restype = ctypes.c_int32
    library.ShroudtopiaAssetsReplaceJson.argtypes = [StringView, ctypes.POINTER(ResourceKey), StringView]
    library.ShroudtopiaAssetsReplaceJson.restype = ctypes.c_int32
    library.ShroudtopiaAssetsFlush.restype = ctypes.c_int32

    directory, directory_bytes = view(str(args.game_directory.resolve()))
    stem, stem_bytes = view(args.stem)
    if library.ShroudtopiaAssetsOpen(directory, stem) != 0:
        raise RuntimeError("asset engine could not open the game files")

    found: list[tuple[str, str, int]] = []
    callback_errors = []
    owner, owner_bytes = view("shroudtopia.smoke")

    @callback_type
    def collect(key_pointer, _):
        # Regression: the old visitor held the engine lock and deadlocked here.
        required = ctypes.c_size_t()
        result = library.ShroudtopiaAssetsReadJson(owner, key_pointer, None, 0, ctypes.byref(required))
        if result != 0:
            callback_errors.append(f"read inside visitor returned {result}")
            return 7
        buffer = ctypes.create_string_buffer(required.value)
        result = library.ShroudtopiaAssetsReadJson(owner, key_pointer, buffer, len(buffer), ctypes.byref(required))
        if result != 0:
            callback_errors.append(f"read inside visitor returned {result}")
            return 7
        replacement, keep = view(buffer.raw.decode("utf-8"))
        if library.ShroudtopiaAssetsReplaceJson(owner, key_pointer, replacement) != 0:
            callback_errors.append("replace inside visitor failed")
            return 7
        key = key_pointer.contents
        found.append((
            ctypes.string_at(key.guid.data, key.guid.size).decode(),
            ctypes.string_at(key.type_name.data, key.type_name.size).decode(),
            key.part,
        ))
        return 0

    type_name, type_name_bytes = view("keen::RecipeRegistryResource")
    if library.ShroudtopiaAssetsVisit(type_name, collect, None) != 0 or not found:
        raise RuntimeError(f"resource visitor failed: {callback_errors}")

    guid, guid_bytes = view(found[0][0])
    qualified_type, qualified_type_bytes = view(found[0][1])
    key = ResourceKey(ctypes.sizeof(ResourceKey), guid, qualified_type, found[0][2])
    owner, owner_bytes = view("shroudtopia.smoke")
    required = ctypes.c_size_t()
    if library.ShroudtopiaAssetsReadJson(owner, ctypes.byref(key), None, 0, ctypes.byref(required)) != 0:
        raise RuntimeError("resource size query failed")
    buffer = ctypes.create_string_buffer(required.value)
    if library.ShroudtopiaAssetsReadJson(owner, ctypes.byref(key), buffer, len(buffer), ctypes.byref(required)) != 0:
        raise RuntimeError("resource read failed")
    document = json.loads(buffer.raw.decode("utf-8"))
    if "recipes" not in document:
        raise RuntimeError("RecipeRegistryResource has no recipes field")

    library.ShroudtopiaAssetsCreateJson.argtypes = [StringView, StringView, StringView, callback_type, ctypes.c_void_p]
    library.ShroudtopiaAssetsCreateJson.restype = ctypes.c_int32
    document_view, document_bytes = view(json.dumps(document, separators=(",", ":")))
    other_owner, other_bytes = view("shroudtopia.other")
    if library.ShroudtopiaAssetsReplaceJson(other_owner, ctypes.byref(key), document_view) != 2:
        raise RuntimeError("conflicting full replacement was not rejected")
    created = []
    @callback_type
    def check_created(key_pointer, _):
        required = ctypes.c_size_t()
        result = library.ShroudtopiaAssetsReadJson(owner, key_pointer, None, 0, ctypes.byref(required))
        if result != 0: return 7
        result = library.ShroudtopiaAssetsReplaceJson(owner, key_pointer, document_view)
        if result != 0: return 7
        created.append(True)
        return 0
    if library.ShroudtopiaAssetsCreateJson(owner, type_name, document_view, check_created, None) != 0 or not created:
        raise RuntimeError("create callback could not read and replace its resource")
    # Drop in-memory test overlays before a disk roundtrip of the original document.
    library.ShroudtopiaAssetsDiscard.argtypes = [StringView]
    if library.ShroudtopiaAssetsDiscard(owner) != 0:
        raise RuntimeError("discard failed")

    library.ShroudtopiaAssetsSetFieldJson.argtypes = [StringView, ctypes.POINTER(ResourceKey), StringView, StringView]
    library.ShroudtopiaAssetsSetFieldJson.restype = ctypes.c_int32
    leaves = []
    def collect_numbers(value, path=""):
        if isinstance(value, dict):
            for name, item in value.items():
                collect_numbers(item, path + "/" + name.replace("~", "~0").replace("/", "~1"))
        elif isinstance(value, list):
            for index, item in enumerate(value): collect_numbers(item, path + "/" + str(index))
        elif isinstance(value, int) and not isinstance(value, bool) and value > 1:
            leaves.append((path, value))
    collect_numbers(document)
    if len(leaves) < 2: raise RuntimeError("fixture needs two numeric fields")
    def set_field(mod_owner, path, value):
        path_view, path_bytes = view(path)
        value_view, value_bytes = view(json.dumps(value))
        return library.ShroudtopiaAssetsSetFieldJson(mod_owner, ctypes.byref(key), path_view, value_view)
    path_a, value_a = leaves[0]
    path_b, value_b = leaves[1]
    if set_field(owner, path_a, value_a - 1) != 0: raise RuntimeError("first field edit failed")
    if set_field(other_owner, path_b, value_b - 1) != 0: raise RuntimeError("disjoint field edit failed")
    if set_field(other_owner, path_a, value_a) != 2: raise RuntimeError("same-field conflict accepted")
    if set_field(owner, "/nonexistent_shroudtopia_field", 1) != 3: raise RuntimeError("unknown field accepted")
    if set_field(owner, path_a, {"invalid": True}) != 1: raise RuntimeError("wrong field type accepted")
    def read_document():
        needed = ctypes.c_size_t()
        if library.ShroudtopiaAssetsReadJson(owner, ctypes.byref(key), None, 0, ctypes.byref(needed)) != 0:
            raise RuntimeError("field read size failed")
        data = ctypes.create_string_buffer(needed.value)
        if library.ShroudtopiaAssetsReadJson(owner, ctypes.byref(key), data, len(data), ctypes.byref(needed)) != 0:
            raise RuntimeError("field read failed")
        return json.loads(data.raw)
    def at_path(doc, path):
        for token in path.split("/")[1:]:
            token = token.replace("~1", "/").replace("~0", "~")
            doc = doc[int(token)] if isinstance(doc, list) else doc[token]
        return doc
    combined = read_document()
    if at_path(combined, path_a) != value_a - 1 or at_path(combined, path_b) != value_b - 1:
        raise RuntimeError("field edits did not compose")
    library.ShroudtopiaAssetsDiscard(owner)
    remaining = read_document()
    if at_path(remaining, path_a) != value_a or at_path(remaining, path_b) != value_b - 1:
        raise RuntimeError("discard did not preserve the other owner's field")
    library.ShroudtopiaAssetsDiscard(other_owner)
    if read_document() != document: raise RuntimeError("discard did not restore original data")

    if args.roundtrip:
        serialized = json.dumps(document, separators=(",", ":"))
        replacement, replacement_bytes = view(serialized)
        if library.ShroudtopiaAssetsReplaceJson(owner, ctypes.byref(key), replacement) != 0:
            raise RuntimeError("resource replacement failed")
        if library.ShroudtopiaAssetsFlush() != 0:
            raise RuntimeError("resource flush failed")
        library.ShroudtopiaAssetsClose()

        for suffix in (".kfc.shroudtopia.bak", ".kfc_resources.shroudtopia.bak"):
            (args.game_directory / f"{args.stem}{suffix}").unlink()
        if library.ShroudtopiaAssetsOpen(directory, stem) != 0:
            raise RuntimeError("roundtrip files could not be reopened")
        required = ctypes.c_size_t()
        if library.ShroudtopiaAssetsReadJson(owner, ctypes.byref(key), None, 0, ctypes.byref(required)) != 0:
            raise RuntimeError("roundtrip size query failed")
        buffer = ctypes.create_string_buffer(required.value)
        if library.ShroudtopiaAssetsReadJson(owner, ctypes.byref(key), buffer, len(buffer), ctypes.byref(required)) != 0:
            raise RuntimeError("roundtrip read failed")
        if json.loads(buffer.raw.decode("utf-8")) != document:
            raise RuntimeError("roundtrip changed the resource document")

    library.ShroudtopiaAssetsClose()
    print(f"Asset engine passed for {args.stem}: {len(found)} registry resource(s), {len(document['recipes'])} recipes")


if __name__ == "__main__":
    main()

"""Export construction assets from an isolated fixture, never the live installation."""
import argparse
import ctypes as c
import json
from pathlib import Path

class View(c.Structure):
    _fields_ = [('data', c.c_void_p), ('size', c.c_size_t)]

class Key(c.Structure):
    _fields_ = [('struct_size', c.c_size_t), ('guid', View), ('type_name', View), ('part', c.c_uint32)]

def view(text):
    raw = text.encode('utf-8')
    storage = c.create_string_buffer(raw)
    return View(c.cast(storage, c.c_void_p), len(raw)), storage

def decode(value):
    return c.string_at(value.data, value.size).decode('utf-8')

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('engine', type=Path)
    parser.add_argument('fixture', type=Path)
    args = parser.parse_args()
    fixture = args.fixture.resolve()
    root = Path(__file__).resolve().parents[1] / 'build'
    if not fixture.is_relative_to(root) or fixture == root:
        parser.error('fixture must be a subdirectory of this repository build directory')
    dll = c.CDLL(str(args.engine.resolve()))
    visitor = c.CFUNCTYPE(c.c_int32, c.POINTER(Key), c.c_void_p)
    dll.ShroudtopiaAssetsOpen.argtypes = [View, View]
    dll.ShroudtopiaListAssets.argtypes = [View, visitor, c.c_void_p]
    dll.ShroudtopiaGetAssetJson.argtypes = [View, c.POINTER(Key), c.c_void_p, c.c_size_t, c.POINTER(c.c_size_t)]
    directory, keep = view(str(fixture))
    stem, keep_stem = view('enshrouded')
    result = dll.ShroudtopiaAssetsOpen(directory, stem)
    if result: raise RuntimeError(f'open: {result}')
    output = fixture / 'building-assets'
    output.mkdir(exist_ok=True)
    owner, keep_owner = view('shroudtopia.building.inspect')
    errors = []
    counts = {}
    @visitor
    def collect(pointer, _):
        try:
            key = pointer.contents
            kind, guid = decode(key.type_name), decode(key.guid)
            required = c.c_size_t()
            result = dll.ShroudtopiaGetAssetJson(owner, pointer, None, 0, c.byref(required))
            if result: raise RuntimeError(f'{kind} size: {result}')
            if required.value > 64 * 1024 * 1024: raise RuntimeError('resource exceeds diagnostic budget')
            buffer = c.create_string_buffer(required.value)
            result = dll.ShroudtopiaGetAssetJson(owner, pointer, buffer, len(buffer), c.byref(required))
            if result: raise RuntimeError(f'{kind} read: {result}')
            data = json.loads(buffer.raw[:required.value])
            if kind == 'keen::ItemInfo' and not any(token in data.get('debugName', '').lower() for token in ('blueprint', 'quickbuild', 'shroudedit')):
                return 0
            folder = output / kind.split('::')[-1]
            folder.mkdir(exist_ok=True)
            (folder / f'{guid}_{key.part}.json').write_text(json.dumps(data, indent=2), encoding='utf-8')
            counts[kind] = counts.get(kind, 0) + 1
            return 0
        except Exception as error:
            errors.append(str(error))
            return 7
    try:
        for kind in ['Game38RootObjects', 'VoxelBlueprintItemRegistryResource', 'ItemRegistryResource', 'ItemInfo', 'VoxelModelResource', 'VoxelBlueprintConfig', 'ItemTag', 'ItemTagInfoList', 'FbUiBundle']:
            name, keep_name = view('keen::' + kind)
            result = dll.ShroudtopiaListAssets(name, collect, None)
            print(kind, 'result', result, 'exported', counts.get('keen::' + kind, 0), flush=True)
            if result not in (0, 3): raise RuntimeError(f'list: {result}: {errors}')
    finally:
        dll.ShroudtopiaAssetsClose()
    (output / 'summary.json').write_text(json.dumps(counts, indent=2), encoding='utf-8')

if __name__ == '__main__':
    main()

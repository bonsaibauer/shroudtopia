"""Read-only search across one reflected Enshrouded asset type."""
import argparse
import ctypes
import json
from pathlib import Path

class StringView(ctypes.Structure):
    _fields_=[("data",ctypes.c_char_p),("size",ctypes.c_size_t)]
class AssetId(ctypes.Structure):
    _fields_=[("struct_size",ctypes.c_size_t),("guid",StringView),("type_name",StringView),("part",ctypes.c_uint32)]
def view(value):
    encoded=value.encode(); return StringView(encoded,len(encoded)),encoded

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("engine",type=Path); parser.add_argument("game",type=Path)
    parser.add_argument("type"); parser.add_argument("needle")
    args=parser.parse_args()
    dll=ctypes.CDLL(str(args.engine.resolve()))
    visitor=ctypes.CFUNCTYPE(ctypes.c_int32,ctypes.POINTER(AssetId),ctypes.c_void_p)
    dll.ShroudtopiaAssetsOpen.argtypes=[StringView,StringView]
    dll.ShroudtopiaListAssets.argtypes=[StringView,visitor,ctypes.c_void_p]
    dll.ShroudtopiaGetAssetJson.argtypes=[StringView,ctypes.POINTER(AssetId),ctypes.c_void_p,ctypes.c_size_t,ctypes.POINTER(ctypes.c_size_t)]
    directory,directory_bytes=view(str(args.game.resolve())); stem,stem_bytes=view("enshrouded")
    owner,owner_bytes=view("shroudtopia.inspect"); type_name,type_bytes=view(args.type)
    if dll.ShroudtopiaAssetsOpen(directory,stem)!=0: raise RuntimeError("cannot open assets")
    matches=[]; errors=[]
    @visitor
    def inspect(key,_):
        required=ctypes.c_size_t()
        result=dll.ShroudtopiaGetAssetJson(owner,key,None,0,ctypes.byref(required))
        if result: errors.append(result); return result
        data=ctypes.create_string_buffer(required.value)
        result=dll.ShroudtopiaGetAssetJson(owner,key,data,len(data),ctypes.byref(required))
        if result: errors.append(result); return result
        text=data.raw.decode("utf-8")
        if args.needle.lower() in text.lower():
            asset=key.contents
            matches.append({"guid":ctypes.string_at(asset.guid.data,asset.guid.size).decode(),
                            "type":ctypes.string_at(asset.type_name.data,asset.type_name.size).decode(),
                            "part":asset.part,"data":json.loads(text)})
        return 0
    try:
        result=dll.ShroudtopiaListAssets(type_name,inspect,None)
        if result or errors: raise RuntimeError(f"asset listing failed: {result}, {errors[:3]}")
        print(json.dumps(matches,indent=2))
    finally:
        dll.ShroudtopiaAssetsClose()
if __name__=="__main__": main()

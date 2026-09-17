"""Read menu selection via the statically traced singleton; no thread suspension.

No debugger, injection, patches, engine calls or process writes. Output is
diagnostic evidence, not an authoritative/coherent gameplay snapshot.
"""
import argparse
import ctypes as c
import json
import struct
import time
from ctypes import wintypes as w

class Module(c.Structure):
    _fields_=[('size',w.DWORD),('id',w.DWORD),('pid',w.DWORD),('globalUse',w.DWORD),('processUse',w.DWORD),
              ('base',c.c_void_p),('imageSize',w.DWORD),('handle',w.HMODULE),('name',w.WCHAR*256),('path',w.WCHAR*260)]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('pid',type=int)
    parser.add_argument('--seconds',type=int,default=20)
    args=parser.parse_args()
    if not 1<=args.seconds<=60: parser.error('seconds must be 1..60')
    kernel=c.WinDLL('kernel32',use_last_error=True)
    kernel.OpenProcess.argtypes=[w.DWORD,w.BOOL,w.DWORD]; kernel.OpenProcess.restype=w.HANDLE
    kernel.CreateToolhelp32Snapshot.argtypes=[w.DWORD,w.DWORD]; kernel.CreateToolhelp32Snapshot.restype=w.HANDLE
    kernel.Module32FirstW.argtypes=[w.HANDLE,c.POINTER(Module)]; kernel.Module32FirstW.restype=w.BOOL
    kernel.ReadProcessMemory.argtypes=[w.HANDLE,c.c_void_p,c.c_void_p,c.c_size_t,c.POINTER(c.c_size_t)]; kernel.ReadProcessMemory.restype=w.BOOL
    kernel.CloseHandle.argtypes=[w.HANDLE]
    process=kernel.OpenProcess(0x410,False,args.pid)
    if not process: raise OSError(c.get_last_error(),'OpenProcess')
    def read(address,size):
        if not address or size>4096: raise ValueError('invalid bounded read')
        data=c.create_string_buffer(size); actual=c.c_size_t()
        if not kernel.ReadProcessMemory(process,address,data,size,c.byref(actual)) or actual.value!=size:
            raise OSError(c.get_last_error(),'ReadProcessMemory')
        return data.raw
    def pointer(address): return struct.unpack('<Q',read(address,8))[0]
    try:
        snapshot=kernel.CreateToolhelp32Snapshot(8,args.pid)
        try:
            module=Module(); module.size=c.sizeof(module)
            if not kernel.Module32FirstW(snapshot,c.byref(module)): raise OSError(c.get_last_error(),'Module32FirstW')
            if module.name.lower()!='enshrouded.exe': raise ValueError('unexpected main module')
            base=module.base
        finally: kernel.CloseHandle(snapshot)
        pe=struct.unpack('<I',read(base+0x3c,4))[0]
        if struct.unpack('<I',read(base+pe+8,4))[0]!=0x6a4236c8 or module.imageSize!=0x2da7000:
            raise ValueError('unsupported game build')
        if read(base+0xbe5b00,10)!=bytes.fromhex('48 8b 05 81 9a b5 01 0f b6 11'):
            raise ValueError('selection accessor bytes changed or hooked')
        until=time.monotonic()+args.seconds; previous=None
        while time.monotonic()<until:
            stage='singleton'
            try:
                singleton=pointer(base+0x273f588)
                stage='context'
                context=pointer(singleton+0xc8)
                stage='voxel_store'
                native_world=context-0x5a8
                try: voxel_store=pointer(native_world+0x1210)
                except OSError: voxel_store=0
                states=[]
                for mode,offset in enumerate((0x4fc20,0x4fc80,0x4fce0,0x4fd40),1):
                    try:
                        address=context+offset
                        header=read(address,24)
                        selected,indices,count=struct.unpack('<qQQ',header)
                        if count>32 or selected < -1 or selected>=count and selected!=-1:
                            states.append({'mode':mode,'invalid':True}); continue
                        values=read(indices,count*8) if count else b''
                        if read(address,24)!=header: continue
                        states.append({'mode':mode,'row':selected,'row_count':count,'slots':list(struct.unpack('<'+'q'*count,values))})
                    except (OSError,ValueError) as error:
                        states.append({'mode':mode,'unavailable':str(error)})
                report={'singleton':hex(singleton),'context':hex(context),'native_world':hex(native_world),
                        'voxel_store':hex(voxel_store),'states':states}
            except (OSError,ValueError) as error:
                report={'stage':stage,'unavailable':str(error)}
                if 'singleton' in locals(): report['singleton']=hex(singleton)
            encoded=json.dumps(report)
            if encoded!=previous: print(encoded,flush=True); previous=encoded
            time.sleep(.1)
    finally: kernel.CloseHandle(process)

if __name__=='__main__': main()

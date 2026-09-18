#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>
struct Thread { HANDLE handle; DWORD id; };
static std::uint64_t Read(HANDLE process,std::uint64_t address) {
    std::uint64_t value{}; SIZE_T done{};
    return ReadProcessMemory(process,reinterpret_cast<void*>(address),&value,8,&done) && done==8 ? value : 0;
}
int main(int argc,char** argv) {
    if(argc!=2) return 1;
    auto pid=static_cast<DWORD>(std::strtoul(argv[1],nullptr,10));
    auto process=OpenProcess(PROCESS_VM_READ|PROCESS_QUERY_INFORMATION,FALSE,pid);
    if(!process) return 2;
    auto modules=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid);
    MODULEENTRY32W module{sizeof(module)};
    if(!Module32FirstW(modules,&module)) return 3;
    auto base=reinterpret_cast<std::uint64_t>(module.modBaseAddr); CloseHandle(modules);
    auto snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
    THREADENTRY32 entry{sizeof(entry)}; std::vector<Thread> threads;
    if(Thread32First(snapshot,&entry)) do {
        if(entry.th32OwnerProcessID!=pid) continue;
        auto h=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT,FALSE,entry.th32ThreadID);
        if(h) threads.push_back({h,entry.th32ThreadID});
    } while(Thread32Next(snapshot,&entry));
    CloseHandle(snapshot);
    auto end=GetTickCount64()+25000; unsigned hits=0;
    while(GetTickCount64()<end && hits<3) for(auto thread:threads) {
        CONTEXT ctx{}; ctx.ContextFlags=CONTEXT_CONTROL|CONTEXT_INTEGER;
        if(SuspendThread(thread.handle)==DWORD(-1)) continue;
        const bool got=GetThreadContext(thread.handle,&ctx)!=FALSE;
        std::uint64_t row[33]{}, roots[10]{};
        const bool reader=got && ctx.Rip>=base+0xe819f8 && ctx.Rip<base+0xe81ee0;
        bool hit=got && ctx.Rip>=base+0x280887 && ctx.Rip<base+0x28104d;
        if(hit) {
            for(unsigned i=0;i<33;++i) row[i]=Read(process,ctx.Rbp-0x30+i*8);
            const auto voxel=row[(0x58+0x30)/8];
            roots[0]=Read(process,voxel); roots[1]=Read(process,voxel+8);
            for(unsigned i=0;i<2;++i) { roots[2+i]=Read(process,roots[i]+0x1210); roots[4+i]=Read(process,roots[i]+0x30); }
            roots[6]=Read(process,ctx.R15); roots[7]=Read(process,roots[6]+0x50); roots[8]=Read(process,roots[6]+0x6c);
            roots[9]=Read(process,row[(0xb8+0x30)/8]+0x1b8);
        }
        if(reader) { roots[0]=Read(process,ctx.Rbp+0x510); roots[1]=Read(process,roots[0]+0x1210); }
        ResumeThread(thread.handle);
        if(reader) { ++hits; std::cout<<std::hex<<"reader rva="<<ctx.Rip-base<<" world="<<roots[0]<<" store="<<roots[1]<<std::dec<<'\n'; }
        if(hit) {
            ++hits; std::cout<<"hit thread="<<thread.id<<std::hex<<" rva="<<ctx.Rip-base<<" rbp="<<ctx.Rbp<<" r15="<<ctx.R15<<'\n';
            for(unsigned i=0;i<33;++i) std::cout<<"row+"<<static_cast<int>(i*8)-0x30<<"="<<row[i]<<'\n';
            for(unsigned i=0;i<10;++i) std::cout<<"root["<<i<<"]="<<roots[i]<<'\n';
            std::cout<<std::dec;
        }
    }
    for(auto t:threads) CloseHandle(t.handle); CloseHandle(process);
    std::cout<<"hits="<<hits<<'\n'; return hits?0:4;
}

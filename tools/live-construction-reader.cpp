#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <array>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

namespace {
bool Number(const char* text,std::uint64_t& value) {
    const auto end=text+std::char_traits<char>::length(text);
    return std::from_chars(text,end,value,10).ec==std::errc{};
}
bool Pointer(std::uint64_t value) { return value>=0x10000 && value<0x0000800000000000ULL; }
bool GamePointer(std::uint64_t value) { return value>=0x0000020000000000ULL && value<0x0000070000000000ULL; }
template<class T> T At(const std::uint8_t* data,std::size_t offset) {
    T value{}; std::memcpy(&value,data+offset,sizeof(value)); return value;
}
bool Readable(HANDLE process,std::uint64_t address) {
    MEMORY_BASIC_INFORMATION info{};
    if (!VirtualQueryEx(process,reinterpret_cast<const void*>(address),&info,sizeof(info)) ||
        info.State!=MEM_COMMIT || (info.Protect&(PAGE_NOACCESS|PAGE_GUARD))) return false;
    return true;
}
}
int main(int argc,char** argv) {
    if (argc!=2 && argc!=3) { std::cerr<<"usage: live-construction-reader <pid> [address-hex]\n"; return 2; }
    std::uint64_t pid{}; if (!Number(argv[1],pid)||pid>UINT32_MAX) return 2;
    const auto process=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,static_cast<DWORD>(pid));
    if (!process) { std::cerr<<"OpenProcess failed "<<GetLastError()<<'\n'; return 3; }
    if (argc==3) {
        std::uint64_t target{};
        const auto end=argv[2]+std::char_traits<char>::length(argv[2]);
        if (std::from_chars(argv[2],end,target,16).ec!=std::errc{}) return 2;
        std::array<std::uint64_t,128> values{}; SIZE_T done{};
        if (!ReadProcessMemory(process,reinterpret_cast<const void*>(target),values.data(),sizeof(values),&done)) return 4;
        for (std::size_t i=0;i<done/8;++i) if (values[i])
            std::cout<<std::hex<<"+0x"<<i*8<<" 0x"<<values[i]<<std::dec<<'\n';
        CloseHandle(process); return 0;
    }
    SYSTEM_INFO system{}; GetSystemInfo(&system);
    auto address=reinterpret_cast<std::uintptr_t>(system.lpMinimumApplicationAddress);
    const auto maximum=reinterpret_cast<std::uintptr_t>(system.lpMaximumApplicationAddress);
    std::size_t matches{};
    while (address<maximum) {
        MEMORY_BASIC_INFORMATION info{};
        if (!VirtualQueryEx(process,reinterpret_cast<const void*>(address),&info,sizeof(info))) break;
        const auto next=address+info.RegionSize;
        const bool candidate=info.State==MEM_COMMIT && info.Type==MEM_PRIVATE &&
            !(info.Protect&(PAGE_NOACCESS|PAGE_GUARD)) && (info.Protect&(PAGE_READWRITE|PAGE_WRITECOPY|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY));
        if (candidate) {
            constexpr std::size_t chunkSize=1u<<20,overlap=0x140;
            for (std::size_t offset=0;offset<info.RegionSize;) {
                const auto wanted=std::min<std::size_t>(chunkSize+overlap,info.RegionSize-offset);
                std::vector<std::uint8_t> bytes(wanted); SIZE_T done{};
                if (ReadProcessMemory(process,reinterpret_cast<const void*>(address+offset),bytes.data(),bytes.size(),&done) && done>=overlap) {
                    const auto limit=std::min<std::size_t>(chunkSize,done-overlap+1);
                    for (std::size_t i=0;i<limit;i+=8) {
                        const auto object=address+offset+i;
                        // Game heap allocations in the observed client use the
                        // low multi-terabyte range; exclude thread stacks and DLL heaps.
                        if (!GamePointer(object)) continue;
                        const auto q0=At<std::uint64_t>(bytes.data()+i,0);
                        const auto b0=At<std::uint64_t>(bytes.data()+i,0xb0);
                        const auto c0=At<std::uint64_t>(bytes.data()+i,0xc0);
                        const auto d8=At<std::uint64_t>(bytes.data()+i,0xd8);
                        const auto e0=At<std::uint64_t>(bytes.data()+i,0xe0);
                        const auto begin=At<std::uint64_t>(bytes.data()+i,0x100);
                        const auto end=At<std::uint64_t>(bytes.data()+i,0x108);
                        const auto auxiliary=At<std::uint64_t>(bytes.data()+i,0x118);
                        const auto owner=At<std::uint32_t>(bytes.data()+i,0x134);
                        if (!GamePointer(q0)||!GamePointer(b0)||!GamePointer(c0)||!GamePointer(d8)||!GamePointer(e0)||!GamePointer(auxiliary)||
                            !owner||owner>100000 || end<begin || end-begin>(1u<<20) || ((begin||end)&&(!Pointer(begin)||!Pointer(end)))) continue;
                        if (!Readable(process,q0)||!Readable(process,b0)||!Readable(process,c0)||!Readable(process,d8)||
                            !Readable(process,e0)||!Readable(process,auxiliary)) continue;
                        std::cout<<std::hex<<"candidate=0x"<<object<<" q0=0x"<<q0<<" b0=0x"<<b0
                                 <<" c0=0x"<<c0<<" d8=0x"<<d8<<" e0=0x"<<e0<<" begin=0x"<<begin
                                 <<" end=0x"<<end<<" aux=0x"<<auxiliary<<std::dec<<" owner="<<owner<<'\n';
                        if (++matches>=100) { CloseHandle(process); return 0; }
                    }
                }
                offset+=std::min<std::size_t>(chunkSize,info.RegionSize-offset);
            }
        }
        if (next<=address) break; address=next;
    }
    std::cout<<"matches="<<matches<<'\n'; CloseHandle(process); return 0;
}

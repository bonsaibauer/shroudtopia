#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
int main(int argc,char** argv) {
    if(argc!=3) return 1;
    DWORD pid=std::strtoul(argv[1],nullptr,10);
    HANDLE p=OpenProcess(PROCESS_VM_READ|PROCESS_QUERY_INFORMATION,FALSE,pid);
    if(!p) return 2;
    auto read=[&](std::uint64_t a,void* b,SIZE_T n){SIZE_T d{};return ReadProcessMemory(p,(void*)a,b,n,&d)&&d==n;};
    HANDLE s=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid);
    MODULEENTRY32W m{sizeof(m)};
    if(!Module32FirstW(s,&m)) return 3;
    CloseHandle(s);
    auto address=(std::uint64_t)m.modBaseAddr+std::strtoull(argv[2],nullptr,16);
    unsigned char jump[5]{}; if(!read(address,jump,5)||jump[0]!=0xe9) return 4;
    std::int32_t delta{};std::memcpy(&delta,jump+1,4);
    auto payload=address+5+delta;
    unsigned char bytes[256]{};if(!read(payload,bytes,sizeof(bytes))) return 5;
    for(unsigned i=0;i<100;++i) if(bytes[i]==0x48&&bytes[i+1]==0xbf) {
        std::uint64_t record{};std::memcpy(&record,bytes+i+2,8);
        std::uint64_t words[8]{};if(!read(record,words,sizeof(words)))return 6;
        std::cout<<std::hex<<"payload="<<payload<<" record="<<record<<std::dec<<" sequence="<<words[1]<<'\n';
        for(unsigned j=0;j<6;++j)std::cout<<"arg"<<j<<"="<<std::hex<<words[j+2]<<'\n';
        CloseHandle(p);return 0;
    }
    CloseHandle(p);return 7;
}

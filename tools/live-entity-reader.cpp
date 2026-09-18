#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <array>
#include <charconv>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>
#include <vector>

namespace {
template<class T> bool Read(HANDLE process,std::uintptr_t address,T& value) {
    SIZE_T done{};
    return address && ReadProcessMemory(process,reinterpret_cast<const void*>(address),&value,sizeof(value),&done) && done==sizeof(value);
}
bool ReadBytes(HANDLE process,std::uintptr_t address,void* value,std::size_t size) {
    SIZE_T done{};
    return address && ReadProcessMemory(process,reinterpret_cast<const void*>(address),value,size,&done) && done==size;
}
bool Number(const char* text,int base,std::uint64_t& value) {
    const auto end=text+std::char_traits<char>::length(text);
    return std::from_chars(text,end,value,base).ec==std::errc{};
}
std::string Ascii(HANDLE process,std::uintptr_t address) {
    std::array<char,96> bytes{};
    SIZE_T done{};
    if (!address || !ReadProcessMemory(process,reinterpret_cast<const void*>(address),bytes.data(),bytes.size()-1,&done) || !done) return {};
    std::size_t size{};
    while (size<done && bytes[size] && static_cast<unsigned char>(bytes[size])>=0x20 && static_cast<unsigned char>(bytes[size])<0x7f) ++size;
    return size>=4 ? std::string(bytes.data(),size) : std::string{};
}
}
int main(int argc,char** argv) {
    if (argc!=4) { std::cerr<<"usage: live-entity-reader <pid-decimal> <manager-hex> <entity-decimal>\n"; return 2; }
    std::uint64_t pid64{},manager64{},wanted64{};
    if (!Number(argv[1],10,pid64)||!Number(argv[2],16,manager64)||!Number(argv[3],10,wanted64)||pid64>UINT32_MAX||wanted64>UINT32_MAX) return 2;
    const auto process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|PROCESS_VM_READ,FALSE,static_cast<DWORD>(pid64));
    if (!process) { std::cerr<<"OpenProcess failed "<<GetLastError()<<'\n'; return 3; }
    const auto manager=static_cast<std::uintptr_t>(manager64);
    std::uint64_t slots{},table{};
    if (!Read(process,manager+0x158,slots)||!Read(process,manager+0x188,table)||!slots||slots>(1u<<20)) {
        std::cerr<<"invalid manager header\n"; CloseHandle(process); return 4;
    }
    std::vector<std::uintptr_t> pointers(static_cast<std::size_t>(slots));
    if (!ReadBytes(process,static_cast<std::uintptr_t>(table),pointers.data(),pointers.size()*sizeof(pointers[0]))) {
        std::cerr<<"cannot read pointer table\n"; CloseHandle(process); return 5;
    }
    std::size_t occupied{}; std::uintptr_t found{};
    for (const auto pointer:pointers) {
        if (!pointer) continue;
        ++occupied; std::uint32_t id{};
        if (Read(process,pointer+0x10,id) && id==wanted64) { found=pointer; break; }
    }
    std::cout<<"slots="<<slots<<" occupied_before_match="<<occupied<<" table=0x"<<std::hex<<table<<" entity=0x"<<found<<std::dec<<'\n';
    if (!found) { CloseHandle(process); return 6; }
    std::array<std::uint64_t,64> words{};
    if (!ReadBytes(process,found,words.data(),sizeof(words))) { CloseHandle(process); return 7; }
    for (std::size_t i=0;i<words.size();++i) std::cout<<"+0x"<<std::hex<<i*8<<" 0x"<<words[i]<<std::dec<<'\n';
    std::array<std::uint64_t,64> archetype{};
    if (ReadBytes(process,static_cast<std::uintptr_t>(words[5]),archetype.data(),sizeof(archetype))) {
        std::cout<<"archetype=0x"<<std::hex<<words[5]<<std::dec<<'\n';
        for (std::size_t i=0;i<archetype.size();++i) {
            const auto string=Ascii(process,static_cast<std::uintptr_t>(archetype[i]));
            if (archetype[i] || !string.empty()) {
                std::cout<<"  +0x"<<std::hex<<i*8<<" 0x"<<archetype[i]<<std::dec;
                if (!string.empty()) std::cout<<" string="<<std::quoted(string);
                std::cout<<'\n';
            }
        }
    }
    const auto layout=static_cast<std::uintptr_t>(words[3]);
    const auto storage=static_cast<std::uintptr_t>(words[4]);
    const auto row=static_cast<std::uint32_t>(words[6]);
    const auto componentCount=static_cast<std::uint32_t>(words[7]);
    std::array<std::uint64_t,16> bits{};
    if (!ReadBytes(process,layout,bits.data(),sizeof(bits))) { CloseHandle(process); return 8; }
    std::cout<<"components layout=0x"<<std::hex<<layout<<" storage=0x"<<storage<<std::dec
             <<" row="<<row<<" count="<<componentCount<<'\n';
    std::uint32_t visited{};
    for (std::uint32_t index=0;index<1024 && visited<componentCount;++index) {
        if (!(bits[index/64]&(std::uint64_t{1}<<(index%64)))) continue;
        ++visited;
        std::uint16_t offset{},stride{};
        if (!Read(process,layout+0x84+index*2,offset)||!Read(process,layout+0xa84+index*2,stride)) continue;
        const auto address=storage+offset+static_cast<std::uintptr_t>(row)*stride;
        std::cout<<"component="<<index<<" offset=0x"<<std::hex<<offset<<" stride=0x"<<stride;
        if (!stride || stride>0x1000) {
            std::cout<<" tag"<<std::dec<<'\n';
            continue;
        }
        std::array<std::uint64_t,8> prefix{};
        if (!ReadBytes(process,address,prefix.data(),sizeof(prefix))) continue;
        std::cout<<" address=0x"<<address<<" data";
        for (const auto value:prefix) std::cout<<" 0x"<<value;
        std::cout<<std::dec<<'\n';
    }
    if (visited!=componentCount) std::cerr<<"component bitmap mismatch: expected "<<componentCount<<", visited "<<visited<<'\n';
    CloseHandle(process); return 0;
}

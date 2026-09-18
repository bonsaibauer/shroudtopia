#ifdef NDEBUG
#undef NDEBUG
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../src/loader/world_diagnostic_payload.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace WorldDiagnostic;
    Record record{}; record.enabled=1;
    std::array<std::uint8_t,0x310> input{};
    for (std::size_t i=0;i<160;++i) input[0x270+i]=static_cast<std::uint8_t>(i);
    auto code=Payload(&record,Kind::Cursor);
    // Test wrapper: preserve R14; set it to RCX; inline diagnostic; restore/return.
    // Also verify RAX, carry and direction flag preservation. Clear DF before
    // returning to C++ so the harness itself obeys the Windows ABI.
    code.insert(code.begin(),{0x41,0x56,0x49,0x89,0xce,0xb8,0x78,0x56,0x34,0x12,0xfd,0xf9});
    code.insert(code.end(),{0x9c,0x5a,0xfc,0x81,0xe2,0x01,0x04,0,0,0x48,0xc1,0xe2,0x20,0x48,0x09,0xd0,0x41,0x5e,0xc3});
    auto allocation=VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE); assert(allocation);
    std::memcpy(allocation,code.data(),code.size()); DWORD previous{};
    assert(VirtualProtect(allocation,4096,PAGE_EXECUTE_READ,&previous));
    FlushInstructionCache(GetCurrentProcess(),allocation,code.size());
    const auto call=reinterpret_cast<std::uint64_t(*)(const void*)>(allocation);
    constexpr auto preserved=(std::uint64_t{0x401}<<32)|0x12345678;
    assert(call(input.data())==preserved);
    assert(record.sequence==1 && record.lock==0 && record.thread==GetCurrentThreadId());
    assert(record.arguments[0]==reinterpret_cast<std::uintptr_t>(input.data()));
    assert(std::memcmp(&record.cursor,input.data()+0x270,160)==0);
    record.enabled=0; assert(call(nullptr)==preserved); assert(record.sequence==1); // disabled must not dereference R14
    record.enabled=1; record.lock=1; assert(call(nullptr)==preserved); assert(record.sequence==1 && record.lock==1);
    VirtualFree(allocation,0,MEM_RELEASE);

    for (const auto kind:{Kind::TerrainEntry,Kind::WorldContext}) {
    record={}; record.enabled=1;
    code=Payload(&record,kind); code.push_back(0xc3);
    allocation=VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE); assert(allocation);
    std::memcpy(allocation,code.data(),code.size());
    assert(VirtualProtect(allocation,4096,PAGE_EXECUTE_READ,&previous));
    FlushInstructionCache(GetCurrentProcess(),allocation,code.size());
    reinterpret_cast<void(*)(std::uint64_t,std::uint64_t,std::uint64_t,std::uint64_t,std::uint64_t)>(allocation)(11,22,33,44,55);
    assert(record.sequence==1 && record.lock==0 && record.thread==GetCurrentThreadId());
    assert(record.arguments[0]==11 && record.arguments[1]==22 && record.arguments[2]==33 && record.arguments[3]==44);
    assert(record.arguments[4]!=0 && record.arguments[5]==55);
    VirtualFree(allocation,0,MEM_RELEASE);
    }
    for (const auto ownerOffset:{0x44,0x40}) {
        record={}; record.enabled=1;
        std::array<std::uint8_t,72> event{};
        std::array<std::uint8_t,0x138> context{};
        for (std::size_t i=0;i<event.size();++i) event[i]=static_cast<std::uint8_t>(i);
        const std::uint32_t owner=1337;
        std::memcpy(context.data()+0x134,&owner,4);
        code=Payload(&record,Kind::BuildingEvent);
        // Wrapper establishes actual hook registers and replays the owner store
        // before the sample, exactly as the runtime trampoline does.
        code.insert(code.begin(),{0x53,0x41,0x56,0x48,0x8b,0xd9,0x4c,0x8b,0xf2,
            0x41,0x8b,0x86,0x34,0x01,0,0,0x89,0x43,static_cast<std::uint8_t>(ownerOffset)});
        code.insert(code.end(),{0x41,0x5e,0x5b,0xc3});
        allocation=VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE); assert(allocation);
        std::memcpy(allocation,code.data(),code.size());
        assert(VirtualProtect(allocation,4096,PAGE_EXECUTE_READ,&previous));
        FlushInstructionCache(GetCurrentProcess(),allocation,code.size());
        reinterpret_cast<void(*)(void*,void*)>(allocation)(event.data(),context.data());
        assert(record.sequence==1 && record.thread==GetCurrentThreadId());
        assert(std::memcmp(&record.cursor,event.data(),event.size())==0);
        std::uint32_t capturedOwner{};
        std::memcpy(&capturedOwner,reinterpret_cast<const std::uint8_t*>(&record.cursor)+ownerOffset,4);
        assert(capturedOwner==owner);
        assert(record.arguments[0]==reinterpret_cast<std::uintptr_t>(context.data()));
        VirtualFree(allocation,0,MEM_RELEASE);
    }
    record={}; record.enabled=1;
    std::array<std::uint32_t,18> descriptor{};
    for (std::size_t i=0;i<descriptor.size();++i) descriptor[i]=static_cast<std::uint32_t>(100+i);
    code=Payload(&record,Kind::VoxelWriteEntry); code.push_back(0xc3);
    allocation=VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE); assert(allocation);
    std::memcpy(allocation,code.data(),code.size());
    assert(VirtualProtect(allocation,4096,PAGE_EXECUTE_READ,&previous));
    FlushInstructionCache(GetCurrentProcess(),allocation,code.size());
    using WriteEntry=void(*)(std::uint64_t,std::uint64_t,std::uint64_t,std::uint64_t,std::uint64_t,const void*,std::uint64_t,std::uint64_t,std::uint64_t);
    reinterpret_cast<WriteEntry>(allocation)(11,22,33,44,55,descriptor.data(),0x32,1,0);
    assert(record.sequence==1 && record.arguments[0]==11 && record.arguments[5]==55);
    assert(std::memcmp(&record.cursor,descriptor.data(),sizeof(descriptor))==0);
    const auto* raw=reinterpret_cast<const std::uint8_t*>(&record.cursor);
    assert(raw[72]==0x32 && raw[80]==1 && raw[88]==0);
    VirtualFree(allocation,0,MEM_RELEASE);
    std::cout<<"Building event replay/copy and voxel descriptor/stack arguments passed.\n";
    std::cout<<"Native diagnostic payload: cursor bytes, entry arguments, thread, disabled and busy paths passed.\n";
}

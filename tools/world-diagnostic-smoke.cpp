#ifdef NDEBUG
#undef NDEBUG
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../src/loader/world_diagnostic_payload.h"
#include <cassert>
#include <iostream>
#include <intrin.h>

static const void* callbackView{};
static const void* callbackFrame{};
__declspec(noinline) static void TestActorCallback(const void* view,const void* frame) {
    assert((reinterpret_cast<std::uintptr_t>(_AddressOfReturnAddress())+8)%16==0);
    callbackView=view;
    callbackFrame=frame;
}

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

    record={}; record.enabled=1;
    std::array<std::uint8_t,0x138> placement{};
    const std::uintptr_t root=0x1122334455667788ULL;
    const std::uintptr_t placeQueue=0x2233445566778899ULL;
    const std::uintptr_t removeQueue=0x33445566778899aaULL;
    const std::uint32_t placementOwner=0x89abcdef;
    const auto rootView=reinterpret_cast<std::uintptr_t>(&root);
    std::memcpy(placement.data(),&rootView,sizeof(rootView));
    std::memcpy(placement.data()+0xb0,&placeQueue,sizeof(placeQueue));
    std::memcpy(placement.data()+0xc0,&removeQueue,sizeof(removeQueue));
    std::memcpy(placement.data()+0x134,&placementOwner,sizeof(placementOwner));
    code=Payload(&record,Kind::PlacementContext); code.push_back(0xc3);
    allocation=VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE); assert(allocation);
    std::memcpy(allocation,code.data(),code.size());
    assert(VirtualProtect(allocation,4096,PAGE_EXECUTE_READ,&previous));
    FlushInstructionCache(GetCurrentProcess(),allocation,code.size());
    reinterpret_cast<void(*)(const void*)>(allocation)(placement.data());
    assert(record.sequence==1 && record.lock==0 && record.thread==GetCurrentThreadId());
    assert(record.arguments[0]==reinterpret_cast<std::uintptr_t>(placement.data()));
    assert(record.arguments[1]==root && record.arguments[2]==placeQueue &&
        record.arguments[3]==removeQueue && record.arguments[4]==placementOwner);
    reinterpret_cast<void(*)(const void*)>(allocation)(nullptr);
    assert(record.sequence==1 && record.lock==0); // A null context must not erase the stable snapshot.
    VirtualFree(allocation,0,MEM_RELEASE);

    record={}; record.enabled=1;
    std::array<std::uintptr_t,254> actorRoot{};
    std::array<std::uint32_t,8> actorOwners{}; actorOwners[4]=1234;
    actorRoot[0x7e0/8]=reinterpret_cast<std::uintptr_t>(actorOwners.data());
    std::array<std::uintptr_t,2> actorView{reinterpret_cast<std::uintptr_t>(actorRoot.data()),2};
    std::array<std::uintptr_t,128> actorRow{}; actorRow[0x310/8]=placeQueue; actorRow[0x320/8]=removeQueue;
    std::array<std::uintptr_t,2> actorServices{0,0x123456789abcdef0ULL};
    actorRow[0x20/8]=reinterpret_cast<std::uintptr_t>(actorServices.data());
    const std::uint32_t actorOwner=1234; std::memcpy(reinterpret_cast<std::uint8_t*>(actorRow.data())+0x394,&actorOwner,4);
    code=Payload(&record,Kind::ActorPlacementContext);
    code.insert(code.begin(),{0x55,0x41,0x57,0x48,0x8b,0xea,0x4c,0x8b,0xf9});
    code.insert(code.end(),{0x41,0x5f,0x5d,0xc3});
    allocation=VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE); assert(allocation);
    std::memcpy(allocation,code.data(),code.size());
    assert(VirtualProtect(allocation,4096,PAGE_EXECUTE_READ,&previous));
    FlushInstructionCache(GetCurrentProcess(),allocation,code.size());
    reinterpret_cast<void(*)(const void*,const void*)>(allocation)(actorView.data(),actorRow.data());
    assert(record.sequence==1 && record.lock==0 && record.arguments[1]==actorView[0]);
    assert(record.arguments[2]==placeQueue && record.arguments[3]==removeQueue && record.arguments[4]==1234);
    assert(record.arguments[5]==actorServices[1]);
    actorView[1]=0;
    actorRow[0x20/8]=0;
    const std::uint32_t noActorOwner=0; std::memcpy(reinterpret_cast<std::uint8_t*>(actorRow.data())+0x394,&noActorOwner,4);
    reinterpret_cast<void(*)(const void*,const void*)>(allocation)(actorView.data(),actorRow.data());
    assert(record.sequence==2 && record.arguments[4]==0);
    assert(record.arguments[5]==0);
    VirtualFree(allocation,0,MEM_RELEASE);

    record={}; record.enabled=1;
    std::array<std::uintptr_t,32> worldRow{};
    std::array<std::uintptr_t,2> worldServices{0,0x0fedcba987654321ULL};
    worldRow[0x58/8]=reinterpret_cast<std::uintptr_t>(worldServices.data());
    code=Payload(&record,Kind::ActorWorldContext);
    code.insert(code.begin(),{0x55,0x48,0x8b,0xea});
    code.insert(code.end(),{0x5d,0xc3});
    allocation=VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE); assert(allocation);
    std::memcpy(allocation,code.data(),code.size());
    assert(VirtualProtect(allocation,4096,PAGE_EXECUTE_READ,&previous));
    FlushInstructionCache(GetCurrentProcess(),allocation,code.size());
    reinterpret_cast<void(*)(const void*,const void*)>(allocation)(nullptr,worldRow.data());
    assert(record.sequence==1 && record.lock==0 && record.arguments[5]==worldServices[1]);
    worldRow[0x58/8]=0;
    reinterpret_cast<void(*)(const void*,const void*)>(allocation)(nullptr,worldRow.data());
    assert(record.sequence==2 && record.arguments[5]==0);
    VirtualFree(allocation,0,MEM_RELEASE);

    code=ActorCallback(reinterpret_cast<void*>(&TestActorCallback));
    // Two saved nonvolatile registers plus padding align the engine call site.
    code.insert(code.begin(),{0x55,0x41,0x57,0x48,0x83,0xec,0x08,
        0x48,0x8b,0xea,0x4c,0x8b,0xf9,0xb8,0x78,0x56,0x34,0x12,0xfd,0xf9});
    code.insert(code.end(),{0x9c,0x5a,0xfc,0x81,0xe2,0x01,0x04,0,0,
        0x48,0xc1,0xe2,0x20,0x48,0x09,0xd0,0x48,0x83,0xc4,0x08,0x41,0x5f,0x5d,0xc3});
    allocation=VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE); assert(allocation);
    std::memcpy(allocation,code.data(),code.size());
    assert(VirtualProtect(allocation,4096,PAGE_EXECUTE_READ,&previous));
    FlushInstructionCache(GetCurrentProcess(),allocation,code.size());
    assert((reinterpret_cast<std::uint64_t(*)(const void*,const void*)>(allocation)(actorView.data(),actorRow.data())==preserved));
    assert(callbackView==actorView.data() && callbackFrame==actorRow.data());
    VirtualFree(allocation,0,MEM_RELEASE);

    callbackView=nullptr; callbackFrame=nullptr;
    code=EntryCallback(reinterpret_cast<void*>(&TestActorCallback)); code.push_back(0xc3);
    allocation=VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE); assert(allocation);
    std::memcpy(allocation,code.data(),code.size());
    assert(VirtualProtect(allocation,4096,PAGE_EXECUTE_READ,&previous));
    FlushInstructionCache(GetCurrentProcess(),allocation,code.size());
    reinterpret_cast<void(*)(const void*,const void*)>(allocation)(actorView.data(),actorRow.data());
    assert(callbackView==actorView.data() && callbackFrame==actorRow.data());
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
    record={}; record.enabled=1;
    std::array<std::uint8_t,0x110> objectSlots{};
    for (std::size_t i=0;i<0x88;++i) objectSlots[0x88+i]=static_cast<std::uint8_t>(i);
    const std::uint32_t objectId=0x12345678;
    std::memcpy(objectSlots.data()+0x88+0x64,&objectId,sizeof(objectId));
    std::array<std::uintptr_t,7> contextRoot{};
    std::array<std::uint64_t,64> entityManager{};
    contextRoot[6]=reinterpret_cast<std::uintptr_t>(entityManager.data()); // +30h
    const auto contextRootAddress=reinterpret_cast<std::uintptr_t>(contextRoot.data());
    code=Payload(&record,Kind::ObjectLook);
    // Wrapper supplies original RDI/RBX and the transient R15 lookup context.
    code.insert(code.begin(),{0x53,0x57,0x41,0x57,0x48,0x8b,0xf9,0x48,0x8b,0xda,0x4d,0x8b,0xf8});
    code.insert(code.end(),{0x41,0x5f,0x5f,0x5b,0xc3});
    allocation=VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE); assert(allocation);
    std::memcpy(allocation,code.data(),code.size());
    assert(VirtualProtect(allocation,4096,PAGE_EXECUTE_READ,&previous));
    FlushInstructionCache(GetCurrentProcess(),allocation,code.size());
    reinterpret_cast<void(*)(void*,std::size_t,const void*)>(allocation)(objectSlots.data(),0x88,&contextRootAddress);
    assert(record.sequence==1 && record.arguments[0]==reinterpret_cast<std::uintptr_t>(objectSlots.data()));
    assert(record.arguments[1]==0x88 && std::memcmp(&record.cursor,objectSlots.data()+0x88,0x88)==0);
    assert(record.arguments[4]==reinterpret_cast<std::uintptr_t>(&contextRootAddress));
    assert(record.arguments[5]==reinterpret_cast<std::uintptr_t>(entityManager.data()));
    reinterpret_cast<void(*)(void*,std::size_t,const void*)>(allocation)(objectSlots.data(),0,&contextRootAddress);
    assert(record.sequence==1 && record.lock==0); // Empty slots do not erase evidence.
    VirtualFree(allocation,0,MEM_RELEASE);

    record={}; record.enabled=1;
    const std::array<std::uint64_t,2> resource{0x1122334455667788ULL,0x99aabbccddeeff00ULL};
    std::memcpy(objectSlots.data()+0x64,&objectId,sizeof(objectId));
    code=Payload(&record,Kind::ObjectResource);
    // Wrapper supplies original RAX=component, RDI=slot base and RBX=0.
    code.insert(code.begin(),{0x53,0x57,0x48,0x8b,0xc1,0x48,0x8b,0xfa,0x31,0xdb});
    code.insert(code.end(),{0x5f,0x5b,0xc3});
    allocation=VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE); assert(allocation);
    std::memcpy(allocation,code.data(),code.size());
    assert(VirtualProtect(allocation,4096,PAGE_EXECUTE_READ,&previous));
    FlushInstructionCache(GetCurrentProcess(),allocation,code.size());
    reinterpret_cast<void(*)(const void*,void*)>(allocation)(resource.data(),objectSlots.data());
    assert(record.sequence==1 && record.arguments[0]==reinterpret_cast<std::uintptr_t>(resource.data()));
    assert(record.arguments[4]==objectId && std::memcmp(&record.cursor,resource.data(),sizeof(resource))==0);
    reinterpret_cast<void(*)(const void*,void*)>(allocation)(nullptr,objectSlots.data());
    assert(record.sequence==1 && record.lock==0);
    VirtualFree(allocation,0,MEM_RELEASE);

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

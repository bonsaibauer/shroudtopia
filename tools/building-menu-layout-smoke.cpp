#ifdef NDEBUG
#undef NDEBUG
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../src/loader/building_native_layout.h"
#include <cassert>
#include <cstring>
#include <iostream>

int wmain(int argc,wchar_t** argv) {
    using namespace BuildingNative;
    std::array<Item,3> items{};
    items[0].item_id=1; items[1].item_id=0x53450001; items[2].item_id=3;
    Row source{}; source.label=7; source.items=items.data(); source.item_count=items.size();
    source.kind=2; source.style=1;
    std::array<Item,1> selected{};
    Row blueprint{};
    assert(PrepareRow(source,items,0x53450001,99,selected,&blueprint)==RESULT_OK);
    assert(blueprint.label==99 && blueprint.item_count==1 && blueprint.items[0].item_id==0x53450001);
    assert(source.label==7 && source.item_count==3);
    assert(PrepareRow(source,items,42,99,selected,&blueprint)==RESULT_NOT_FOUND);
    assert(PrepareRow(source,items,0x53450001,99,{},&blueprint)==RESULT_INVALID_ARGUMENT);
    std::array<Row,7> rows{}; rows[2]=source;
    std::size_t count=6;
    assert(AppendOwnedRow(std::span(rows).first(6),&count,blueprint)==RESULT_NOT_AVAILABLE && count==6);
    assert(AppendOwnedRow(rows,&count,blueprint)==RESULT_OK && count==7);
    assert(rows[2].label==7 && rows[6].label==99);
    if (argc==2) {
        // Execute only the proven leaf copy routine on our own two buffers.
        auto image=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);
        assert(image);
        auto* base=reinterpret_cast<const unsigned char*>(image);
        const auto* dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
        const auto* nt=reinterpret_cast<const IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
        assert(nt->FileHeader.TimeDateStamp==0x6a4236c8 && nt->OptionalHeader.SizeOfImage==0x2da7000);
        const unsigned char expected[]{0x0f,0xb6,0x02,0x88,0x01,0x8b,0x42,0x04};
        assert(std::memcmp(base+0xa05180,expected,sizeof(expected))==0);
        // Static guard for the initialization condition used by Selection.
        // Do not call it: this block accesses live engine/UI-owned state.
        const unsigned char selectionInit[]{0x49,0x83,0x7f,0x10,0x00};
        assert(std::memcmp(base+0xb08f75,selectionInit,sizeof(selectionInit))==0);
        using CopyRow=Row* (CALL*)(Row*,const Row*);
        auto copy=reinterpret_cast<CopyRow>(const_cast<unsigned char*>(base)+0xa05180);
        Row actual{};
        assert(copy(&actual,&blueprint)==&actual);
        assert(actual.label==99 && actual.items==selected.data() && actual.item_count==1);
        assert(actual.kind==source.kind && actual.style==source.style);
        // The menu's inline vector is allocator-less. The original resize
        // callback must refuse growth in its fallible mode; passing false to
        // the same growth request reaches the engine fatal-error path.
        const unsigned char reserveEntry[]{0x45,0x0f,0xb6,0xc8,0x48,0x8b,0xc2};
        assert(std::memcmp(base+0xa112a0,reserveEntry,sizeof(reserveEntry))==0);
        struct InlineRows { Rows vector; std::array<Row,6> storage; } owned{};
        owned.vector={owned.storage.data(),6,6,nullptr,nullptr};
        owned.storage[2]=source;
        const auto before=owned;
        using ReserveRows=bool (CALL*)(Rows*,std::int64_t,bool);
        const auto reserve=reinterpret_cast<ReserveRows>(const_cast<unsigned char*>(base)+0xa112a0);
        assert(!reserve(&owned.vector,-7,true));
        assert(std::memcmp(&owned,&before,sizeof(owned))==0);
        FreeLibrary(image);
        std::cout << "Original engine row-copy and allocator-less seventh-row rejection passed with owned buffers.\n";
    }
    std::cout << "Building row filtering and six/seven-row capacity tests passed.\n";
}

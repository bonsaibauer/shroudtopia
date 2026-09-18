// Execute only the recovered buffer operation in an isolated process, with
// owned test buffers. Does not connect to the running game or load a save.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include "../src/loader/world_native_layout.h"

static int Invoke(WorldNative::ApplyTerrainBuffer operation, const WorldNative::TerrainOperation* context, WorldNative::TerrainBuffer* buffer,
    const std::uint32_t* offset, const std::uint32_t* extent, bool* result) {
    __try { *result=operation(context,buffer,nullptr,offset,extent); return 0; }
    __except(EXCEPTION_EXECUTE_HANDLER) { return static_cast<int>(GetExceptionCode()); }
}
int wmain(int argc, wchar_t** argv) {
    if (argc != 2) return 1;
    const auto image=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);
    if (!image) { std::cerr << "Map failed: " << GetLastError() << '\n'; return 2; }
    const auto base=reinterpret_cast<const std::uint8_t*>(image);
    const auto dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    const auto nt=reinterpret_cast<const IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
    const unsigned char signature[]{0x48,0x8b,0xc4,0x48,0x89,0x58,0x18,0x4c,0x89,0x48,0x20,0x55};
    if (nt->FileHeader.TimeDateStamp != 0x6a4236c8 || nt->OptionalHeader.SizeOfImage != 0x2da7000 ||
        std::memcmp(base+0x994810,signature,sizeof(signature))) return 3;
    WorldNative::TerrainOperation context{};
    std::array<float,256> rates{}; rates.fill(1);
    context.radius=1; context.cellSize=1;
    context.center[0]=context.center[1]=context.center[2]=.5f;
    context.materialCosts=rates.data(); context.materialCount=rates.size();
    WorldNative::TerrainCell cell{1,255};
    WorldNative::TerrainBuffer buffer{&cell,1,{1,1,1}};
    const std::uint32_t offset[3]{}, extent[3]{1,1,1};
    const auto operation=reinterpret_cast<WorldNative::ApplyTerrainBuffer>(const_cast<std::uint8_t*>(base)+0x994810);
    bool result=false;
    const auto error=Invoke(operation,&context,&buffer,offset,extent,&result);
    std::cout << "native buffer operation: exception=0x" << std::hex << error
        << " before=0xff01 after=0x" << WorldNative::EncodeCell(cell) << " return=" << result << '\n';
    if (error || WorldNative::EncodeCell(cell) != 0) return 4;
    std::array<WorldNative::TerrainCell,24> region;
    region.fill({5,255});
    buffer={region.data(),region.size(),{4,3,2}};
    const std::uint32_t subregion[3]{2,1,1};
    context.radius=.4f; context.cellSize=.5f;
    context.gridOrigin[0]=2; context.gridOrigin[1]=context.gridOrigin[2]=1;
    context.center[0]=1.25f; context.center[1]=context.center[2]=.75f;
    rates.fill(2);
    const auto suberror=Invoke(operation,&context,&buffer,subregion,extent,&result);
    std::cout << "native offset/density: exception=0x" << suberror
        << " index18=0x" << WorldNative::EncodeCell(region[18]) << '\n';
    bool unchanged=true;
    for (std::size_t i=0;i<region.size();++i)
        if (i != 18 && WorldNative::EncodeCell(region[i]) != 0xff05) unchanged=false;
    FreeLibrary(image);
    return suberror || !unchanged || WorldNative::EncodeCell(region[18]) != 0x7f05 ? 5 : 0;
}

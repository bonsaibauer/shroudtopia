// Read-only runtime sampling. No debugger attachment, code patch, DLL injection,
// or engine call. Each thread is resumed immediately after its register sample.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#include "../src/loader/world_native_layout.h"
#include <iostream>
#include <vector>
#include <cstdlib>

struct Thread {
    HANDLE handle;
    DWORD id;
};
int main(int argc, char** argv) {
    if (argc != 2) return 1;
    const DWORD pid = static_cast<DWORD>(std::strtoul(argv[1],nullptr,10));
    HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,FALSE,pid);
    if (!process) { std::cerr << "OpenProcess: " << GetLastError() << '\n'; return 2; }
    HANDLE modules = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid);
    MODULEENTRY32W module{sizeof(module)};
    if (!Module32FirstW(modules,&module)) return 3;
    const auto base = reinterpret_cast<std::uintptr_t>(module.modBaseAddr);
    CloseHandle(modules);
    const unsigned char expected[]{0x4d,0x8d,0x86,0x08,0x03,0x00,0x00};
    unsigned char actual[sizeof(expected)]{}; SIZE_T read=0;
    if (!ReadProcessMemory(process,reinterpret_cast<void*>(base+0x249cd8),actual,sizeof(actual),&read) ||
        read != sizeof(actual) || std::memcmp(actual,expected,sizeof(actual))) return 4;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
    THREADENTRY32 entry{sizeof(entry)};
    std::vector<Thread> threads;
    if (Thread32First(snapshot,&entry)) do {
        if (entry.th32OwnerProcessID != pid) continue;
        auto handle=OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT,FALSE,entry.th32ThreadID);
        if (handle) threads.push_back({handle,entry.th32ThreadID});
    } while (Thread32Next(snapshot,&entry));
    CloseHandle(snapshot);
    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_NO_PROMPTS);
    if (!SymInitialize(process,"",TRUE)) { std::cerr << "Symbol setup failed: " << GetLastError() << '\n'; return 6; }
    std::cerr << "Sampling " << threads.size() << " threads for 30 seconds.\n";
    const auto until = GetTickCount64()+30000;
    unsigned long long samples=0, matches=0;
    while (GetTickCount64() < until) {
        for (auto thread : threads) {
            CONTEXT context{}; context.ContextFlags=CONTEXT_CONTROL | CONTEXT_INTEGER;
            if (SuspendThread(thread.handle) == DWORD(-1)) continue;
            const bool got=GetThreadContext(thread.handle,&context) != FALSE;
            WorldNative::Cursor raw{};
            bool captured=false;
            auto rva=context.Rip-base;
            if (got && !(rva >= 0x249ca2 && rva < 0x24abee)) {
                STACKFRAME64 frame{};
                frame.AddrPC = {context.Rip,0,AddrModeFlat};
                frame.AddrStack = {context.Rsp,0,AddrModeFlat};
                frame.AddrFrame = {context.Rbp,0,AddrModeFlat};
                for (int depth=0; depth<24; ++depth) {
                    if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64,process,thread.handle,&frame,&context,nullptr,
                        SymFunctionTableAccess64,SymGetModuleBase64,nullptr)) break;
                    rva=frame.AddrPC.Offset-base;
                    if (rva >= 0x249ca2 && rva < 0x24abee) break;
                }
            }
            // R14 retains ClientPlayerInputData in this caller interval.
            if (got && rva >= 0x249ca2 && rva < 0x24abee) {
                captured=ReadProcessMemory(process,reinterpret_cast<void*>(context.R14+0x270),&raw,sizeof(raw),&read) && read==sizeof(raw);
            }
            ResumeThread(thread.handle);
            ++samples;
            if (!captured) continue;
            CursorSnapshot result{sizeof(result)};
            if (WorldNative::DecodeCursor({reinterpret_cast<const std::uint8_t*>(&raw),sizeof(raw)},&result) != RESULT_OK) continue;
            ++matches;
            std::cout << "{\"thread\":" << thread.id << ",\"rva\":" << rva
                << ",\"position\":[" << result.primary.position.x << ',' << result.primary.position.y << ',' << result.primary.position.z
                << "],\"flags\":" << unsigned(result.primary_flags) << ",\"material\":" << unsigned(result.material)
                << ",\"object\":" << result.selected_object.value << "}\n";
        }
        Sleep(1);
    }
    for (auto thread : threads) CloseHandle(thread.handle);
    SymCleanup(process);
    CloseHandle(process);
    std::cerr << "Register samples=" << samples << " cursor matches=" << matches << '\n';
    return matches ? 0 : 5;
}

// Read-only register sampler for the construction query called by the player
// input update. It never attaches a debugger, injects code, or calls the game.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

struct Thread { HANDLE handle{}; DWORD id{}; };

static bool Read(HANDLE process, std::uintptr_t address, void* target, std::size_t size) {
    SIZE_T read{};
    return ReadProcessMemory(process,reinterpret_cast<const void*>(address),target,size,&read) && read==size;
}

int main(int argc,char** argv) {
    if (argc < 2 || argc > 3) return 1;
    const DWORD pid=static_cast<DWORD>(std::strtoul(argv[1],nullptr,10));
    const unsigned seconds=argc==3 ? static_cast<unsigned>(std::strtoul(argv[2],nullptr,10)) : 30;
    HANDLE process=OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,FALSE,pid);
    if (!process) { std::cerr << "OpenProcess: " << GetLastError() << '\n'; return 2; }
    HANDLE modules=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid);
    MODULEENTRY32W module{sizeof(module)};
    if (!Module32FirstW(modules,&module)) return 3;
    const auto base=reinterpret_cast<std::uintptr_t>(module.modBaseAddr);
    CloseHandle(modules);

    const unsigned char expected[]{0x40,0x53,0x56,0x41,0x56,0x48,0x81,0xec,0x80,0x00,0x00,0x00};
    unsigned char actual[sizeof(expected)]{};
    if (!Read(process,base+0x3ecaf0,actual,sizeof(actual)) || std::memcmp(actual,expected,sizeof(actual))) {
        std::cerr << "Executable version mismatch at RVA 3ECAF0.\n"; return 4;
    }

    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
    THREADENTRY32 entry{sizeof(entry)};
    std::vector<Thread> threads;
    if (Thread32First(snapshot,&entry)) do {
        if (entry.th32OwnerProcessID != pid) continue;
        HANDLE handle=OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT,FALSE,entry.th32ThreadID);
        if (handle) threads.push_back({handle,entry.th32ThreadID});
    } while (Thread32Next(snapshot,&entry));
    CloseHandle(snapshot);

    std::set<std::uintptr_t> contexts;
    std::uint64_t samples{},hits{};
    std::cerr << "Sampling " << threads.size() << " threads for " << seconds << " seconds.\n";
    const auto until=GetTickCount64()+static_cast<ULONGLONG>(seconds)*1000;
    while (GetTickCount64()<until) {
        for (const auto& thread:threads) {
            CONTEXT context{}; context.ContextFlags=CONTEXT_CONTROL | CONTEXT_INTEGER;
            if (SuspendThread(thread.handle)==DWORD(-1)) continue;
            const bool got=GetThreadContext(thread.handle,&context)!=FALSE;
            ResumeThread(thread.handle);
            ++samples;
            if (!got || context.Rip < base+0x3ecaf0 || context.Rip >= base+0x3ecc50) continue;
            ++hits;
            // RCX is authoritative before `mov rsi,rcx` at +16; RSI afterwards.
            const auto rva=context.Rip-base;
            const auto candidate=rva < 0x3ecb03 ? context.Rcx : context.Rsi;
            if (candidate) contexts.insert(static_cast<std::uintptr_t>(candidate));
            std::cout << "hit thread=" << thread.id << " rva=0x" << std::hex << rva
                      << " context=0x" << candidate << " rdx=0x" << context.Rdx
                      << " r8=0x" << context.R8 << std::dec << '\n';
        }
        Sleep(0);
    }
    std::cout << "samples=" << samples << " hits=" << hits << " unique=" << contexts.size() << '\n';
    for (const auto address:contexts) {
        std::uintptr_t words[64]{};
        std::cout << "context 0x" << std::hex << address;
        if (Read(process,address,words,sizeof(words))) {
            for (unsigned i=0;i<64;++i) {
                if (words[i]) std::cout << " +" << std::hex << i*8 << "=0x" << words[i];
            }
        } else std::cout << " unreadable";
        std::cout << std::dec << '\n';
    }
    for (const auto& thread:threads) CloseHandle(thread.handle);
    CloseHandle(process);
    return contexts.empty() ? 5 : 0;
}

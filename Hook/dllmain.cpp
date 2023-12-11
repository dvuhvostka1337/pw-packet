#include <windows.h>
#include <detours.h>
#include <fstream>
#include <process.h>
#include <chrono>
#include <iomanip>

void WritePacketBytesToFile(void* packet, size_t len, const std::string& filename) {
    BYTE* bytes = static_cast<BYTE*>(packet);
    std::ofstream file(filename, std::ios_base::app);
    if (file.is_open()) {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        struct tm timeinfo;
        localtime_s(&timeinfo, &time);
        char timeStr[80];
        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

        file << timeStr << " - Length: " << len << std::endl;
        for (size_t i = 0; i < len; i++) {
            file << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(bytes[i]);
            if (i != len - 1) {
                file << " ";
            }
        }
        file << std::endl;

        uintptr_t address = reinterpret_cast<uintptr_t>(packet);
        file << "PacketAddress: 0x" << std::hex << std::uppercase << address << std::endl;
        file.close();
    }
}

// Original type func
// typedef void (__stdcall OrigFuncType)(void* Size, size_t size);
typedef int(__stdcall* pSendPacket) (void* Src, size_t Size);

// Get game address in memory
DWORD GameBase = (uintptr_t)GetModuleHandle(NULL);

DWORD SendPacketAddress = GameBase + 0x367CE0;
DWORD BaseAddress = GameBase + 0xA88F70;

// Original function
pSendPacket oSendPacket = (pSendPacket)(SendPacketAddress);

// unsigned char packetBytes[] = { 0x02, 0x00, 0xB0, 0x57, 0x0B, 0x00 };
// void* packet = static_cast<void*>(packetBytes);
// size_t len = sizeof(packetBytes);

// Hooked function
int __stdcall hSendPacket(void* packet, size_t len)
{
    // DEBUG Save packet's bytes to file
    WritePacketBytesToFile(packet, len, "output.txt");

    DWORD ecx_val = *(DWORD*)(*(DWORD*)BaseAddress + 0x20);
    __asm
    {
        push ecx
        push len
        push packet
        mov ecx, ecx_val
        call oSendPacket
        pop ecx
    }
}

/*
extern "C" __declspec(dllexport) int __stdcall CallSendPacketFromExternal() {
    DWORD ecx_val = *(DWORD*)(*(DWORD*)BaseAddress + 0x20);
    wprintf(L"ecx_val: %lu\n", BaseAddress);
    return 1;
}
*/

unsigned int __stdcall mainThread(void* pArguments)
{
    // Init Detours
    DetourRestoreAfterWith();
    // Start hook game function
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)oSendPacket, hSendPacket);
    DetourTransactionCommit();
    return S_OK; // exit current thread
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        HANDLE hThread;
        unsigned int threadId;
        hThread = (HANDLE)_beginthreadex(NULL, 0x1000, &mainThread, (void*)hModule, 0, &threadId);
        break;
    case DLL_PROCESS_DETACH:
        // Cancel detouring
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)oSendPacket, hSendPacket);
        DetourTransactionCommit();

        break;
    }
    return TRUE;
}
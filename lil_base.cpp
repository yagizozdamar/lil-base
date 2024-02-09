// Begin Windows
#include <iostream>
#include <chrono>
#include <thread>
#include <windows.h>
#include <stdlib.h>
#include <tlhelp32.h>

OPENFILENAME ofn;
char szFileName[MAX_PATH] = "";

#define _CRT_SECURE_NO_WARNINGS
// End Windows

// Begin Injection & Misc
#include <injection/Blackbone/Config.h>
#include <injection/Blackbone/PE/PEImage.h>
#include <injection/Blackbone/Misc/Utils.h>
#include <injection/Blackbone/Process/Process.h>

#pragma comment(lib, "C:/Users/Bendis/Desktop/lil_base/include/injection/BlackBone.lib")

using namespace blackbone;
blackbone::Process process;

#include <color.hpp>
#include <filesystem>
// End Injection & Misc

std::string programChoice;
static bool driverCheck = blackbone::Driver().EnsureLoaded();

static DWORD getPID(const std::string& processName) {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return 0;
    }

    do {
        std::filesystem::path path(pe32.szExeFile);
        std::string baseName = path.filename().stem().string();
        if (_stricmp(baseName.c_str(), processName.c_str()) == 0 ||
            _stricmp(pe32.szExeFile, processName.c_str()) == 0) {
            CloseHandle(hProcessSnap);
            return pe32.th32ProcessID;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return 0;
}

static void sleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

static void clearConsole() {
    system("cls");
}

static void inject(int pID, std::string programName, std::string dllPath) {
    std::wstring convertedPath(dllPath.begin(), dllPath.end()); // I feel violated
    std::wstring convertedName(programName.begin(), programName.end());

    process.Attach(convertedName.c_str());

    clearConsole();

    if (!blackbone::Driver().MmapDll(pID, convertedPath, KWipeHeader | KNoTLS)) {
        std::cout << dye::green("[+] ") << "Injection successful. You may close the program.";
        sleep(2500);
        exit(0);
    }
    else {
        std::cout << dye::red("[!] ") << "Looks like something went off with the injection. Please try again. Exiting in 2 seconds.";
        sleep(2500);
        exit(0);
    }
};

static void begin() {
    clearConsole();

    std::cout << dye::yellow("[*] ") << "Please write the program name you want to manual map inject your DLL to (with extension included such as .exe): ";
    std::cin >> std::noskipws >> programChoice;

    if (programChoice == "" || getPID(programChoice) == 0) {
        clearConsole();
        std::cout << dye::red("[!] ") << "Please write a valid program name. Closing program in 2 seconds...";
        sleep(2500);
    }

    clearConsole();

    std::cout << dye::aqua("[+] ") << "Gathering info about process...\n";
    sleep(900);
    std::cout << dye::aqua("\n[*] ") << "Clearing buffers...";
    sleep(1000);
    clearConsole();
    std::cout << dye::green("[+] ") << "Done. Please choose a file on the opened window:";
    sleep(500);

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Dynamic Link Libraries (*.dll)\0*.dll\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (GetOpenFileName(&ofn) == TRUE) { // User chose a valid path with lpstrFile being the path
        // MessageBox(NULL, ofn.lpstrFile, "", MB_OK);
        inject(process.pid(), programChoice, ofn.lpstrFile);
    }
}

int main()
{
    std::cout << dye::green("[+] ") << "Welcome to lil base. Please hold on...";
    sleep(2000); // Do whatever you want here (initialization or whatever)

    begin();
}

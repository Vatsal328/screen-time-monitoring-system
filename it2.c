#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <tlhelp32.h>

#define MAX_APPS 50
#define UPDATE_INTERVAL 1000  // Update every second

typedef struct {
    char processName[256];
    char appName[256];
    int timeSpent;
    int timeLimit;
    DWORD pid;  // Process ID to allow application closure
} AppUsage;

AppUsage appUsages[MAX_APPS];
int appCount = 0;
HWND hwndMain, hwndList;
HINSTANCE hInstance;

// Get friendly names for the applications, otherwise use the process name.
void GetAppFriendlyName(char *processName, char *friendlyName, int size) {
    if (strcmp(processName, "notepad.exe") == 0) {
        strncpy(friendlyName, "Notepad", size);
    } else if (strcmp(processName, "chrome.exe") == 0) {
        strncpy(friendlyName, "Google Chrome", size);
    } else {
        strncpy(friendlyName, processName, size);
    }
}


// Retrieve the active process name and PID of the application in focus.
int GetActiveProcessName(char *processName, int size, DWORD *pid) {
    HWND hwnd = GetForegroundWindow();
    if (hwnd) {
        GetWindowThreadProcessId(hwnd, pid);
        HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, *pid);
        if (process) {
            HMODULE hMod;
            DWORD cbNeeded;
            if (EnumProcessModules(process, &hMod, sizeof(hMod), &cbNeeded)) {
                GetModuleBaseName(process, hMod, processName, size);
            }
            CloseHandle(process);
            return 1;
        }
    }
    return 0;
}

// Find or add an application to the tracked list.
AppUsage* GetOrAddAppUsage(const char *processName, DWORD pid) {
    for (int i = 0; i < appCount; i++) {
        if (strcmp(appUsages[i].processName, processName) == 0) {
            return &appUsages[i];
        }
    }
    if (appCount < MAX_APPS) {
        strncpy(appUsages[appCount].processName, processName, sizeof(appUsages[appCount].processName) - 1);
        GetAppFriendlyName(processName, appUsages[appCount].appName, sizeof(appUsages[appCount].appName) - 1);
        appUsages[appCount].timeSpent = 0;
        appUsages[appCount].timeLimit = 60;  // Default 30-minute limit
        appUsages[appCount].pid = pid;  // Store the process ID
        return &appUsages[appCount++];
    }
    return NULL;
}

// Update the GUI list with all tracked application times.
void UpdateUsageList() {
    char buffer[256];
    SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < appCount; i++) {
        sprintf(buffer, "%s: %d sec (Limit: %d sec)", appUsages[i].appName, appUsages[i].timeSpent, appUsages[i].timeLimit);
        SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)buffer);
    }
}

// Function to terminate a process by its PID.
void TerminateProcessById(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess) {
        TerminateProcess(hProcess, 1);
        CloseHandle(hProcess);
    }
}

// Function to display a warning and terminate the application if the user agrees.
void ShowWarningAndTerminate(const char *appName, DWORD pid) {
    char warningText[512];
    sprintf(warningText, "Time limit reached for %s! Please take a break. Click OK to close the app.", appName);
    int result = MessageBox(hwndMain, warningText, "Warning", MB_ICONWARNING | MB_OKCANCEL);
    if (result == IDOK) {
        TerminateProcessById(pid);  // Terminate the process if "OK" is clicked
    }
}

// Window procedure to handle messages.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            hwndList = CreateWindow("LISTBOX", NULL, WS_VISIBLE | WS_CHILD | LBS_STANDARD, 10, 10, 400, 300, hwnd, NULL, hInstance, NULL);
            SetTimer(hwnd, 1, UPDATE_INTERVAL, NULL);
            break;
        case WM_TIMER: {
            char processName[256] = "";
            DWORD pid;
            if (GetActiveProcessName(processName, sizeof(processName), &pid)) {
                AppUsage *appUsage = GetOrAddAppUsage(processName, pid);
                if (appUsage) {
                    appUsage->timeSpent++;
                    UpdateUsageList();

                    // Check and display warning if time limit exceeded
                    if (appUsage->timeSpent >= appUsage->timeLimit) {
                        ShowWarningAndTerminate(appUsage->appName, appUsage->pid);
                        appUsage->timeSpent = 0;  // Reset or handle as needed
                    }
                }
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Main function to create and run the GUI.
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInstance = hInst;
    const char *CLASS_NAME = "AppUsageTracker";
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClass(&wc);

    hwndMain = CreateWindowEx(0, CLASS_NAME, "Application Usage Tracker", WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 450, 400,
                              NULL, NULL, hInstance, NULL);

    if (hwndMain == NULL) {
        return 0;
    }

    ShowWindow(hwndMain, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

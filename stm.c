#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <string.h>
#include <time.h>

#define APP_LIMIT_SECONDS 1800 
#define UPDATE_INTERVAL 1000 

typedef struct {
    char processName[256];
    int timeSpent;  // seconds
} AppUsage;

AppUsage appUsages[50];  // Array to hold usage data for multiple applications
int appCount = 0;
HWND hwndMain, hwndLabel;
time_t lastUpdateTime = 0;

// Function to get the process name of the currently active application
int GetActiveProcessName(char *processName, int size) {
    HWND hwnd = GetForegroundWindow();
    DWORD pid;
    HANDLE process;
    if (hwnd) {
        GetWindowThreadProcessId(hwnd, &pid);
        process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
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

// Function to find or add an application to the tracked list
AppUsage* GetOrAddAppUsage(const char *processName) {
    for (int i = 0; i < appCount; i++) {
        if (strcmp(appUsages[i].processName, processName) == 0) {
            return &appUsages[i];
        }
    }
    // If not found, add a new entry
    if (appCount < sizeof(appUsages) / sizeof(AppUsage)) {
        strncpy(appUsages[appCount].processName, processName, sizeof(appUsages[appCount].processName) - 1);
        appUsages[appCount].timeSpent = 0;
        return &appUsages[appCount++];
    }
    return NULL;
}

// Function to update the GUI label with all tracked application times
void UpdateUsageLabel() {
    char buffer[1024] = "Time spent on applications:\n";
    char timeText[256];
    for (int i = 0; i < appCount; i++) {
        sprintf(timeText, "%s: %d seconds\n", appUsages[i].processName, appUsages[i].timeSpent);
        strcat(buffer, timeText);
    }
    SetWindowText(hwndLabel, buffer);
}

// Function to display a warning message
void ShowWarning(const char *processName) {
    char warningText[512];
    sprintf(warningText, "Time limit reached for %s! Please take a break.", processName);
    MessageBox(hwndMain, warningText, "Warning", MB_ICONWARNING | MB_OK);
}

// Window procedure to handle messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            hwndLabel = CreateWindow("STATIC", "", WS_VISIBLE | WS_CHILD | SS_LEFT, 10, 10, 400, 300, hwnd, NULL, NULL, NULL);
            SetWindowText(hwndLabel, "Tracking time spent on applications...");
            break;
        case WM_TIMER: {
            char processName[256] = "";
            if (GetActiveProcessName(processName, sizeof(processName))) {
                AppUsage *appUsage = GetOrAddAppUsage(processName);
                if (appUsage) {
                    appUsage->timeSpent++;
                    UpdateUsageLabel();

                    // Show warning if time limit exceeded for any app
                    if (appUsage->timeSpent >= APP_LIMIT_SECONDS) {
                        ShowWarning(appUsage->processName);
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

// Main function to create and run the GUI
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
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

    // Set a timer to check the application usage every second
    SetTimer(hwndMain, 1, UPDATE_INTERVAL, NULL);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

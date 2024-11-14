#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <string.h>
#include <tlhelp32.h>

#define MAX_APPS 50
#define UPDATE_INTERVAL 1000  // Update every second

typedef struct {
    char processName[256];
    char appName[256];
    int timeSpent;
    int timeLimit;
    DWORD pid;  // Process ID to allow application closure
    BOOL popupShown;  // Flag to track if popup has been shown for this app
} AppUsage;

AppUsage appUsages[MAX_APPS];
int appCount = 0;
HWND hwndMain, hwndList, hwndTop5List, hwndIncButton, hwndDecButton;
HINSTANCE hInstance;
int selectedAppIndex = -1;

// Check if the process is a user-opened application (exclude system processes).
BOOL IsUserApplication(DWORD pid) {
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (process) {
        DWORD priorityClass = GetPriorityClass(process);
        CloseHandle(process);
        return (priorityClass == NORMAL_PRIORITY_CLASS || priorityClass == HIGH_PRIORITY_CLASS);
    }
    return FALSE;
}

// Map process name to application-friendly name.
void GetAppFriendlyName(const char *processName, char *friendlyName, int size) {
    if (strcmp(processName, "notepad.exe") == 0) {
        strncpy(friendlyName, "Notepad", size);
    } else if (strcmp(processName, "chrome.exe") == 0) {
        strncpy(friendlyName, "Google Chrome", size);
    } else if (strcmp(processName, "EXPLORER.EXE") == 0) {
        strncpy(friendlyName, "File Explorer", size);
    } else if (strcmp(processName, "firefox.exe") == 0) {
        strncpy(friendlyName, "Mozilla Firefox", size);
    } else if (strcmp(processName, "outlook.exe") == 0) {
        strncpy(friendlyName, "Microsoft Outlook", size);
    } else if (strcmp(processName, "spotify.exe") == 0) {
        strncpy(friendlyName, "Spotify", size);
    } else if (strcmp(processName, "teams.exe") == 0) {
        strncpy(friendlyName, "Microsoft Teams", size);
    } else if (strcmp(processName, "zoom.exe") == 0) {
        strncpy(friendlyName, "Zoom", size);
    } else if (strcmp(processName, "vlc.exe") == 0) {
        strncpy(friendlyName, "VLC Media Player", size);
    } else if (strcmp(processName, "discord.exe") == 0) {
        strncpy(friendlyName, "Discord", size);
    } else if (strcmp(processName, "skype.exe") == 0) {
        strncpy(friendlyName, "Skype", size);
    } else if (strcmp(processName, "mspmsn.exe") == 0) {
        strncpy(friendlyName, "Windows Messenger", size);
    } else if (strcmp(processName, "steam.exe") == 0) {
        strncpy(friendlyName, "Steam", size);
    } else if (strcmp(processName, "sublime_text.exe") == 0) {
        strncpy(friendlyName, "Sublime Text", size);
    } else if (strcmp(processName, "visualstudio.exe") == 0) {
        strncpy(friendlyName, "Visual Studio", size);
    } else if (strcmp(processName, "wordpad.exe") == 0) {
        strncpy(friendlyName, "WordPad", size);
    } else if (strcmp(processName, "msedge.exe") == 0) {
        strncpy(friendlyName, "Microsoft Edge", size);
    } else if (strcmp(processName, "iexplore.exe") == 0) {
        strncpy(friendlyName, "Internet Explorer", size);
    } else if (strcmp(processName, "Explorer.EXE") == 0) {
        strncpy(friendlyName, "Windows Explorer", size);
    } 
    else if (strcmp(processName, "Code.exe") == 0) {
        strncpy(friendlyName, "VS Code", size);
    } else {
        strncpy(friendlyName, processName, size);  // Default to process name if no match
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

// Update the AppUsage initialization
AppUsage* GetOrAddAppUsage(const char *processName, DWORD pid) {
    if (strcmp(processName, "it2.exe") == 0) {
        return NULL;  // Skip tracking this process
    }

    for (int i = 0; i < appCount; i++) {
        if (strcmp(appUsages[i].processName, processName) == 0) {
            return &appUsages[i];
        }
    }
    if (appCount < MAX_APPS) {
        strncpy(appUsages[appCount].processName, processName, sizeof(appUsages[appCount].processName) - 1);
        GetAppFriendlyName(processName, appUsages[appCount].appName, sizeof(appUsages[appCount].appName) - 1);
        appUsages[appCount].timeSpent = 0;
        appUsages[appCount].timeLimit = 60;  // Default 60-second limit
        appUsages[appCount].pid = pid;
        appUsages[appCount].popupShown = FALSE;  // Initialize flag to FALSE
        return &appUsages[appCount++];
    }
    return NULL;
}

// Sorts applications by time spent in descending order and updates the top 5 list.
void UpdateTop5List() {
    AppUsage top5[MAX_APPS];
    memcpy(top5, appUsages, appCount * sizeof(AppUsage));
    
    // Sort by timeSpent (descending)
    for (int i = 0; i < appCount - 1; i++) {
        for (int j = i + 1; j < appCount; j++) {
            if (top5[i].timeSpent < top5[j].timeSpent) {
                AppUsage temp = top5[i];
                top5[i] = top5[j];
                top5[j] = temp;
            }
        }
    }

    // Update the top 5 list box
    SendMessage(hwndTop5List, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < 5 && i < appCount; i++) {
        char buffer[256];
        sprintf(buffer, "%s: %d sec", top5[i].appName, top5[i].timeSpent);
        SendMessage(hwndTop5List, LB_ADDSTRING, 0, (LPARAM)buffer);
    }
}

// Update the GUI list with all tracked application times.
void UpdateUsageList() {
    char buffer[256];
    SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < appCount; i++) {
        sprintf(buffer, "%s: %d sec (Limit: %d sec)", appUsages[i].appName, appUsages[i].timeSpent, appUsages[i].timeLimit);
        SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)buffer);
    }
    UpdateTop5List();
}

// Handles increment and decrement of time limits based on selection.
void AdjustTimeLimit(int adjustment) {
    if (selectedAppIndex >= 0 && selectedAppIndex < appCount) {
        appUsages[selectedAppIndex].timeLimit += adjustment;
        UpdateUsageList();
    }
}

// Window procedure to handle messages.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            hwndList = CreateWindow("LISTBOX", NULL, WS_VISIBLE | WS_CHILD | LBS_STANDARD, 10, 10, 400, 200, hwnd, NULL, hInstance, NULL);
            hwndTop5List = CreateWindow("LISTBOX", NULL, WS_VISIBLE | WS_CHILD | LBS_STANDARD, 10, 220, 400, 100, hwnd, NULL, hInstance, NULL);
            hwndIncButton = CreateWindow("BUTTON", "+", WS_VISIBLE | WS_CHILD, 420, 10, 30, 30, hwnd, (HMENU)1, hInstance, NULL);
            hwndDecButton = CreateWindow("BUTTON", "-", WS_VISIBLE | WS_CHILD, 420, 50, 30, 30, hwnd, (HMENU)2, hInstance, NULL);
            SetTimer(hwnd, 1, UPDATE_INTERVAL, NULL);
            break;
        case WM_COMMAND:
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                selectedAppIndex = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
            } else if (LOWORD(wParam) == 1) {  // Increment button
                AdjustTimeLimit(10);  // Increase limit by 10 sec
            } else if (LOWORD(wParam) == 2) {  // Decrement button
                AdjustTimeLimit(-10);  // Decrease limit by 10 sec
            }
            break;
        // Modify the WM_TIMER case to check for popup shown status
case WM_TIMER: {
    char processName[256] = "";
    DWORD pid;
    if (GetActiveProcessName(processName, sizeof(processName), &pid) && IsUserApplication(pid)) {
        AppUsage *appUsage = GetOrAddAppUsage(processName, pid);
        if (appUsage) {
            appUsage->timeSpent++;

            // Check if the time limit is exceeded and popup has not been shown
            if (appUsage->timeSpent >= appUsage->timeLimit && !appUsage->popupShown) {
                appUsage->popupShown = TRUE;  // Mark the popup as shown
                MessageBox(hwnd, "Time limit exceeded!", "Warning", MB_OK | MB_ICONEXCLAMATION);
            }

            UpdateUsageList();
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
                              CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
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

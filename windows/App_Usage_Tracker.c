#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <string.h>
#include <tlhelp32.h>
#include "resource.h"

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

// Checking if the process is a user-opened application (exclude system processes).
BOOL IsUserApplication(DWORD pid) {
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (process) {
        DWORD priorityClass = GetPriorityClass(process);
        CloseHandle(process);
        return (priorityClass == NORMAL_PRIORITY_CLASS || priorityClass == HIGH_PRIORITY_CLASS);
    }
    return FALSE;
}

// Mapping process name to application-friendly name.
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
    } else if (strcmp(processName, "Spotify.exe") == 0) {
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
    }
    else if (strcmp(processName, "olk.exe") == 0) {
        strncpy(friendlyName, "Microsoft Outlook", size);
    } else {
        strncpy(friendlyName, processName, size);  // Default to process name if no match
    }
}

// Retrieving the active process name and PID of the application in focus.
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

// Retrieving  AppUsage for a specific process
AppUsage* GetOrAddAppUsage(const char *processName, DWORD pid) {
    if (strcmp(processName, "AppUsageTracker.exe") == 0 || strcmp(processName, "ShellExperienceHost.exe") == 0 || strcmp(processName, "WindowsTerminal.exe") == 0 || strcmp(processName, "SearchHost.exe") == 0) {
        return NULL;  // Skipping tracking this process
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
        appUsages[appCount].popupShown = FALSE;
        return &appUsages[appCount++];
    }
    return NULL;
}

// InputBox: it prompts the user to input a new time limit
BOOL InputBox(HWND hwndOwner, const char *title, const char *prompt, int *timeLimit) {
    char inputBuffer[32];
    sprintf(inputBuffer, "%d", *timeLimit);

    char message[256];
    sprintf(message, "%s\n\nCurrent limit: %d seconds", prompt, *timeLimit);

    int result = MessageBox(hwndOwner, message, title, MB_OKCANCEL);
    if (result == IDOK) {
        // Simulate user input by increasing the limit
        *timeLimit += 60;  // Increment by 60 seconds
        return TRUE;
    }
    return FALSE;
}

// Update the top 5 list
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
// Update the usage list
void UpdateUsageList() {
    char buffer[256];
    SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < appCount; i++) {
        sprintf(buffer, "%s: %d sec (Limit: %d sec)", appUsages[i].appName, appUsages[i].timeSpent, appUsages[i].timeLimit);
        SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)buffer);
    }
    UpdateTop5List();
}

// time limit for the specific app is incremented
void AdjustTimeLimit(int adjustment) {
    if (selectedAppIndex >= 0 && selectedAppIndex < appCount) {
        appUsages[selectedAppIndex].timeLimit += adjustment;
        UpdateUsageList();
    }
}

// application warnings and timers are handled
void HandleAppWarnings(AppUsage *appUsage) {
    if (appUsage->timeSpent >= appUsage->timeLimit && !appUsage->popupShown) {
        appUsage->popupShown = TRUE;      // set it to true so that the warning is shown only one time

        // Bringing the warning box to the foreground
        SetForegroundWindow(hwndMain);

        KillTimer(hwndMain, 1); // Stop the timer

        //message box is displayed to the user when it exceeds the time limit
        int result = MessageBox(hwndMain, "Time limit exceeded. Do you want to close the app?", 
                                "Warning", MB_YESNOCANCEL | MB_ICONEXCLAMATION);

        // Handle user choice
        if (result == IDYES) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, appUsage->pid);
            //close the process
            if (hProcess) {
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
            }
        } else if (result == IDCANCEL) {          //adjust the time limit
            InputBox(hwndMain, "Adjust Time Limit", "Increase the time limit:", &appUsage->timeLimit);
        }

        // Resume tracking
        SetTimer(hwndMain, 1, UPDATE_INTERVAL, NULL);
        appUsage->popupShown = FALSE;  // Reset popup flag

    }
}


// Update the selected app index and handle time adjustment
void UpdateSelectionAndAdjustTimeLimit(HWND hwnd, WPARAM wParam) {
    //case when only selection is changed
    if (HIWORD(wParam) == LBN_SELCHANGE) {
        selectedAppIndex = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
    } else if (LOWORD(wParam) == 1) {  // case for Increment button
        if (selectedAppIndex >= 0 && selectedAppIndex < appCount) {
            appUsages[selectedAppIndex].timeLimit += 10;  // Increase limit by 10 sec
            UpdateUsageList();
        }
    } else if (LOWORD(wParam) == 2) {  // case for Decrement button
        if (selectedAppIndex >= 0 && selectedAppIndex < appCount) {
            appUsages[selectedAppIndex].timeLimit -= 10;  // Decrease limit by 10 sec
            UpdateUsageList();
        }
    }
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
    // Creating the heading for the main list
    CreateWindow(
        "STATIC", "App Usage Details", 
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        10, 0, 400, 20, // Place at the top
        hwnd, NULL, hInstance, NULL
    );

    // Creating the main ListBox
    hwndList = CreateWindow(
        "LISTBOX", NULL,
        WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY,
        10, 20, 400, 180,
        hwnd, NULL, hInstance, NULL
    );

    // Creating the heading for the top 5 list
    CreateWindow(
        "STATIC", "Most Used Apps", 
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        10, 210, 400, 20,
        hwnd, NULL, hInstance, NULL
    );

    // Creating the Top 5 ListBox
    hwndTop5List = CreateWindow(
        "LISTBOX", NULL,
        WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY,
        10, 230, 400, 100,
        hwnd, NULL, hInstance, NULL
    );

            hwndIncButton = CreateWindow("BUTTON", "+", WS_VISIBLE | WS_CHILD, 
                                         420, 10, 30, 30, hwnd, (HMENU)1, hInstance, NULL);
            hwndDecButton = CreateWindow("BUTTON", "-", WS_VISIBLE | WS_CHILD, 
                                         420, 50, 30, 30, hwnd, (HMENU)2, hInstance, NULL);
            SetTimer(hwnd, 1, UPDATE_INTERVAL, NULL);
            break;
        case WM_COMMAND:
            UpdateSelectionAndAdjustTimeLimit(hwnd, wParam);
            break;
        case WM_TIMER:
            {
                char processName[256] = "";
                DWORD pid;
                if (GetActiveProcessName(processName, sizeof(processName), &pid) && IsUserApplication(pid)) {
                    AppUsage *appUsage = GetOrAddAppUsage(processName, pid);
                    if (appUsage) {
                        appUsage->timeSpent++;
                        HandleAppWarnings(appUsage);
                        UpdateUsageList();
                    }
                }
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}


// Main function
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInstance = hInst;
    const char *CLASS_NAME = "AppUsageTracker";
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON)); // for custom icon

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


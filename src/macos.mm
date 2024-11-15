#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <chrono>

using namespace std;
using namespace std::chrono;

struct AppUsage {
    string appName;
    int totalSeconds = 0;
    int usageLimit = -1;
    bool limitReached = false;

    AppUsage() = default;
    AppUsage(const string& name) : appName(name) {}
};

unordered_map<string, AppUsage> appUsageMap;
string currentApp;
auto lastSwitchTime = steady_clock::now();
NSTimer *usageTimer;

// Function to send a notification when the usage limit is reached
void sendNotification(const string& appName) {
    string script = "osascript -e 'display notification \"" + appName +
                    " has reached its usage limit.\" with title \"App Usage Limit Reached\"'";
    system(script.c_str());
}

// Attempt to terminate the application
void terminateApp(const string& appName) {
    NSArray<NSRunningApplication *> *runningApps = [[NSWorkspace sharedWorkspace] runningApplications];
    for (NSRunningApplication *app in runningApps) {
        if ([[app localizedName] isEqualToString:[NSString stringWithUTF8String:appName.c_str()]]) {
            [app terminate];
            cout << "Terminated application: " << appName << endl;
            break;
        }
    }
}

// Show a dialog box and terminate the app
void showDialogAndTerminate(const string& appName) {
    cout << "Attempting to show dialog on main thread for: " << appName << endl;
    dispatch_async(dispatch_get_main_queue(), ^{
        @autoreleasepool {
            cout << "Inside dispatch_async main queue block for: " << appName << endl;
            NSAlert *alert = [[NSAlert alloc] init];
            alert.messageText = @"App Usage Limit Reached";
            alert.informativeText = [NSString stringWithFormat:@"%s has reached its usage limit. The app will now terminate.", appName.c_str()];
            [alert addButtonWithTitle:@"OK"];

            NSModalResponse response = [alert runModal];
            cout << "Dialog response received: " << response << endl;

            if (response == NSAlertFirstButtonReturn) {
                cout << "User acknowledged dialog. Terminating app: " << appName << endl;
                terminateApp(appName);
            } else {
                cout << "User dismissed dialog without acknowledgment." << endl;
            }
        }
    });
}

// Update application usage time and check if limit is reached
void updateAppUsage(const string& appName, int duration) {
    // cout << "Updating usage for: " << appName << " with duration: " << duration << " seconds" << endl;
    appUsageMap[appName].appName = appName;
    appUsageMap[appName].totalSeconds += duration;

    if (appUsageMap[appName].usageLimit != -1 &&
        appUsageMap[appName].totalSeconds >= appUsageMap[appName].usageLimit &&
        !appUsageMap[appName].limitReached) {
        
        cout << "Usage limit reached for " << appName << "! Sending notification and showing dialog." << endl;
        sendNotification(appName);
        showDialogAndTerminate(appName);

        appUsageMap[appName].limitReached = true;
    }
}

// Periodically check the usage
void checkUsage() {
    auto now = steady_clock::now();
    int duration = duration_cast<seconds>(now - lastSwitchTime).count();

    if (!currentApp.empty() && duration > 0) {
        updateAppUsage(currentApp, duration);
        lastSwitchTime = now; // Only update after usage is added
    }
}


void displayUsageStatistics() {
    cout << "\nApplication Usage Statistics:" << endl;
    cout << "------------------------------------" << endl;
    for (const auto& entry : appUsageMap) {
        cout << entry.second.appName << ": " << entry.second.totalSeconds << " seconds";
        if (entry.second.usageLimit != -1) {
            cout << " (Limit: " << entry.second.usageLimit << " seconds)";
        }
        cout << endl;
    }
    cout << "------------------------------------" << endl;
}

void activeAppDidChange(NSNotification *notification) {
    auto now = steady_clock::now();
    int duration = duration_cast<seconds>(now - lastSwitchTime).count();

    if (!currentApp.empty()) {
        updateAppUsage(currentApp, duration);
    }

    NSRunningApplication *app = notification.userInfo[NSWorkspaceApplicationKey];
    currentApp = [app.localizedName UTF8String];

    cout << "Active application changed to: " << currentApp << endl;

    lastSwitchTime = now;
    displayUsageStatistics();
}

int main() {
    @autoreleasepool {
        cout << "Program started" << endl;

        // Ask the user for app limits
        cout << "Enter application usage limits in seconds (type 'done' to finish):" << endl;
        while (true) {
            string appName;
            int limit;
            cout << "App name: ";
            getline(cin, appName);
            if (appName == "done") break;
            cout << "Usage limit (in seconds): ";
            cin >> limit;
            cin.ignore();
            appUsageMap[appName] = AppUsage(appName);
            appUsageMap[appName].usageLimit = limit;
        }

        [[[NSWorkspace sharedWorkspace] notificationCenter]
            addObserverForName:NSWorkspaceDidActivateApplicationNotification
                        object:nil
                         queue:[NSOperationQueue mainQueue]
                    usingBlock:^(NSNotification *notification) {
            activeAppDidChange(notification);
        }];

        // Start the usage timer
        usageTimer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                                    repeats:YES
                                                        block:^(NSTimer *timer) {
            checkUsage(); // Periodically check and update usage
        }];

        [[NSRunLoop mainRunLoop] run];
    }
    return 0;
}

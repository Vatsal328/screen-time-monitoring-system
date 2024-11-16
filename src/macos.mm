#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>

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
string currentApp = "Terminal";  // Placeholder for current app
auto lastSwitchTime = steady_clock::now();
bool running = true;
mutex appUsageMutex;

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
    dispatch_async(dispatch_get_main_queue(), ^{
        @autoreleasepool {
            NSAlert *alert = [[NSAlert alloc] init];
            alert.messageText = @"App Usage Limit Reached";
            alert.informativeText = [NSString stringWithFormat:@"%s has reached its usage limit. The app will now terminate.", appName.c_str()];
            [alert addButtonWithTitle:@"OK"];

            NSModalResponse response = [alert runModal];
            if (response == NSAlertFirstButtonReturn) {
                terminateApp(appName);
            }
        }
    });
}

// Update application usage and check for limits
void checkAppUsage() {
    auto now = steady_clock::now();
    int duration = duration_cast<seconds>(now - lastSwitchTime).count();

    lock_guard<mutex> lock(appUsageMutex); // Protect shared data

    if (!currentApp.empty() && duration > 0) {
        // Ensure the app entry exists in the map and update appName
        if (appUsageMap.find(currentApp) == appUsageMap.end()) {
            appUsageMap[currentApp] = AppUsage(currentApp);
        }
        appUsageMap[currentApp].appName = currentApp;
        appUsageMap[currentApp].totalSeconds += duration;

        // Update the last switch time
        lastSwitchTime = now;

        // Check if usage limit is exceeded
        if (appUsageMap[currentApp].usageLimit != -1 &&
            appUsageMap[currentApp].totalSeconds >= appUsageMap[currentApp].usageLimit &&
            !appUsageMap[currentApp].limitReached) {

            appUsageMap[currentApp].limitReached = true;
            sendNotification(currentApp);
            showDialogAndTerminate(currentApp);
        }
    }
}

// Live statistics display
// Live statistics display
void liveStatistics() {
    while (running) {
        checkAppUsage(); // Perform limit check and update usage time

        // Clear the terminal screen
        system("clear");

        // Display updated statistics
        lock_guard<mutex> lock(appUsageMutex); // Protect shared data
        cout << "\nLive Application Usage Statistics:" << endl;
        cout << "------------------------------------" << endl;
        for (const auto& entry : appUsageMap) {
            const string& appName = entry.second.appName.empty() ? entry.first : entry.second.appName;
            cout << appName << ": " << entry.second.totalSeconds << " seconds";
            if (entry.second.usageLimit != -1) {
                cout << " (Limit: " << entry.second.usageLimit << " seconds)";
            }
            cout << endl;
        }
        cout << "------------------------------------" << endl;

        this_thread::sleep_for(chrono::seconds(1));
    }
}

// Handle active app change notifications
void activeAppDidChange(NSNotification *notification) {
    auto now = steady_clock::now();
    int duration = duration_cast<seconds>(now - lastSwitchTime).count();

    lock_guard<mutex> lock(appUsageMutex); // Protect shared data

    // Update usage time for the previously active app
    if (!currentApp.empty() && currentApp != "Terminal") {
        appUsageMap[currentApp].totalSeconds += duration;
    }

    // Get the name of the newly active app
    NSRunningApplication *app = notification.userInfo[NSWorkspaceApplicationKey];
    currentApp = [app.localizedName UTF8String];

    // Ensure the app name is stored in the map
    if (!appUsageMap[currentApp].appName.empty() && appUsageMap[currentApp].appName != currentApp) {
        appUsageMap[currentApp] = AppUsage(currentApp);
    }

    lastSwitchTime = now;
}

// Main function
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

        // Initialize lastSwitchTime
        lastSwitchTime = steady_clock::now();

        // Start live statistics thread
        thread statsThread(liveStatistics);

        [[[NSWorkspace sharedWorkspace] notificationCenter]
            addObserverForName:NSWorkspaceDidActivateApplicationNotification
                        object:nil
                         queue:[NSOperationQueue mainQueue]
                    usingBlock:^(NSNotification *notification) {
            activeAppDidChange(notification);
        }];

        // Main run loop
        [[NSRunLoop mainRunLoop] run];

        // Stop live statistics when exiting
        running = false;
        statsThread.join();
    }
    return 0;
}
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

// Structure to track application usage details
struct AppUsage {
    string appName;      
    int totalSeconds = 0; 
    int usageLimit = -1;  
    bool limitReached = false;

    AppUsage() = default;
    AppUsage(const string& name) : appName(name) {}
};

unordered_map<string, AppUsage> appUsageMap; // Maps app names to usage data
string currentApp = "Terminal";  
auto lastSwitchTime = steady_clock::now(); // Timestamp of last app switch
bool running = true;             // Control variable for threads
mutex appUsageMutex;             // Mutex to synchronize shared data

// Sends a macOS notification when an app's usage limit is reached
void sendNotification(const string& appName) {
    string script = "osascript -e 'display notification \"" + appName +
                    " has reached its usage limit.\" with title \"App Usage Limit Reached\"'";
    system(script.c_str());
}

// Terminates the specified application
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

// Displays a dialog box and terminates the application
void showDialogAndTerminate(const string& appName) {
    dispatch_async(dispatch_get_main_queue(), ^{
        @autoreleasepool {
            NSAlert *alert = [[NSAlert alloc] init];
            alert.messageText = @"App Usage Limit Reached";
            alert.informativeText = [NSString stringWithFormat:@"%s has reached its usage limit. The app will now terminate.", appName.c_str()];
            [alert addButtonWithTitle:@"OK"];

            if ([alert runModal] == NSAlertFirstButtonReturn) {
                terminateApp(appName);
            }
        }
    });
}

// Tracks and checks app usage, triggers actions if limits are exceeded
void checkAppUsage() {
    auto now = steady_clock::now();
    int duration = duration_cast<seconds>(now - lastSwitchTime).count();

    lock_guard<mutex> lock(appUsageMutex);

    if (!currentApp.empty() && duration > 0) {
        if (appUsageMap.find(currentApp) == appUsageMap.end()) {
            appUsageMap[currentApp] = AppUsage(currentApp);
        }

        appUsageMap[currentApp].totalSeconds += duration;
        lastSwitchTime = now;

        if (appUsageMap[currentApp].usageLimit != -1 &&
            appUsageMap[currentApp].totalSeconds >= appUsageMap[currentApp].usageLimit &&
            !appUsageMap[currentApp].limitReached) {

            appUsageMap[currentApp].limitReached = true;
            sendNotification(currentApp);
            showDialogAndTerminate(currentApp);
        }
    }
}

// Continuously displays live app usage statistics
void liveStatistics() {
    while (running) {
        checkAppUsage();

        system("clear"); 

        lock_guard<mutex> lock(appUsageMutex);
        cout << "\nLive Application Usage Statistics:" << endl;
        cout << "------------------------------------" << endl;
        for (const auto& entry : appUsageMap) {
            cout << entry.second.appName << ": " << entry.second.totalSeconds << " seconds";
            if (entry.second.usageLimit != -1) {
                cout << " (Limit: " << entry.second.usageLimit << " seconds)";
            }
            cout << endl;
        }
        cout << "------------------------------------" << endl;

        this_thread::sleep_for(chrono::seconds(1));
    }
}

// Handles app switch events and updates the active app
void activeAppDidChange(NSNotification *notification) {
    auto now = steady_clock::now();
    int duration = duration_cast<seconds>(now - lastSwitchTime).count();

    lock_guard<mutex> lock(appUsageMutex);

    if (!currentApp.empty() && currentApp != "Terminal") {
        appUsageMap[currentApp].totalSeconds += duration;
    }

    NSRunningApplication *app = notification.userInfo[NSWorkspaceApplicationKey];
    currentApp = [app.localizedName UTF8String];

    if (appUsageMap.find(currentApp) == appUsageMap.end()) {
        appUsageMap[currentApp] = AppUsage(currentApp);
    }

    lastSwitchTime = now;
}

int main() {
    @autoreleasepool {
        cout << "Program started" << endl;

        // User inputs application usage limits
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

        lastSwitchTime = steady_clock::now();

        // Start live statistics display thread
        thread statsThread(liveStatistics);

        // Register app switch event listener
        [[[NSWorkspace sharedWorkspace] notificationCenter]
            addObserverForName:NSWorkspaceDidActivateApplicationNotification
                        object:nil
                         queue:[NSOperationQueue mainQueue]
                    usingBlock:^(NSNotification *notification) {
            activeAppDidChange(notification);
        }];

        [[NSRunLoop mainRunLoop] run];

        running = false;
        statsThread.join();
    }
    return 0;
}

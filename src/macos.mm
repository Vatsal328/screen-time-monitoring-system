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
};

unordered_map<string, AppUsage> appUsageMap;
string currentApp;
auto lastSwitchTime = steady_clock::now();

void updateAppUsage(const string& appName, int duration) {
    cout << "Updating usage for: " << appName << " with duration: " << duration << " seconds" << endl;
    appUsageMap[appName].appName = appName;
    appUsageMap[appName].totalSeconds += duration;
}

void displayUsageStatistics() {
    cout << "\nApplication Usage Statistics:" << endl;
    cout << "------------------------------------" << endl;
    for (const auto& entry : appUsageMap) {
        cout << entry.second.appName << ": " << entry.second.totalSeconds << " seconds" << endl;
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

        [[[NSWorkspace sharedWorkspace] notificationCenter]
            addObserverForName:NSWorkspaceDidActivateApplicationNotification
                        object:nil
                         queue:[NSOperationQueue mainQueue]
                    usingBlock:^(NSNotification *notification) {
            activeAppDidChange(notification);
        }];

        [[NSRunLoop mainRunLoop] run];
    }
    return 0;
}

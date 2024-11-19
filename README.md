# Screen Time Monitoring System

## Description  
The **Screen Time Monitoring System** is a cross-platform application designed to track and manage the time users spend on their devices. It supports **Windows** and **macOS**, providing tailored functionalities for each operating system. Users can monitor active application usage, set time limits, and receive alerts when limits are exceeded. 

---

## Features  

### Common Features  
- **Active Application Tracking**: Monitors the time spent on actively opened user applications.  
- **Time Limit Functionality**: Users can set and adjust time limits for each application.  
- **Notifications on Time Exceed**: Alerts users when an application's usage exceeds the set limit.  

### Windows-Specific Features  
- **Top 5 Most Used Applications**: Displays the five most used applications based on time spent.  
- **Friendly Application Names**: Converts process names like `notepad.exe` to user-friendly names such as "Notepad."  
- **Dynamic Time Limit Modification**: Allows users to increment or decrement time limits dynamically.  

### macOS-Specific Features  
- **Live Usage Statistics**: Displays real-time statistics of all monitored applications in the terminal.  
- **Valid for All Applications**: Automatically tracks all user applications without needing pre-configuration.  
- **User-Friendly Names**: Application names are displayed without additional configurations.  
- **Dialog Box and Termination**: When a time limit is exceeded, a notification and dialog box are shown. Pressing "OK" terminates the application.  

---

## Installation  

### Requirements  

#### Windows  
- **Operating System**: Windows 7 or later.  
- **Compiler**: A C compiler (e.g., GCC).  
- **Library**: `psapi` for interacting with system processes.  

#### macOS  
- **Operating System**: macOS Mojave or later.  
- **Compiler**: A C++ compiler supporting Objective-C++ (e.g., `clang`).  

---

### Installation Steps  

#### Windows  
1. Clone the repository or download the project files.  
2. Open a terminal/command prompt and navigate to the project directory.  
3. Compile the code:  
    ```bash
    gcc -o screen-time-monitoring-system AppUsageTracker.c -lpsapi
    ```  
4. Run the program:  
    ```bash
    ./screen-time-monitoring-system
    ```  
Alternatively, use the `build.bat` file to create the application and then open the .exe file to use it.  

#### macOS  
1. Clone the repository or download the project files.  
2. Open a terminal and navigate to the project directory.  
3. Compile the code with `clang`:  
    ```bash
    clang++ -o screen-time-monitoring-system screen_time_monitoring.cpp -framework AppKit -framework Foundation
    ```  
4. Run the program:  
    ```bash
    ./screen-time-monitoring-system
    ```  

---

## Usage  

### Windows  
1. Launch the program to begin tracking active applications.  
2. Enter time limits for specific applications.  
3. Monitor the top 5 most used applications in real time.  
4. Modify time limits as needed to manage usage.  

### macOS  
1. Launch the program to start tracking all user applications.  
2. Enter time limits for applications at the start of the program.  
3. View live usage statistics updated every second in the terminal.  
4. When a time limit is exceeded, a notification and dialog box appear. Press "OK" to terminate the application.  

---

## Supported Applications  

### Windows  
- Notepad  
- Google Chrome  
- File Explorer  
- Mozilla Firefox  
- Microsoft Outlook  
- Spotify  
- VLC Media Player  
- Microsoft Teams  
- Discord  
- Steam  

### macOS  
All actively opened user applications are supported without the need for pre-configuration.  

---

## Contributing  
We welcome contributions to improve and expand the project. Fork the repository, make your changes, and submit a pull request!  

---

## License  
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

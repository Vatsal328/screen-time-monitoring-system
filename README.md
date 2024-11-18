# Screen Time Monitoring System

## Description
The **Screen Time Monitoring System** tracks the time spent on user-initiated applications, excluding system background processes. It allows users to set time limits for applications, track their usage, and receive notifications when a time limit is exceeded. The program identifies common programs and assigns user-friendly names.

## Features
- **Track user-opened applications**: It only tracks the time spent on applications actively opened by the user.
- **Time limit functionality**: Users can set and update time limits for each application.
- **Top 5 most used applications**: Displays the top 5 most used applications based on time spent.
- **Friendly app names**: Converts process names like `notepad.exe` to user-friendly names such as "Notepad."
- **Notification on time limit exceed**: Alerts users when their time limit for an application is exceeded.
- **Increment/Decrement time limit**: Users can modify the time limits dynamically.

## Installation

### Requirements
- A C compiler (e.g., GCC).
- Windows OS for best compatibility (though it can be adapted to other platforms with necessary modifications).
- The `psapi` library for interacting with system processes.

### Steps
1. Clone the repository or download the project files.
2. Open a terminal/command prompt and navigate to the project directory.
3. To compile the code, run:

    ```bash
    gcc -o screen-time-monitoring-system it2.c -lpsapi
    ```

4. Once compiled, you can run the program in two ways:
   - **Using the terminal/command prompt**:

     ```bash
     ./screen-time-monitoring-system
     ```
   
   - **Using the `.bat` file**: Simply double-click the `run.bat` file in your project directory. It will automatically open the `.exe` file and run the application.

## Usage

1. **Tracking active applications**: When you run the program, it will display a list of active user applications and track the time spent on each.
2. **Set time limits**: You can set a time limit for each application. Once an application exceeds its limit, you will receive a notification.
3. **Top 5 most used applications**: The program displays the top 5 applications based on the total time spent.
4. **Dynamic time limit modification**: You can adjust the time limit for any application using the increment and decrement buttons.

## Supported Applications
The program currently recognizes the following commonly used applications (additional ones can be added to the source code):
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

## Contributing
Feel free to fork the repository and make improvements. Pull requests are welcome!

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

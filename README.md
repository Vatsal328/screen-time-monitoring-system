# Screen Time Monitoring System

## Description
This project is a Screen Time Monitoring System that tracks the time spent on user-initiated applications, excluding system background processes. It allows users to set time limits for applications, track their usage, and receive notifications when the time limit is exceeded. The application identifies common programs and assigns them user-friendly names.

## Features
- **Track user-opened applications**: It tracks time spent only on applications actively opened by the user.
- **Time limit functionality**: Users can set and update time limits for applications.
- **Top 5 most used applications**: Displays the top 5 most used applications based on the time spent.
- **Friendly app names**: Converts process names like "notepad.exe" to user-friendly names such as "Notepad."
- **Notification on time limit exceed**: Alerts users when their time limit for an application is exceeded.
- **Increment/Decrement time limit**: Users can modify the time limits dynamically.

## Installation

### Requirements
- A C compiler (e.g., GCC) is required to compile the project.
- Windows OS for best compatibility (but can be adapted for other platforms with necessary modifications).

### Steps
1. Clone the repository or download the project files.
2. Open the terminal/command prompt and navigate to the project directory.
3. Run the following command to compile the code:

    ```bash
    gcc -o screen-time-monitoring-system it2.c -lpsapi
    ```

4. Execute the program:

    ```bash
    ./screen-time-monitoring-system
    ```

## Usage

1. When you run the program, a window will pop up showing a list of active user applications.
2. The time spent on each application will be tracked and displayed in real-time.
3. You can set a time limit for each application. When an application exceeds the time limit, you will be notified.
4. The program also displays the top 5 most used applications based on time spent.
5. Users can adjust the time limit for each application dynamically using the increment and decrement buttons.

## Supported Applications
The program recognizes the following commonly used applications (additional ones can be added to the source code):
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

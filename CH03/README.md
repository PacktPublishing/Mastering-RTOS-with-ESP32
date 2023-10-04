| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- |

# CH02_Project: Web server to configure ESP32 wirelessly

The project **web_server_project** contains following source files located in folder [main](main) in C language:

1. Main source file [main.c](main/main.c)
2. Webserver source file [webserver.c](main/webserver.c)
3. Blinking LED task source file [task_blink.c](main/task_blink.c)
4. WiFi task source file [task_wifi.c](main/task_wifi.c)
5. Utils source file [utils.c](main/utils.c)

**Note:** Make sure you add all source files in `CMakeLists.txt` otherwise compiler will not compile them.

## How to configure the project

Using SDK configuration features, provide following configurations:

1. WiFi SSID
2. WiFi Password
3. LED GPIO number

## Compile and Upload the code on ESP32

Following the steps explained in chapter 1 of "Mastering RTOS with ESP32" book:

1. Compile the code
2. Upload to board
3. Monitor the debug logs

# References

1. Learn about creating new project [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project)

## Project folder contents

The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both).

Below is short explanation of files in the project folder.

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c				   Setup the project, and initialize tasks
│   └── task_blink.c		   Manage user indication using LED
│   └── task_wifi.c			   Manage the wifi connection
│   └── utils.c				   Common utilities functions
│   └── webserver.c			   Contains http based local webserver code
└── README.md                  This is the file you are currently reading
```

Additionally, the this project contains Makefile and component.mk files, used for the legacy Make based build system.
They are not used or needed when building with CMake and idf.py.

# Partition Table Spreadsheet

1.
2. https://docs.google.com/spreadsheets/d/1M6_qx00ZR82jyWa2slCMcvLtnXR9Q9dnis5mR5YChW8/edit#gid=0

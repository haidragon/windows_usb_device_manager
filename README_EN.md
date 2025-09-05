# Windows Device Manager
* A simple Windows tool to enumerate, disable, enable, and reset USB devices by their Device Instance ID.
# by haidragon  
# Public Account: Safety Dog's Self-Cultivation (安全狗的自我修养) [中文说明文档](./README.md)
# vx:2207344074
# github.com/haidragon
# bilibili:haidragonx
# http://securitytech.cc Official Website
# To learn more about stable commercial-grade solutions, USB security, and DLP development, [please click](https://github.com/haidragon/haidragon.github.io/blob/main/windows/img/usb/windows%E4%B8%8Ausb%E8%BF%87%E6%BB%A4%E4%B8%8E%E9%80%8F%E4%BC%A0(%E8%99%9A%E6%8B%9F%E5%8C%96)%E8%A7%86%E9%A2%91%E6%95%99%E7%A8%8B.png).

## Compilation

Use the provided compilation script:
```
.\compile_final.bat
```

This will generate the `final_device_manager.exe` executable.

## Usage

### 1. List all USB devices
Running the program without arguments will list all USB devices:
```
.\final_device_manager.exe
```

### 2. Disable a specific device
Use part of the device ID to disable a device, for example:
```
.\final_device_manager.exe "VID_ABCD&PID_1234" disable
```

### 3. Enable a specific device
Use part of the device ID to enable a device:
```
.\final_device_manager.exe "VID_ABCD&PID_1234" enable
```

### 4. Reset a specific device
Reset a device (disable then enable):
```
.\final_device_manager.exe "VID_ABCD&PID_1234" reset
```

### 5. Interactive operation
Provide only the device ID and the program will prompt for an operation:
```
.\final_device_manager.exe "VID_ABCD&PID_1234"
```

## Notes

1. **Administrator privileges**: To actually enable/disable devices, you need to run the program as administrator.
2. **Device ID**: You can use part of the device ID string for matching, and the program will find the first matching device.
3. **Error handling**: If an operation fails, the program will display the corresponding error message.

## Sample Output

When running the program, it will first display a list of all USB devices:
```
=== USB Devices ===
Found 16 USB devices
1. USB\VID_046D&PID_C548&MI_01\6&351EEABF&0&0001
   Description: USB Input Device

2. USB\VID_ABCD&PID_1234\2412022124011826550143
   Description: USB Mass Storage Device

...
```

Then perform the corresponding operation based on the provided arguments.

## Troubleshooting

If you encounter an "Access denied" error, make sure to run the command prompt or PowerShell as administrator before executing the program.

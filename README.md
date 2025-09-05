# windows_usb_device_manager
* 一个用于枚举、禁用、启用和重置 USB 设备的简单 Windows 工具（通过设备实例 ID 操作）。
* A simple Windows tool to enumerate, disable, enable, and reset USB devices by their Device Instance ID. [English Description](./README_EN.md)


# Windows设备管理器

# by haidragon  
# 公众号:安全狗的自我修养
# vx:2207344074
# github.com/haidragon
# bilibili:haidragonx
# http://securitytech.cc 认准官方唯一网站
# 想了解更稳定商业级的、学习usb安全、学习dlp开发的，[请点击](https://github.com/haidragon/haidragon.github.io/blob/main/windows/img/usb/windows%E4%B8%8Ausb%E8%BF%87%E6%BB%A4%E4%B8%8E%E9%80%8F%E4%BC%A0(%E8%99%9A%E6%8B%9F%E5%8C%96)%E8%A7%86%E9%A2%91%E6%95%99%E7%A8%8B.png)
 

## 编译

使用提供的编译脚本：
```
.\compile_final.bat
```

这将生成 `final_device_manager.exe` 可执行文件。

## 使用方法

### 1. 列出所有USB设备
不带参数运行程序将列出所有USB设备：
```
.\final_device_manager.exe
```

### 2. 禁用特定设备
使用设备的部分ID来禁用设备，例如：
```
.\final_device_manager.exe "VID_ABCD&PID_1234" disable
```

### 3. 启用特定设备
使用设备的部分ID来启用设备：
```
.\final_device_manager.exe "VID_ABCD&PID_1234" enable
```

### 4. 重置特定设备
重置设备（先禁用再启用）：
```
.\final_device_manager.exe "VID_ABCD&PID_1234" reset
```

### 5. 交互式操作
只提供设备ID，程序会提示选择操作：
```
.\final_device_manager.exe "VID_ABCD&PID_1234"
```

## 注意事项

1. **管理员权限**：要实际启用/禁用设备，需要以管理员身份运行程序。
2. **设备ID**：可以使用设备ID的部分字符串进行匹配，程序会找到第一个匹配的设备。
3. **错误处理**：如果操作失败，程序会显示相应的错误信息。

## 示例输出

运行程序时，首先会显示所有USB设备列表：
```
=== USB Devices ===
Found 16 USB devices
1. USB\VID_046D&PID_C548&MI_01\6&351EEABF&0&0001
   Description: USB 输入设备

2. USB\VID_ABCD&PID_1234\2412022124011826550143
   Description: USB 大容量存储设备

...
```

然后根据提供的参数执行相应操作。

## 常见问题

如果遇到"Access denied"错误，请确保以管理员身份运行命令提示符或PowerShell，然后再执行程序。

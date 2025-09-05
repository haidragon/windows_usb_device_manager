// by haidragon  
// 公众号:安全狗的自我修养
// vx:2207344074
// github.com/haidragon
// bilibili:haidragonx
// http://securitytech.cc

#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <devguid.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#pragma comment(lib, "setupapi.lib")

// 设备信息结构体
struct DeviceInfo {
    std::string instanceId;     // 设备实例ID
    std::string description;    // 设备描述
    bool isEnabled;             // 是否启用
};

// 启用或禁用设备
bool SetDeviceEnabled(const std::string& instanceId, bool enable) {
    DEVINST devInst;
    CONFIGRET cr = CM_Locate_DevNodeA(&devInst, (DEVINSTID_A)instanceId.c_str(), CM_LOCATE_DEVNODE_NORMAL);
    if (cr != CR_SUCCESS) {
        std::cerr << "找不到设备: " << instanceId << " (错误代码: " << cr << ")" << std::endl;
        return false;
    }

    ULONG stateChange = enable ? DICS_ENABLE : DICS_DISABLE;
    SP_PROPCHANGE_PARAMS params = {};
    params.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    params.StateChange = stateChange;
    params.Scope = DICS_FLAG_GLOBAL;
    params.HwProfile = 0;

    // 尝试直接使用设备实例ID
    HDEVINFO hDevInfo = SetupDiGetClassDevsA(NULL, instanceId.c_str(), NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        // 备用方案：尝试使用设备接口类
        hDevInfo = SetupDiGetClassDevsA(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
        if (hDevInfo == INVALID_HANDLE_VALUE) {
            std::cerr << "SetupDiGetClassDevs 失败" << std::endl;
            return false;
        }
    }

    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
    bool foundDevice = false;
    DWORD index = 0;
    
    // 枚举设备以找到匹配的设备
    while (SetupDiEnumDeviceInfo(hDevInfo, index, &devInfoData)) {
        char buffer[512];
        if (SetupDiGetDeviceInstanceIdA(hDevInfo, &devInfoData, buffer, sizeof(buffer), NULL)) {
            if (_stricmp(buffer, instanceId.c_str()) == 0) {
                foundDevice = true;
                break;
            }
        }
        index++;
    }
    
    if (!foundDevice) {
        std::cerr << "在设备信息列表中找不到设备: " << instanceId << std::endl;
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return false;
    }

    if (!SetupDiSetClassInstallParams(hDevInfo, &devInfoData,
                                      (SP_CLASSINSTALL_HEADER*)&params,
                                      sizeof(params))) {
        std::cerr << "设置参数失败" << std::endl;
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return false;
    }

    if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &devInfoData)) {
        DWORD errorCode = GetLastError();
        std::cerr << "更改设备状态失败，错误代码: " << errorCode << std::endl;
        // 错误代码5表示访问被拒绝，通常是因为没有管理员权限
        if (errorCode == ERROR_ACCESS_DENIED) {
            std::cerr << "访问被拒绝。请以管理员身份运行此程序。" << std::endl;
        }
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return false;
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    std::cout << (enable ? "设备已启用: " : "设备已禁用: ") << instanceId << std::endl;
    return true;
}

// 枚举所有设备
std::vector<DeviceInfo> EnumerateDevices() {
    std::vector<DeviceInfo> devices;
    
    HDEVINFO hDevInfo = SetupDiGetClassDevsA(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        std::cerr << "获取设备列表失败" << std::endl;
        return devices;
    }

    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    char buffer[512];

    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); i++) {
        if (SetupDiGetDeviceInstanceIdA(hDevInfo, &devInfoData, buffer, sizeof(buffer), NULL)) {
            DeviceInfo device;
            device.instanceId = std::string(buffer);
            
            // 获取设备描述
            char description[512];
            DWORD dataType;
            if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &devInfoData, SPDRP_DEVICEDESC,
                                                 &dataType, (PBYTE)description, sizeof(description), NULL)) {
                device.description = std::string(description);
            } else {
                device.description = "未知设备";
            }
            
            // 检查设备是否启用
            device.isEnabled = true; // 默认启用
            
            devices.push_back(device);
        }
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return devices;
}

// 根据ID查找设备（支持部分匹配）
DeviceInfo* FindDeviceById(std::vector<DeviceInfo>& devices, const std::string& targetId) {
    // 首先尝试精确匹配
    for (std::vector<DeviceInfo>::iterator it = devices.begin(); it != devices.end(); ++it) {
        if (_stricmp(it->instanceId.c_str(), targetId.c_str()) == 0) {
            return &(*it);
        }
    }
    
    // 如果没有精确匹配，尝试部分匹配
    for (std::vector<DeviceInfo>::iterator it = devices.begin(); it != devices.end(); ++it) {
        if (it->instanceId.find(targetId) != std::string::npos) {
            return &(*it);
        }
    }
    
    return NULL;
}

// 查找所有USB设备
std::vector<DeviceInfo*> FindUsbDevices(std::vector<DeviceInfo>& devices) {
    std::vector<DeviceInfo*> usbDevices;
    
    for (std::vector<DeviceInfo>::iterator it = devices.begin(); it != devices.end(); ++it) {
        // 检查设备实例ID是否包含"USB"
        std::string instanceId = it->instanceId;
        std::transform(instanceId.begin(), instanceId.end(), instanceId.begin(), ::toupper);
        
        if (instanceId.find("USB") != std::string::npos) {
            usbDevices.push_back(&(*it));
        }
    }
    
    return usbDevices;
}

// 打印USB设备
void PrintUsbDevices(const std::vector<DeviceInfo*>& usbDevices) {
    std::cout << "\n=== USB设备列表 ===" << std::endl;
    std::cout << "找到 " << usbDevices.size() << " 个USB设备" << std::endl;
    
    if (usbDevices.empty()) {
        std::cout << "未找到USB设备。" << std::endl;
        return;
    }
    
    for (size_t i = 0; i < usbDevices.size(); ++i) {
        std::cout << (i + 1) << ". " << usbDevices[i]->instanceId << std::endl;
        std::cout << "   描述: " << usbDevices[i]->description << std::endl;
        std::cout << std::endl;
    }
}

// 打印使用说明
void PrintUsage(const char* programName) {
    std::cout << "用法: " << programName << " [设备ID]" << std::endl;
    std::cout << "      " << programName << " [设备ID] [enable|disable|reset]" << std::endl;
    std::cout << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  设备ID     要操作的设备实例ID（可以是部分ID）" << std::endl;
    std::cout << "  enable     启用指定设备" << std::endl;
    std::cout << "  disable    禁用指定设备" << std::endl;
    std::cout << "  reset      重置指定设备（先禁用再启用）" << std::endl;
    std::cout << std::endl;
    std::cout << "如果不提供设备ID，将列出所有USB设备。" << std::endl;
    std::cout << "设备ID可以是部分字符串 - 程序将找到第一个匹配的设备。" << std::endl;
    std::cout << "示例: " << programName << " \"VID_1234&PID_5678\" disable" << std::endl;
}

// 处理设备操作
bool ProcessDeviceOperation(const std::string& deviceId, const std::string& operation) {
    std::cout << "正在搜索设备: " << deviceId << std::endl;
    std::vector<DeviceInfo> devices = EnumerateDevices();
    DeviceInfo* targetDevice = FindDeviceById(devices, deviceId);
    
    if (!targetDevice) {
        std::cerr << "未找到设备: " << deviceId << std::endl;
        return false;
    }
    
    std::cout << "找到设备!" << std::endl;
    std::cout << "设备: " << targetDevice->instanceId << std::endl;
    std::cout << "描述: " << targetDevice->description << std::endl;
    
    if (operation == "enable") {
        std::cout << "\n正在启用设备..." << std::endl;
        return SetDeviceEnabled(targetDevice->instanceId, true);
    } else if (operation == "disable") {
        std::cout << "\n正在禁用设备..." << std::endl;
        return SetDeviceEnabled(targetDevice->instanceId, false);
    } else if (operation == "reset") {
        std::cout << "\n正在重置设备（先禁用再启用）..." << std::endl;
        std::cout << "正在禁用设备..." << std::endl;
        if (SetDeviceEnabled(targetDevice->instanceId, false)) {
            std::cout << "等待2秒..." << std::endl;
            Sleep(2000);
            std::cout << "正在启用设备..." << std::endl;
            return SetDeviceEnabled(targetDevice->instanceId, true);
        }
        return false;
    } else {
        std::cerr << "未知操作: " << operation << std::endl;
        return false;
    }
}

// 主函数
int main(int argc, char* argv[]) {
    std::cout << "=== Windows设备管理器 ===" << std::endl;
    
    // 枚举所有设备
    std::cout << "正在枚举所有设备..." << std::endl;
    std::vector<DeviceInfo> devices = EnumerateDevices();
    std::cout << "找到 " << devices.size() << " 个设备" << std::endl;
    
    // 打印所有USB设备
    std::vector<DeviceInfo*> usbDevices = FindUsbDevices(devices);
    PrintUsbDevices(usbDevices);
    
    // 如果没有提供参数，仅列出USB设备并退出
    if (argc == 1) {
        std::cout << "\n要对特定设备进行操作，请提供设备ID和操作:" << std::endl;
        PrintUsage(argv[0]);
        return 0;
    }
    
    // 解析命令行参数
    std::string deviceId = argv[1];
    
    if (argc == 2) {
        // 只提供了设备ID，提示选择操作
        std::cout << "\n设备ID: " << deviceId << std::endl;
        std::cout << "选择操作:" << std::endl;
        std::cout << "1. 启用" << std::endl;
        std::cout << "2. 禁用" << std::endl;
        std::cout << "3. 重置（先禁用再启用）" << std::endl;
        std::cout << "请输入选择 (1-3): ";
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                ProcessDeviceOperation(deviceId, "enable");
                break;
            case 2:
                ProcessDeviceOperation(deviceId, "disable");
                break;
            case 3:
                ProcessDeviceOperation(deviceId, "reset");
                break;
            default:
                std::cerr << "无效选择!" << std::endl;
                return 1;
        }
    } else if (argc == 3) {
        // 提供了设备ID和操作
        std::string operation = argv[2];
        ProcessDeviceOperation(deviceId, operation);
    } else {
        // 参数数量无效
        PrintUsage(argv[0]);
        return 1;
    }
    
    std::cout << "\n程序执行完成。" << std::endl;
    return 0;
}
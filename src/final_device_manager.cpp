// by haidragon  
// ���ں�:��ȫ������������
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

// �豸��Ϣ�ṹ��
struct DeviceInfo {
    std::string instanceId;     // �豸ʵ��ID
    std::string description;    // �豸����
    bool isEnabled;             // �Ƿ�����
};

// ���û�����豸
bool SetDeviceEnabled(const std::string& instanceId, bool enable) {
    DEVINST devInst;
    CONFIGRET cr = CM_Locate_DevNodeA(&devInst, (DEVINSTID_A)instanceId.c_str(), CM_LOCATE_DEVNODE_NORMAL);
    if (cr != CR_SUCCESS) {
        std::cerr << "�Ҳ����豸: " << instanceId << " (�������: " << cr << ")" << std::endl;
        return false;
    }

    ULONG stateChange = enable ? DICS_ENABLE : DICS_DISABLE;
    SP_PROPCHANGE_PARAMS params = {};
    params.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    params.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    params.StateChange = stateChange;
    params.Scope = DICS_FLAG_GLOBAL;
    params.HwProfile = 0;

    // ����ֱ��ʹ���豸ʵ��ID
    HDEVINFO hDevInfo = SetupDiGetClassDevsA(NULL, instanceId.c_str(), NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        // ���÷���������ʹ���豸�ӿ���
        hDevInfo = SetupDiGetClassDevsA(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
        if (hDevInfo == INVALID_HANDLE_VALUE) {
            std::cerr << "SetupDiGetClassDevs ʧ��" << std::endl;
            return false;
        }
    }

    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    
    bool foundDevice = false;
    DWORD index = 0;
    
    // ö���豸���ҵ�ƥ����豸
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
        std::cerr << "���豸��Ϣ�б����Ҳ����豸: " << instanceId << std::endl;
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return false;
    }

    if (!SetupDiSetClassInstallParams(hDevInfo, &devInfoData,
                                      (SP_CLASSINSTALL_HEADER*)&params,
                                      sizeof(params))) {
        std::cerr << "���ò���ʧ��" << std::endl;
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return false;
    }

    if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &devInfoData)) {
        DWORD errorCode = GetLastError();
        std::cerr << "�����豸״̬ʧ�ܣ��������: " << errorCode << std::endl;
        // �������5��ʾ���ʱ��ܾ���ͨ������Ϊû�й���ԱȨ��
        if (errorCode == ERROR_ACCESS_DENIED) {
            std::cerr << "���ʱ��ܾ������Թ���Ա������д˳���" << std::endl;
        }
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return false;
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    std::cout << (enable ? "�豸������: " : "�豸�ѽ���: ") << instanceId << std::endl;
    return true;
}

// ö�������豸
std::vector<DeviceInfo> EnumerateDevices() {
    std::vector<DeviceInfo> devices;
    
    HDEVINFO hDevInfo = SetupDiGetClassDevsA(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        std::cerr << "��ȡ�豸�б�ʧ��" << std::endl;
        return devices;
    }

    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    char buffer[512];

    for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); i++) {
        if (SetupDiGetDeviceInstanceIdA(hDevInfo, &devInfoData, buffer, sizeof(buffer), NULL)) {
            DeviceInfo device;
            device.instanceId = std::string(buffer);
            
            // ��ȡ�豸����
            char description[512];
            DWORD dataType;
            if (SetupDiGetDeviceRegistryPropertyA(hDevInfo, &devInfoData, SPDRP_DEVICEDESC,
                                                 &dataType, (PBYTE)description, sizeof(description), NULL)) {
                device.description = std::string(description);
            } else {
                device.description = "δ֪�豸";
            }
            
            // ����豸�Ƿ�����
            device.isEnabled = true; // Ĭ������
            
            devices.push_back(device);
        }
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return devices;
}

// ����ID�����豸��֧�ֲ���ƥ�䣩
DeviceInfo* FindDeviceById(std::vector<DeviceInfo>& devices, const std::string& targetId) {
    // ���ȳ��Ծ�ȷƥ��
    for (std::vector<DeviceInfo>::iterator it = devices.begin(); it != devices.end(); ++it) {
        if (_stricmp(it->instanceId.c_str(), targetId.c_str()) == 0) {
            return &(*it);
        }
    }
    
    // ���û�о�ȷƥ�䣬���Բ���ƥ��
    for (std::vector<DeviceInfo>::iterator it = devices.begin(); it != devices.end(); ++it) {
        if (it->instanceId.find(targetId) != std::string::npos) {
            return &(*it);
        }
    }
    
    return NULL;
}

// ��������USB�豸
std::vector<DeviceInfo*> FindUsbDevices(std::vector<DeviceInfo>& devices) {
    std::vector<DeviceInfo*> usbDevices;
    
    for (std::vector<DeviceInfo>::iterator it = devices.begin(); it != devices.end(); ++it) {
        // ����豸ʵ��ID�Ƿ����"USB"
        std::string instanceId = it->instanceId;
        std::transform(instanceId.begin(), instanceId.end(), instanceId.begin(), ::toupper);
        
        if (instanceId.find("USB") != std::string::npos) {
            usbDevices.push_back(&(*it));
        }
    }
    
    return usbDevices;
}

// ��ӡUSB�豸
void PrintUsbDevices(const std::vector<DeviceInfo*>& usbDevices) {
    std::cout << "\n=== USB�豸�б� ===" << std::endl;
    std::cout << "�ҵ� " << usbDevices.size() << " ��USB�豸" << std::endl;
    
    if (usbDevices.empty()) {
        std::cout << "δ�ҵ�USB�豸��" << std::endl;
        return;
    }
    
    for (size_t i = 0; i < usbDevices.size(); ++i) {
        std::cout << (i + 1) << ". " << usbDevices[i]->instanceId << std::endl;
        std::cout << "   ����: " << usbDevices[i]->description << std::endl;
        std::cout << std::endl;
    }
}

// ��ӡʹ��˵��
void PrintUsage(const char* programName) {
    std::cout << "�÷�: " << programName << " [�豸ID]" << std::endl;
    std::cout << "      " << programName << " [�豸ID] [enable|disable|reset]" << std::endl;
    std::cout << std::endl;
    std::cout << "ѡ��:" << std::endl;
    std::cout << "  �豸ID     Ҫ�������豸ʵ��ID�������ǲ���ID��" << std::endl;
    std::cout << "  enable     ����ָ���豸" << std::endl;
    std::cout << "  disable    ����ָ���豸" << std::endl;
    std::cout << "  reset      ����ָ���豸���Ƚ��������ã�" << std::endl;
    std::cout << std::endl;
    std::cout << "������ṩ�豸ID�����г�����USB�豸��" << std::endl;
    std::cout << "�豸ID�����ǲ����ַ��� - �����ҵ���һ��ƥ����豸��" << std::endl;
    std::cout << "ʾ��: " << programName << " \"VID_1234&PID_5678\" disable" << std::endl;
}

// �����豸����
bool ProcessDeviceOperation(const std::string& deviceId, const std::string& operation) {
    std::cout << "���������豸: " << deviceId << std::endl;
    std::vector<DeviceInfo> devices = EnumerateDevices();
    DeviceInfo* targetDevice = FindDeviceById(devices, deviceId);
    
    if (!targetDevice) {
        std::cerr << "δ�ҵ��豸: " << deviceId << std::endl;
        return false;
    }
    
    std::cout << "�ҵ��豸!" << std::endl;
    std::cout << "�豸: " << targetDevice->instanceId << std::endl;
    std::cout << "����: " << targetDevice->description << std::endl;
    
    if (operation == "enable") {
        std::cout << "\n���������豸..." << std::endl;
        return SetDeviceEnabled(targetDevice->instanceId, true);
    } else if (operation == "disable") {
        std::cout << "\n���ڽ����豸..." << std::endl;
        return SetDeviceEnabled(targetDevice->instanceId, false);
    } else if (operation == "reset") {
        std::cout << "\n���������豸���Ƚ��������ã�..." << std::endl;
        std::cout << "���ڽ����豸..." << std::endl;
        if (SetDeviceEnabled(targetDevice->instanceId, false)) {
            std::cout << "�ȴ�2��..." << std::endl;
            Sleep(2000);
            std::cout << "���������豸..." << std::endl;
            return SetDeviceEnabled(targetDevice->instanceId, true);
        }
        return false;
    } else {
        std::cerr << "δ֪����: " << operation << std::endl;
        return false;
    }
}

// ������
int main(int argc, char* argv[]) {
    std::cout << "=== Windows�豸������ ===" << std::endl;
    
    // ö�������豸
    std::cout << "����ö�������豸..." << std::endl;
    std::vector<DeviceInfo> devices = EnumerateDevices();
    std::cout << "�ҵ� " << devices.size() << " ���豸" << std::endl;
    
    // ��ӡ����USB�豸
    std::vector<DeviceInfo*> usbDevices = FindUsbDevices(devices);
    PrintUsbDevices(usbDevices);
    
    // ���û���ṩ���������г�USB�豸���˳�
    if (argc == 1) {
        std::cout << "\nҪ���ض��豸���в��������ṩ�豸ID�Ͳ���:" << std::endl;
        PrintUsage(argv[0]);
        return 0;
    }
    
    // ���������в���
    std::string deviceId = argv[1];
    
    if (argc == 2) {
        // ֻ�ṩ���豸ID����ʾѡ�����
        std::cout << "\n�豸ID: " << deviceId << std::endl;
        std::cout << "ѡ�����:" << std::endl;
        std::cout << "1. ����" << std::endl;
        std::cout << "2. ����" << std::endl;
        std::cout << "3. ���ã��Ƚ��������ã�" << std::endl;
        std::cout << "������ѡ�� (1-3): ";
        
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
                std::cerr << "��Чѡ��!" << std::endl;
                return 1;
        }
    } else if (argc == 3) {
        // �ṩ���豸ID�Ͳ���
        std::string operation = argv[2];
        ProcessDeviceOperation(deviceId, operation);
    } else {
        // ����������Ч
        PrintUsage(argv[0]);
        return 1;
    }
    
    std::cout << "\n����ִ����ɡ�" << std::endl;
    return 0;
}
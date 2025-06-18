#ifndef DETECTUSB_H
#define DETECTUSB_H

#include <windows.h>
#include <setupapi.h>
#include <initguid.h>
#include <devguid.h>
#include <QDebug>
#include <QString>
#include <QVector>

#pragma comment(lib, "setupapi.lib")

class DetectUSB {
private:
    QVector<QString> InstanceIDs;

public:
    QVector<QString> getUSBDevices(){
        return InstanceIDs;
    }
    void detectUSBDevices() {
        HDEVINFO AllDevicesInfo = SetupDiGetClassDevsW(&GUID_DEVCLASS_DISKDRIVE, NULL, NULL, DIGCF_PRESENT);
        if (AllDevicesInfo == INVALID_HANDLE_VALUE) {
            qDebug() << "Failed to get device info set." ;
            return;
        }

        SP_DEVINFO_DATA DeviceInfo;
        DeviceInfo.cbSize = sizeof(SP_DEVINFO_DATA);
        DWORD index = 0;

        while (SetupDiEnumDeviceInfo(AllDevicesInfo, index, &DeviceInfo)) {
            wchar_t DeviceInstanceId[256] = { 0 };


            SetupDiGetDeviceInstanceIdW(AllDevicesInfo, &DeviceInfo, DeviceInstanceId, sizeof(DeviceInstanceId) / sizeof(wchar_t), NULL);


            QString qStr = QString::fromWCharArray(DeviceInstanceId);
            if (!qStr.isEmpty()) {
                InstanceIDs.append(qStr);
            }

            index++;
        }



        for (int i = InstanceIDs.size() - 1; i >= 0; i--) {
            if (!InstanceIDs[i].contains("USBSTOR")) {
                InstanceIDs.remove(i);
            }
        }


        for (int i = 0; i < InstanceIDs.size(); i++) {
            int position = InstanceIDs[i].lastIndexOf("\\");
            if (position != -1) {
                InstanceIDs[i] = InstanceIDs[i].mid(position + 1);
                if (InstanceIDs[i].length() > 2) {
                    InstanceIDs[i].chop(2);
                }
            }
        }



        SetupDiDestroyDeviceInfoList(AllDevicesInfo);
    }
};

#endif // DETECTUSB_H

#pragma once
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;
#define SERIALPATH _T("Hardware\\DeviceMap\\SerialComm")
#define USBPATH "SYSTEM\\ControlSet001\\Enum\\USB\\"
#define EUSB_KEYNAME _T(USBPATH "VID_CAFE&PID_4003&MI_00")

void wcharTochar(const wchar_t* wcharSrc, char* chrDst, int chrDstlength);
bool QueryRegKey(LPCWSTR keyPath, LPCWSTR ValueName, char* Value, int valueLength);
vector<string> QuerySerialPort();
vector<string> QueryEUSBPort();
string findUsbSerialCom();


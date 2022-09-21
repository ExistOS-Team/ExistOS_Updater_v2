#include "WinReg.h"
//#define debug(fmt, ...) printf(fmt, __VA_ARGS__)


#include <iostream>
using namespace std;

void wcharTochar(const wchar_t* wcharSrc, char* chrDst, int chrDstlength)
{
	WideCharToMultiByte(CP_ACP, 0, wcharSrc, -1, chrDst, chrDstlength, NULL, NULL);
}


bool QueryRegKey(LPCWSTR keyPath, LPCWSTR ValueName, char* Value, int valueLength)
{
	HKEY hKey;
	RegOpenKey(HKEY_LOCAL_MACHINE, keyPath, &hKey);

	/*
	if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, keyPath, &hKey))
	{
		debug("OpenRegKey success!\n");

	}
	else { debug("OpenRegKey failed!\n"); }
	*/

	DWORD dwType = REG_SZ;//定义数据类型
	DWORD dwLen = MAX_PATH;
	BYTE data[MAX_PATH];

	if (ERROR_SUCCESS == RegQueryValueEx(hKey, ValueName, 0, &dwType, (LPBYTE)data, &dwLen))
	{
		WideCharToMultiByte(CP_ACP, 0, (LPCWCH)data, -1, Value, valueLength, NULL, NULL);
		//debug("query success : %s!\n", Value);
		RegCloseKey(hKey); //关闭注册表	
		return true;
	}
	return false;
}

vector<string> QueryEUSBPort()
{
	HKEY hKey;
	vector<string> COM;

	LPCTSTR lpSubKey = EUSB_KEYNAME;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		//debug("open USB registry key failed !");
		RegCloseKey(hKey); //关闭注册表
		return COM;
	}

	DWORD dwIndex = 0;
	TCHAR valueName[MAX_PATH];
	char strValue[MAX_PATH];
	LONG status;

	TCHAR paraPath[MAX_PATH];

	do {
		status = RegEnumKey(hKey, dwIndex++, valueName, MAX_PATH);
		if ((status == ERROR_SUCCESS))
		{
			//wprintf(L"%s exists!\n", portName);
			//strValue 目标
			lstrcpyW(paraPath, EUSB_KEYNAME);
			lstrcpyW(&paraPath[lstrlenW(paraPath)], _T("\\"));
			lstrcpyW(&paraPath[lstrlenW(paraPath)], valueName);
			lstrcpyW(&paraPath[lstrlenW(paraPath)], _T("\\Device Parameters"));

			if (QueryRegKey(paraPath, _T("PortName"), strValue, MAX_PATH) == false)
			{
				//debug("open USB Port Parameters registry key failed !");
				continue;
			}

			//wprintf(L"paraPath: %s\n", paraPath);

			//WideCharToMultiByte(CP_ACP, 0, (LPCWCH)valueName, -1, strValue, MAX_PATH, NULL, NULL);
			COM.push_back(strValue);
		}

	} while ((status != ERROR_NO_MORE_ITEMS));
	

	return COM;
}

vector<string> QuerySerialPort()
{
	HKEY hKey;
	vector<string> COM;

	LPCTSTR lpSubKey = SERIALPATH;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		//debug("open registry key failed !");
		RegCloseKey(hKey); //关闭注册表
		return COM;
	}


	TCHAR valueName[MAX_PATH];
	BYTE portName[MAX_PATH];
	LONG status;
	DWORD dwIndex = 0;
	DWORD dwSizeValueName = MAX_PATH;
	DWORD dwSizeofPortName = MAX_PATH;
	DWORD Type;


	char strValue[MAX_PATH];
	int length = MAX_PATH;
	do
	{
		status = RegEnumValue(hKey, dwIndex++, valueName, &dwSizeValueName, NULL, &Type, portName, &dwSizeofPortName);
		if ((status == ERROR_SUCCESS))
		{
			//w//
			// (L"%s exists!\n", portName);
			//strValue 目标
			WideCharToMultiByte(CP_ACP, 0, (LPCWCH)portName, -1, strValue, length, NULL, NULL);
			COM.push_back(strValue);
		}
		//每读取一次dwSizeValueName和dwSizeofPortName都会被修改
		//注意一定要重置,否则会出现很离奇的错误,本人就试过因没有重置,出现读不了COM大于10以上的串口
		dwSizeValueName = MAX_PATH;
		dwSizeofPortName = MAX_PATH;
	} while ((status != ERROR_NO_MORE_ITEMS));
	RegCloseKey(hKey); //关闭注册表
	return COM;
}


string findUsbSerialCom()
{
	vector<string> COMListAvailable = QuerySerialPort();
	vector<string> EUSBPort = QueryEUSBPort();
	std::string COM = "NONE";

	/*
	//cout << "EUSB Port:" << EUSBPort.size() << endl;
	for (string& var : EUSBPort)
	{
		//cout << "[" << var << "]";
	}
	//cout << endl;
	*/
	if (EUSBPort.size() == 0) {
		return COM;
	}

	/*
	//cout << "COMListAvailable:" << COMListAvailable.size() << endl;
	for (string& var : COMListAvailable)
	{
		//cout << "[" << var << "]";
	}
	//cout << endl;
	*/

	for (string& EUSB : EUSBPort)
	{
		for (string& ACOM : COMListAvailable)
		{
			if (EUSB.compare(ACOM) == 0)
			{
				//cout << "Find:" << ACOM << endl;
				COM = ACOM;
				break;
			}
		}
	}

	return COM;
}
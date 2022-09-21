#include "CComHelper.h"

#include <string>
#include <iostream>
#include <vector>
using namespace std;

LPCWSTR stringToLPCWSTR(std::string orig)
{
	size_t origsize = orig.length() + 1;
	size_t convertedChars = 0;
	wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t) * (orig.length() - 1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);
	
	return wcstring;
}

bool CComHelper::Open(string com)
{
	//QMessageBox::information(nullptr, "", "Start CreateFileW()");	//for debug

	hCom = CreateFileW(
		stringToLPCWSTR(com),
		GENERIC_WRITE | GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (hCom == (HANDLE) - 1)
	{
		return false;
	}

	return true;
}

void CComHelper::Set()
{
	SetupComm(hCom, 500, 500);
	COMMTIMEOUTS TimeOuts; //�趨����ʱ
	TimeOuts.ReadIntervalTimeout = MAXDWORD;
	TimeOuts.ReadTotalTimeoutMultiplier = 100;
	TimeOuts.ReadTotalTimeoutConstant = 1000;


	TimeOuts.WriteTotalTimeoutConstant = 10;//�趨д��ʱ
	TimeOuts.WriteTotalTimeoutMultiplier = 100;
	SetCommTimeouts(hCom, &TimeOuts);
	

	DCB dcb;
	GetCommState(hCom, &dcb);
	dcb.BaudRate = 115200; 
	dcb.ByteSize = 8; //ÿ���ֽ���8λ
	dcb.Parity = NOPARITY; //����żУ��λ
	dcb.StopBits = TWOSTOPBITS; //����ֹͣλ
	SetCommState(hCom, &dcb);
	

	PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR);

}



bool CComHelper::Read(char* data, int length, DWORD *dwCount)
{

	bool bReadStat = ReadFile(hCom, data, (DWORD)length, dwCount, NULL);
	return bReadStat;
}

bool CComHelper::Write(char* data, int length)
{
	DWORD dwWrite = (DWORD)length;
	COMSTAT ComStat;
	DWORD  dwError;
	ClearCommError(hCom, &dwError, &ComStat);
	bool bWriteStat = WriteFile(hCom, data, dwWrite, &dwWrite, NULL);
	if (!bWriteStat)
	{
		return false;
	}
	PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	return true;
}

bool CComHelper::WriteStr(const char* data)
{
	return Write((char*)data, strlen(data));
}

void CComHelper::SetDTR(bool set)
{
	if (set)
	{
		EscapeCommFunction(hCom, SETDTR);
	}
	else 
	{
		EscapeCommFunction(hCom, CLRDTR);
	}
}

void CComHelper::SetRTS(bool set)
{
	if (set)
	{
		EscapeCommFunction(hCom, SETRTS);
	}
	else
	{
		EscapeCommFunction(hCom, CLRRTS);
	}
}

bool CComHelper::Close()
{
	bool result = CloseHandle(hCom);
	return result;
}


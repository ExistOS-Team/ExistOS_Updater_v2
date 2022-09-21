#pragma once
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <vector>

#include <iostream>

#include <QMessageBox>
using namespace std;


class CComHelper
{
private:
	HANDLE hCom;

public:
	bool Open(string com);
	void Set();
	bool Read(char* data, int length, DWORD *dwCount);
	bool Write(char* data, int length);
	bool WriteStr(const char* data);
	void SetDTR(bool set);
	void SetRTS(bool set);
	bool Close();
};


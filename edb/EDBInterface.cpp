#include "EDBInterface.h"
#include <stdint.h>
#include <stdio.h>
#include <iostream>

#include <time.h>
using namespace std;


unsigned long long speed, uploadedSize, pageNow, blockNow, fsize;

long long getTime()
{
	return GetTickCount64();
}


unsigned char blockChksum(char* block, unsigned int blockSize)
{
	unsigned char sum = 0x5A;
	for (unsigned int i = 0; i < blockSize; i++)
	{
		sum += block[i];
	}
	return sum;
}

void EDBInterface::reset(bool mode) 
{
	//DWORD wrcnt;
	//BOOL ret;
	if (USBHighSpeed)
	{
		//Sleep(100);
	}
	else {
		com.SetRTS(mode);
		Sleep(20);
		com.SetDTR(false);
	}
}

bool EDBInterface::waitStr(char* str)
{
	int len = int(strlen(str));
	int retry = 2;
	DWORD cnt;
	char buf[128];
	memset(buf, 0, 128);
	int ret;
	if (USBHighSpeed) {
		while (retry > 0)
		{
			//cout << "Waiting Sync...\r";
			memset(wrBuf, 0, sizeof(wrBuf));
			SetFilePointer(hCMDf, 0, NULL, FILE_BEGIN);
			ret = ReadFile(hCMDf, wrBuf, sizeof(wrBuf), &cnt, NULL);
			if (ret)
			{
				if (strcmp(str, wrBuf) == 0) {
					return true;
				}
				else {
					//cout << "Sync Failed... Retry:" << retry << endl;
					this->reset(EDB_MODE_TEXT);
					retry--;
				}
			}else
			{
				//cout << "Sync Failed... Retry:" << retry << endl;
				this->reset(EDB_MODE_TEXT);
				retry--;
			}
		}
	}
	else {
		while (retry > 0)
		{
			//cout << "Waiting Sync...\r";
			com.Read(buf, len, &cnt);
			if (cnt > 0)
			{
				if (strcmp(str, buf) == 0) {
					return true;
				}
				else {
					//cout << "Sync Failed... Retry:" << retry << endl;
					this->reset(EDB_MODE_TEXT);
					retry--;
				}
			}
			else {
				//cout << "Sync Failed... Retry:" << retry << endl;
				this->reset(EDB_MODE_TEXT);
				retry--;
			}
		}
		
	}
	return false;
}

void EDBInterface::wrStr(const char *str)
{
	if (!str) {
		return;
	}
	DWORD cnt;
	//int len = int(strlen(str));
	if (USBHighSpeed) {
		memset(wrBuf, 0, sizeof(wrBuf));
		strcpy_s(wrBuf, str);
		SetFilePointer(hCMDf, 0, NULL, FILE_BEGIN);
		WriteFile(hCMDf, wrBuf, sizeof(wrBuf), &cnt, NULL);
	}
	else {
		com.WriteStr(str);
	}
}

bool EDBInterface::wrDat(char* dat, size_t len)
{
	int ret = 0;
	DWORD cnt;
	if (USBHighSpeed)
	{
		SetFilePointer(hDATf, 0, NULL, FILE_BEGIN);
		
		ret = WriteFile(hDATf, dat, int(len), &cnt, NULL);

		if (ret) {
			return true;
		}
		return false;
	}
	else {
		return com.Write(dat, int(len));
	}
}

bool EDBInterface::rdDat(char* dat, size_t len, size_t *rbcnt)
{
	DWORD cnt;
	if (USBHighSpeed)
	{
		memset(wrBuf, 0, sizeof(wrBuf));
		SetFilePointer(hCMDf, 0, NULL, FILE_BEGIN);
		int ret = ReadFile(hCMDf, wrBuf, sizeof(wrBuf), &cnt, NULL);
		memcpy(dat, wrBuf, len);
		*rbcnt = len;
		if (ret) 
		{
			return true;
		}
		return false;
	}
	else {
		return com.Read(dat, len, (DWORD *)rbcnt);
	}
}

bool EDBInterface::eraseBlock(unsigned int block)
{
	char cmdbuf[64];
	memset(cmdbuf, 0, sizeof(cmdbuf));
	sprintf_s(cmdbuf, "ERASEB:%d\n", block);
	this->wrStr(cmdbuf);
	if (this->waitStr((char*)"EROK\n") == false)
	{
		//printf("Erase Block Time Out:%d\n", block);
		return false;
	}
	return true;
}


int EDBInterface::flash(flashImg item, const callBack& refreshStatus)
{
	char cmdbuf[64];
	size_t cnt;
	size_t rbcnt;
	this->reset(EDB_MODE_TEXT);
	if (this->ping() == false)
	{
		//cout << "Device No Responses." << endl;
		return false;
	}
	this->wrStr("RESETDBUF\n");
	if (!this->waitStr((char*)"READY\n"))
	{
		//cout << "Device No Responses." << endl;
		return false;
	}
	//cout << "Writing: " << item.filename << "..." << endl;
	fseek(item.f, 0, SEEK_END);
	fsize = ftell(item.f);
	uint8_t chksum;
	uint32_t rcshkdum;
	long long st;
	fseek(item.f, 0, SEEK_SET);

	uint32_t page_cnt = item.toPage;
	uint32_t block_cnt = page_cnt / 64;
	uint32_t last_block = 0;
	do {
		block_cnt = page_cnt / 64;

		st = getTime();

		memset(sendBuf, 0xFF, BIN_BLOCK_SIZE);
		memset(cmdbuf, 0, 64);

		cnt = fread(sendBuf, 1, BIN_BLOCK_SIZE, item.f);
		chksum = blockChksum(sendBuf, BIN_BLOCK_SIZE);

		this->reset(EDB_MODE_BIN);
		this->wrDat(sendBuf, BIN_BLOCK_SIZE);
		this->reset(EDB_MODE_TEXT);

		this->wrStr("BUFCHK\n");
		
		this->rdDat(cmdbuf, 10, &rbcnt);
		
		sscanf_s(cmdbuf, "CHKSUM:%02x\n", &rcshkdum);

		if (rcshkdum != chksum) {
			//printf("chksum error, w:%02x r:%02x\n", chksum, rcshkdum);
			return false;
		}

		if (block_cnt != last_block)
		{
			if (eraseBlock(block_cnt) == false)
			{
				//printf("Erase Block Time Out:%d\n", block_cnt);
				return false;
			}
			//Block Erase;
		}

		if (item.bootImg) {
			sprintf_s(cmdbuf, "PROGP:%d,1\n", page_cnt);
		}
		else {
			sprintf_s(cmdbuf, "PROGP:%d,0\n", page_cnt);
		}
		this->wrStr(cmdbuf);
		if (this->waitStr((char*)"PGOK\n") == false)
		{
			//printf("Program Page Time Out:%d\n", page_cnt);
			return false;
		}

		if (page_cnt % 10 == 0) {

			speed = (getTime() - st) == 0 ? 0 : BIN_BLOCK_SIZE / (getTime() - st);
			uploadedSize = ftell(item.f);
			pageNow = page_cnt;
			blockNow = block_cnt;

			refreshStatus();	//callback function to send status back.

			//cout << "Upload: " << ftell(item.f) << "/" << fsize;
			//cout << " Page:" << page_cnt << " Block:" << block_cnt << " ";
			//printf(" chksum:%02x==%02x, %lld KB/s", chksum, rcshkdum, speed);
			//cout << "  remaining:" << (fsize - ftell(item.f)) / speed / 1000 << "s        \r";
			//fflush(stdout);

		}

		page_cnt += BIN_BLOCK_SIZE / 2048;
		last_block = block_cnt;
	} while (cnt > 0);

	if (item.bootImg)
	{
		//cout << "\nSetting NCB..." << endl;
		memset(cmdbuf, 0, sizeof(cmdbuf));
		sprintf_s(cmdbuf, "MKNCB:%d,%d\n", item.toPage / 64, fsize / 2048);
		this->wrStr(cmdbuf);
		if (this->waitStr((char*)"MKOK\n") == false)
		{
			//printf("Setting NCB Page Time Out:%d\n", item.toPage / 64);
			return false;
		}
	}

	return true;
}

void EDBInterface::reboot()
{
	this->wrStr("REBOOT\n");
}

void EDBInterface::vm_suspend()
{
	this->wrStr("VMSUSPEND\n");
}

void EDBInterface::vm_resume()
{
	this->wrStr("VMRESUME\n");
}

void EDBInterface::vm_reset()
{
	this->wrStr("VMRESET\n");
}

void EDBInterface::mscmode()
{
	this->wrStr("MSCDATA\n");
}

bool EDBInterface::ping()
{
	char buf[16];
	DWORD cnt;
	int retry = 5;
	int ret;
	
	if (USBHighSpeed) {
		while (retry > 0)
		{
			memset(wrBuf, 0, sizeof(wrBuf));
			strcpy_s(wrBuf, "PING\n");
			SetFilePointer(hCMDf, 0, NULL, FILE_BEGIN);
			ret = WriteFile(hCMDf, wrBuf, sizeof(wrBuf), &cnt, NULL);
			if (ret)
			{
				SetFilePointer(hCMDf, 0, NULL, FILE_BEGIN);
				ret = ReadFile(hCMDf, wrBuf, sizeof(wrBuf), &cnt, NULL);
				if (ret)
				{
					if (strcmp(wrBuf, "PONG\n") == 0)
					{
						return true;
					}
					Sleep(2000);
					retry--;
				}
			}
			else
			{
				this->reset(EDB_MODE_TEXT);
				Sleep(2000);
				retry--;
			}
		}
	}
	else {
		while (retry > 0)
		{
			com.WriteStr("PING\n");
			memset(buf, 0, 16);
			if (com.Read(buf, 5, &cnt))
			{
				if (strcmp(buf, "PONG\n") == 0)
				{
					return true;
				}
				Sleep(2000);
				retry--;
			}
			else
			{
				Sleep(2000);
				this->reset(EDB_MODE_TEXT);
				retry--;
			}
		}
	}

	return false;
}

void EDBInterface::close()
{
	if (USBHighSpeed) {
		CloseHandle(hCMDf);
		CloseHandle(hDATf);
	}
	else {
		com.Close();
	}
}



LPWSTR ConvertCharToLPWSTR(const char* szString)

{

	int dwLen = int(strlen(szString)) + 1;

	int nwLen = MultiByteToWideChar(CP_ACP, 0, szString, dwLen, NULL, 0);//算出合适的长度

	LPWSTR lpszPath = new WCHAR[dwLen];

	MultiByteToWideChar(CP_ACP, 0, szString, dwLen, lpszPath, nwLen);

	return lpszPath;

}


char getp()
{
	DWORD size = GetLogicalDriveStrings(0, NULL);
	DWORD namesize = 12;
	DWORD serialnumber;
	DWORD maxlen;
	DWORD fileflag;
	DWORD sysnamesize = 10;

	if (size != 0)
	{
		HANDLE heap = ::GetProcessHeap();
		LPWSTR lp = (LPWSTR)HeapAlloc(heap, HEAP_ZERO_MEMORY, size * sizeof(TCHAR));
		GetLogicalDriveStrings(size * sizeof(TCHAR), lp);

		while (*lp != 0)
		{
			LPTSTR namebuf = new TCHAR[namesize];
			LPTSTR sysnamebuf = new TCHAR[sysnamesize];

			GetVolumeInformation(lp, namebuf, namesize, &serialnumber, &maxlen, &fileflag, sysnamebuf, sysnamesize);
			
			if (lstrcmpW(_T("EOSRECDISK"), namebuf) == 0)
			{
				if (lstrcmpW(_T("FAT"), sysnamebuf) == 0)
				{
					return *lp;
				}
			}

			lp = _tcschr(lp, 0) + 1;
		}
	}
	return 0;
}



bool EDBInterface::open(bool mode)
{
	int retry = 2;
	USBHighSpeed = mode;
	if (USBHighSpeed) {
		char d = getp();
		while (d == 0)
		{
			//cout << "Waiting USB CDC Connect..." << endl;
			d = getp();
			Sleep(2000);
			retry--;
			if (retry == 0) {
				return false;
			}
		}
		char c_cmd_path[] = "X:/CMD_PORT";
		char c_dat_path[] = "X:/DAT_PORT";

		c_cmd_path[0] = d;
		c_dat_path[0] = d;

		hCMDf = CreateFile(ConvertCharToLPWSTR(c_cmd_path), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL |
			FILE_FLAG_NO_BUFFERING |
			FILE_FLAG_WRITE_THROUGH, NULL);
		hDATf = CreateFile(ConvertCharToLPWSTR(c_dat_path), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL |
			FILE_FLAG_NO_BUFFERING |
			FILE_FLAG_WRITE_THROUGH, NULL);

		if (hCMDf == INVALID_HANDLE_VALUE || hDATf == INVALID_HANDLE_VALUE)
		{
			//cout << "Failed to open Device." << endl;
			return false;
		}

	}
	else {


		std::string COM = findUsbSerialCom();
		while (COM == "NONE")
		{
			//cout << "Waiting USB CDC Connect..." << endl;
			Sleep(2000);
			COM = findUsbSerialCom();
		}
		//cout << "Select:" << COM << endl;

		//QMessageBox::information(nullptr, "", QString::fromStdString(COM));

		if (com.Open(COM) == false)
		{
			//cout << "Open Serial Port:" << COM << " Failed.\n" << endl;
			Sleep(2000);
			return false;
		}
		com.Set();
	}
	return true;
}
#pragma once
#include "libusb/libusb.h"

#include "CComHelper.h"
#include "WinReg.h"


#define INFO(fmt, ...) printf(fmt, __VA_ARGS__)


#define EOS_VID		0xCAFE
#define EOS_PID		0x4003
// Device endpoint(s)
#define EP_IN 0x83
#define EP_OUT 0x03

#define MY_CONFIG 1
#define MY_INTF 2


#define EDB_MODE_BIN	true
#define EDB_MODE_TEXT	false


#define BIN_BLOCK_SIZE	32768



typedef struct flashImg
{
	FILE* f;
	char* filename;
	uint32_t toPage;
	bool bootImg;
}flashImg;



class EDBInterface
{
private:
	libusb_device_handle* usbdev = NULL; /* the device handle */
	bool USBHighSpeed = false;
	CComHelper com;
	char sendBuf[BIN_BLOCK_SIZE];
	HANDLE hCMDf = NULL;
	HANDLE hDATf = NULL;
	char wrBuf[512];

public:
	void reset(bool mode);
	bool waitStr(char* str);
	void wrStr(const char* str);
	bool wrDat(char* dat, size_t len);
	bool rdDat(char* dat, size_t len, size_t* rbcnt);
	bool eraseBlock(unsigned int block);
	int flash(flashImg item);
	void reboot();
	void vm_suspend();
	void vm_resume();
	void vm_reset();
	void mscmode();
	bool ping();
	void close();
	bool open(bool mode);
};
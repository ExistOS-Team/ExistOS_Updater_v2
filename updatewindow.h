#ifndef UPDATEWINDOW_H
#define UPDATEWINDOW_H

#define UNCONNECT_MODE 0
#define HOSTLINK_MODE 1
//#define EDB_TEXT 2
#define EDB_BIN 3

#define UPDATE_SYSTEM 1
#define UPDATE_OSLOADER 2

#define MAX_PAGE 65536

#define HOSTLINK_VID 0x066F
#define HOSTLINK_PID 0x3770

#define REBOOT_INTERVAL 4000
#define REBOOT_EDB_INTERVAL 3000
#define REBOOT_RETRY_TIME 3

#define DEFAULT_OSLOADER_PAGE 1408
#define DEFAULT_SYSTEM_PAGE 1984

#define UPDATE_NONE 0
#define UPDATE_PROCESSING 1

//EDB includes//
#include <stdint.h>
#include "edb/CComHelper.h"
#include "edb/WinReg.h"
#include <time.h>
#include "edb/EDBInterface.h"
////////////////

#include <libusb/libusb.h>
#include <sb_loader_DLL.h>

#include <QDialog>
#include <functional>

#include <time.h>

typedef std::function<void()> callBack;

extern long long speed, uploadedSize, pageNow, blockNow, fsize;		//edb status

namespace Ui {
class updateWindow;
}

class updateWindow : public QDialog
{
    Q_OBJECT

public:
    explicit updateWindow(QWidget *parent = nullptr);
    ~updateWindow();

    void addLine(const QString &text);
    void clear();
    void refresh();

    void setProgressBarVisible(bool isVisible);
    void setProgressBarValue(int n);
    
    bool startUpdate(const QList<int>& work, const QString& OSLoader_path, const QString& System_path, const int& page_OSLoader, const int& page_System);

    int searchDevices();

    int searchForDevices();

    void refreshStatus();

    bool reboot();

signals:
    void sendUpdateStatus(int status);

private:
    Ui::updateWindow *ui;

    //edb things//
    vector<flashImg> imglist;
    EDBInterface edb;
    //////////////

    int link_mode = UNCONNECT_MODE;

};

#endif // UPDATEWINDOW_H

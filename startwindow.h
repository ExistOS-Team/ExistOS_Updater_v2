#ifndef STARTWINDOW_H
#define STARTWINDOW_H

#define UNCONNECT_MODE 0
#define HOSTLINK_MODE 1
#define EDB_TEXT 2
#define EDB_BIN 3

#define UPDATE_SYSTEM 1
#define UPDATE_OSLOADER 2

#define MAX_PAGE 65536

#define HOSTLINK_VID 0x066F
#define HOSTLINK_PID 0x3770

#define REBOOT_INTERVAL 6000

//define texts
constexpr auto VERSION                           =     " ver 1.0.2.1";
constexpr auto TEXT_DEVICE_DISCONNECTED          =     "Device Disconnected";
constexpr auto TEXT_DEVICE_CONNECTED_HOSTLINK    =     "Device Connected [HostLink Mode]";
constexpr auto TEXT_DEVICE_CONNECTED_EDB_TEXT    =     "Device Connected [Text Mode EDB]";
constexpr auto TEXT_DEVICE_CONNECTED_EDB_BIN     =     "Device Connected [Bin Mode EDB]";
constexpr auto TEXT_SEARCHING                    =     "Searching for Devices...";
constexpr auto TEXT_UPDATING                     =     "Updating...  DO NOT DISCONNECT";
//

//EDB includes
#include <stdint.h>
#include "edb/CComHelper.h"
#include "edb/WinReg.h"
#include <time.h>
#include "edb/EDBInterface.h"
//

#include <libusb/libusb.h>

#include <QMainWindow>
#include <QGraphicsScene>
#include <QPixmap>
#include <QFileDialog>
#include <QIcon>
#include <QTextCodec>
#include <QMessageBox>
#include <QFileInfo>
#include <QProcess>
#include <QByteArray>
#include <QFile>

#include <about.h>
#include <updatewindow.h>
#include <options.h>
#include <wait.h>



QT_BEGIN_NAMESPACE
namespace Ui { class startWindow; }
QT_END_NAMESPACE

class startWindow : public QMainWindow
{
    Q_OBJECT

public:
    startWindow(QWidget *parent = nullptr);
    ~startWindow();

private:
    Ui::startWindow *ui;
    QGraphicsScene *image_OSLoader = new QGraphicsScene;
    QGraphicsScene *image_System = new QGraphicsScene;
    QGraphicsScene *image_OSLoader_System= new QGraphicsScene;

    About *aboutWindow = new About(this);
    updateWindow* updWindow = new updateWindow(this);
    Options* optionsWindow = new Options(this);
    wait* waitWindow = new wait(this);

    int page_OSLoader = DEFAULT_OSLOADER_PAGE;
    int page_System = DEFAULT_SYSTEM_PAGE;
    int edbMode = EDB_BIN;

    int link_mode = UNCONNECT_MODE;

    const QStringList link_texts = {
        TEXT_DEVICE_DISCONNECTED,
        TEXT_DEVICE_CONNECTED_HOSTLINK,
        TEXT_DEVICE_CONNECTED_EDB_TEXT,
        TEXT_DEVICE_CONNECTED_EDB_BIN
    };

    //edb things
    vector<flashImg> imglist;
    EDBInterface edb;
    //

    bool startUpdate(const QList<int> &work);

    void setButtonStatus(const bool& O, const bool& S, const bool& OandS);

    bool searchRecoveryModeDevice();
    void openProcess(const QString& path, const QStringList& argu);
    QProcess* process = new QProcess(this);

private slots:
    void on_button_OSLoader_path_clicked();
    void on_button_System_path_clicked();
    void on_pushButton_about_clicked();
    void on_pushButton_options_clicked();
    void on_pushButton_refresh_clicked();
    void on_pushButton_update_O_clicked();
    void on_pushButton_update_S_clicked();
    void on_pushButton_update_OandS_clicked();

    void getReturnData(int OSLoader, int System, int edb);
    void readResult(int exitCode);
};
#endif // STARTWINDOW_H

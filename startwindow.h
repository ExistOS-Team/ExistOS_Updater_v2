#ifndef STARTWINDOW_H
#define STARTWINDOW_H

#define UNCONNECT_MODE 0
#define HOSTLINK_MODE 1
//#define EDB_TEXT 2
#define EDB_BIN 3

#define UPDATE_SYSTEM 1
#define UPDATE_OSLOADER 2

#define MAX_PAGE 65536

//define texts//
constexpr auto VERSION                           =     "v1.1.0";
constexpr auto TEXT_DEVICE_DISCONNECTED          =     "Device Disconnected";
constexpr auto TEXT_DEVICE_CONNECTED_HOSTLINK    =     "Device Connected [HostLink Mode]";
constexpr auto TEXT_DEVICE_CONNECTED_EDB_BIN     =     "Device Connected [EDB Mode]";
////////////////


#include <QMainWindow>
#include <QGraphicsScene>
#include <QPixmap>
#include <QFileDialog>
#include <QIcon>
#include <QTextCodec>
#include <QMessageBox>
#include <QFileInfo>
#include <QByteArray>
#include <QSettings>
#include <QThread>

#include <about.h>
#include <updatewindow.h>
#include <options.h>


QT_BEGIN_NAMESPACE
namespace Ui { class startWindow; }
QT_END_NAMESPACE

class startWindow : public QMainWindow
{
    Q_OBJECT

        QThread deviceUpdateThread;
        
public:
    startWindow(QWidget *parent = nullptr);
    ~startWindow();

    void setButtonStatus(const bool& O, const bool& S, const bool& OandS);

    void setStatus(int link_mode);

    bool updateDevice(const QList<int>& work);

private:
    Ui::startWindow* ui;

    QGraphicsScene* image_OSLoader = new QGraphicsScene;
    QGraphicsScene* image_System = new QGraphicsScene;
    QGraphicsScene* image_OSLoader_System= new QGraphicsScene;

    About* aboutWindow = new About(this);
    updateWindow* updWindow = new updateWindow(this);
    Options* optionsWindow = new Options(this);

    QSettings* ini = new QSettings("config.ini", QSettings::IniFormat);

    int page_OSLoader = DEFAULT_OSLOADER_PAGE;
    int page_System = DEFAULT_SYSTEM_PAGE;

    int link_mode = UNCONNECT_MODE;

private slots:
    void on_button_OSLoader_path_clicked();
    void on_button_System_path_clicked();
    void on_pushButton_about_clicked();
    void on_pushButton_options_clicked();
    void on_pushButton_refresh_clicked();
    void on_pushButton_update_O_clicked();
    void on_pushButton_update_S_clicked();
    void on_pushButton_update_OandS_clicked();

    void getReturnData(int OSLoader, int System);
};



#endif // STARTWINDOW_H

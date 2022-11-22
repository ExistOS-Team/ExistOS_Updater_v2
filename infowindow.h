#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include <QDialog>
#include <QTimer>

extern unsigned long long speed, uploadedSize, pageNow, blockNow;		//edb status

namespace Ui {
class infoWindow;
}

class infoWindow : public QDialog
{
    Q_OBJECT

public:
    explicit infoWindow(QWidget *parent = nullptr);
    ~infoWindow();

    void timer_start(int interval);
    void timer_stop();

    void refreshBasicInfo(QString name, unsigned long long file_size);
    void clear();

private:
    Ui::infoWindow *ui;
    QTimer* info_timer = new QTimer;

    unsigned long long _fsize = 1;

private slots:
    void refreshInfo();
};

#endif // INFOWINDOW_H

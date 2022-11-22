#include "infowindow.h"
#include "ui_infowindow.h"

infoWindow::infoWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::infoWindow)
{
    ui->setupUi(this);

    connect(info_timer, SIGNAL(timeout()), this, SLOT(refreshInfo()));
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint);
    setWindowTitle("Flash Status");
}

infoWindow::~infoWindow()
{
    delete ui;
}

void infoWindow::timer_start(int interval) {
    info_timer->setInterval(interval);
    info_timer->start();
}

void infoWindow::timer_stop() {
    info_timer->stop();
}

void infoWindow::refreshInfo() {
    ui->lineEdit_speed->setText(QString::number(speed) + " KB/s");
    ui->lineEdit_uploaded->setText(QString::number((int)(uploadedSize / 100) / 10.0) + " KB");
    ui->lineEdit_page->setText(QString::number(pageNow));
    ui->lineEdit_block->setText(QString::number(blockNow));
    ui->progressBar->setValue((int)((double)uploadedSize / _fsize * 10000) / 100.0);
}

void infoWindow::refreshBasicInfo(QString name, unsigned long long file_size) {
    _fsize = file_size;
    ui->lineEdit_flashing->setText(name);
    ui->lineEdit_size->setText(QString::number((int)(_fsize / 100) / 10.0) + " KB");
}

void infoWindow::clear() {
    _fsize = 0;
    ui->lineEdit_block->clear();
    ui->lineEdit_page->clear();
    ui->lineEdit_size->clear();
    ui->lineEdit_speed->clear();
    ui->lineEdit_uploaded->clear();
}
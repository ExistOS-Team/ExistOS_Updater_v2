#include "wait.h"
#include "ui_wait.h"

wait::wait(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::wait)
{
    ui->setupUi(this);
    this->resize(QSize(320, 0));
    setWindowTitle("Searching for Devices...");
}

wait::~wait()
{
    delete ui;
}

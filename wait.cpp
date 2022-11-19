#include "wait.h"
#include "ui_wait.h"

wait::wait(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::wait)
{
    ui->setupUi(this);
    setWindowTitle("Searching for Devices...");
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint);
}

wait::~wait()
{
    delete ui;
}

void wait::refresh() {
    qApp->processEvents();

}
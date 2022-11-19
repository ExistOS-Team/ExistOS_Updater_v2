#include "options.h"
#include "ui_options.h"

Options::Options(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Options)
{
    ui->setupUi(this);
    setWindowTitle("Advanced Options");
    //setWindowFlag(Qt::SubWindow);
}

Options::~Options()
{
    delete ui;
}

void Options::set(const int &page_OSLoader, const int &page_System){
    ui->spinBox_OSLoader_Page->setValue(page_OSLoader);
    ui->spinBox_System_Page->setValue(page_System);
}

void Options::on_buttonBox_accepted()
{
    emit returnData(ui->spinBox_OSLoader_Page->value(), ui->spinBox_System_Page->value());
}


void Options::on_pushButton_clicked()
{
    ui->spinBox_OSLoader_Page->setValue(DEFAULT_OSLOADER_PAGE);
    ui->spinBox_System_Page->setValue(DEFAULT_SYSTEM_PAGE);
}


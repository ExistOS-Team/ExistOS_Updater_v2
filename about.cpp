#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    setWindowTitle("About");
}

About::~About()
{
    delete ui;
}

void About::on_button_About_Qt_clicked()
{
    QMessageBox::aboutQt(this);
}

void About::on_button_OK_clicked() {
    this->hide();
}


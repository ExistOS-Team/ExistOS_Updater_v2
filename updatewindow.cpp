#include "updatewindow.h"
#include "ui_updatewindow.h"

updateWindow::updateWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::updateWindow)
{
    ui->setupUi(this);
    setWindowTitle("Update Dialog");
    setWindowFlag(Qt::SubWindow);
}

updateWindow::~updateWindow()
{
    delete ui;
}

void updateWindow::addLine(const QString &text){
    time_t currentTime;
    struct tm* pstTime;
    char timeString[32];
    time(&currentTime);
    pstTime = localtime(&currentTime);
    strftime(timeString, 32, "%H:%M:%S", pstTime);
    ui->textEdit->setText(ui->textEdit->toPlainText() + "["+ timeString + "] " + text + "\n");
    ui->textEdit->moveCursor(QTextCursor::End);
    qApp->processEvents();
}

void updateWindow::clear()
{
    ui->textEdit->clear();
}


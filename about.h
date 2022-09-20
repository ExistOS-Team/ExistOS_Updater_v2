#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>
#include <QMessageBox>
#include <QUrl>
#include <QDesktopServices>

namespace Ui {
class About;
}

class About : public QDialog
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = nullptr);
    ~About();

private slots:
    void on_button_About_Qt_clicked();
    void on_button_OK_clicked();
    void on_button_Website_clicked();

private:
    Ui::About *ui;
};

#endif // ABOUT_H

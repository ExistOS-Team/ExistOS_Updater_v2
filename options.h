#ifndef OPTIONS_H
#define OPTIONS_H

#include <QDialog>

#define DEFAULT_OSLOADER_PAGE 1408
#define DEFAULT_SYSTEM_PAGE 1984

namespace Ui {
class Options;
}

class Options : public QDialog
{
    Q_OBJECT

public:
    explicit Options(QWidget *parent = nullptr);
    ~Options();

    void set(const int &page_OSLoader, const int &page_System, const int &edbMode);

signals:
    void returnData(int page_OSLoader, int page_System, int edbMode);

private slots:
    void on_buttonBox_accepted();

    void on_pushButton_clicked();

private:
    Ui::Options *ui;
};

#endif // OPTIONS_H

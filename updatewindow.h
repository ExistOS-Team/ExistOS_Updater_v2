#ifndef UPDATEWINDOW_H
#define UPDATEWINDOW_H

#include <QDialog>
#include <time.h>

namespace Ui {
class updateWindow;
}

class updateWindow : public QDialog
{
    Q_OBJECT

public:
    explicit updateWindow(QWidget *parent = nullptr);
    ~updateWindow();

    void addLine(const QString &text);
    void clear();
    void refresh();

    void setProgressBarVisible(bool isVisible);
    void setProgressBarValue(int n);
private:
    Ui::updateWindow *ui;

};

#endif // UPDATEWINDOW_H

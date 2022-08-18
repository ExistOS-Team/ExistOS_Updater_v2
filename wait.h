#ifndef WAIT_H
#define WAIT_H

#include <QDialog>

namespace Ui {
class wait;
}

class wait : public QDialog
{
    Q_OBJECT

public:
    explicit wait(QWidget *parent = nullptr);

    void refresh();

    ~wait();

private:
    Ui::wait *ui;
};

#endif // WAIT_H

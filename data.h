#ifndef DATA_H
#define DATA_H

#include <QWidget>

namespace Ui {
class People;
}

class People : public QWidget
{
    Q_OBJECT

public:
    explicit People(QWidget *parent = nullptr);
    ~People();

private:
    Ui::People *ui;
};

#endif // DATA_H

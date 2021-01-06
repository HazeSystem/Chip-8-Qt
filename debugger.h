#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QMainWindow>
#include <QString>
#include <QTextStream>
#include <string>
#include <vector>

namespace Ui {
class Debugger;
}

class Debugger : public QMainWindow
{
    Q_OBJECT

public:
    explicit Debugger(QWidget *parent = nullptr);
    ~Debugger();
    void recRom(std::vector<unsigned char> rom);

private slots:
    void on_listWidget_itemSelectionChanged();

    void on_listWidget_2_itemSelectionChanged();

    void on_listWidget_3_itemSelectionChanged();

    void on_listWidget_4_itemSelectionChanged();

private:
    Ui::Debugger *ui;
    void addItem(std::vector<QString> item);
};

#endif // DEBUGGER_H

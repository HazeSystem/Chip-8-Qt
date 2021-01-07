#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "chip8.h"

#include <QListWidgetItem>
#include <QMainWindow>
#include <QScrollBar>
#include <QSignalMapper>
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
    bool disassembleRom(Chip8 *c8, QString filepath);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void on_listWidget_itemSelectionChanged();
    void on_listWidget_2_itemSelectionChanged();
    void on_listWidget_3_itemSelectionChanged();
    void on_listWidget_4_itemSelectionChanged();
    void scroll();

private:
    Ui::Debugger *ui;
    void addItem(std::vector<QString> item);
};

#endif // DEBUGGER_H

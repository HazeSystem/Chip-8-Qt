#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "chip8.h"
#include "SDL2Widget.h"

#include <QFontDatabase>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QScrollBar>
#include <QSettings>
#include <QSignalMapper>
#include <QString>
#include <QTextStream>
#include <string>
#include <vector>

#define UI_VERSION 1

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
    SDL2Widget *sw;

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void on_listWidget_itemSelectionChanged();
    void on_listWidget_2_itemSelectionChanged();
    void on_listWidget_3_itemSelectionChanged();
    void on_listWidget_4_itemSelectionChanged();
    void init();

private:
    Ui::Debugger *ui;
    void addItem(std::vector<QString> item);
};

#endif // DEBUGGER_H

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
#include <QStyledItemDelegate>
#include <QString>
#include <QTextStream>
#include <QTranslator>
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
    bool disassembleRom(QString filepath);
    void updateWidgets();
//    static Debugger* getDebugContext();

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
//    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private slots:
    void on_listWidget_itemSelectionChanged();
    void on_listWidget_2_itemSelectionChanged();
    void on_listWidget_3_itemSelectionChanged();
    void on_listWidget_4_itemSelectionChanged();
    void init();

    void on_leftRegisterListWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_rightRegisterListWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_stackListWidget_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::Debugger *ui;
    void addDisassemblyViewItem(std::vector<QString> item);
    void addRegisterViewItem();
    void addStackViewItem();
};

#endif // DEBUGGER_H

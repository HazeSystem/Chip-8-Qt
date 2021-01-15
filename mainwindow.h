#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "debugger.h"
#include "settings.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>
#include <QLayout>
#include <QMainWindow>
#include <QSplitter>
#include <QTextEdit>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    SDL2Widget* sw;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void openRom();
    void openSettings();
    void openDebugger();

private:
    Ui::MainWindow *ui;
    Debugger *debugWindow;
    Settings *settingsWindow;
    void sendRom(Debugger *debugWindow);
    unsigned int fileSize;
};

#endif // MAINWINDOW_H

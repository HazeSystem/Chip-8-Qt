#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SDL2Widget.h"
#include "chip8.h"

#include<iostream>
#include<fstream>

QByteArray rom;
QString filepath;
std::vector<unsigned char> rom_out;
Debugger* dbg;
MainWindow* mainctx;
bool romLoaded = false;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setFixedSize(512, 282);
    mainctx = this;

    QSettings settings("adalovegirls", "chip8-qt");
    restoreGeometry(settings.value("emu_geometry").toByteArray());

    // File
    connect(ui->actionOpenROM, SIGNAL(triggered()), this, SLOT(openRom()));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(openSettings()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));

    //Debug
    connect(ui->actionOpenDebugger, SIGNAL(triggered()), this, SLOT(openDebugger()));
}

MainWindow::~MainWindow() {
    delete ui;
}

MainWindow* MainWindow::getMainWindowContext() {
    return mainctx;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    sw->recKey(event->key(), 1);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    sw->recKey(event->key(), 0);
}

void MainWindow::openRom() {
    isDebuggerCreated = false;
    romLoaded = false;

    filepath =  QFileDialog::getOpenFileName(
              this,
              tr("Open Document"),
              QDir::currentPath(),
              tr("Chip-8 ROM file (*.ch8)"));

    if(!filepath.isNull()) {
      qDebug() << "selected file path : " << filepath.toUtf8();
    }

    std::string filename = filepath.toUtf8().constData();
    std::string fnFormatted = filename;

    // Remove directory if present.
    // Do this before extension removal incase directory has a period character.
    const size_t last_slash_idx = fnFormatted.find_last_of("\\/");
    if (std::string::npos != last_slash_idx)
    {
        fnFormatted.erase(0, last_slash_idx + 1);
    }

    // Remove extension if present.
    const size_t period_idx = fnFormatted.rfind('.');
    if (std::string::npos != period_idx)
    {
        fnFormatted.erase(period_idx);
    }

    this->setWindowTitle("Chip-8 Qt - " + QString::fromStdString(fnFormatted));

    std::ifstream input(filename, std::ios::binary);
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});

    romLoaded = true;

    sw = SDL2Widget::getSDLContext();
    sw->running = true;

    sw->loadRom(buffer);
    sw->breakPoint();
}

void MainWindow::openDebugger() {
    if (romLoaded && !isDebuggerCreated) {
        debugWindow = new Debugger();
        dbg = debugWindow;

        if (!filepath.isNull()) {
            debugWindow->disassembleRom(filepath);
            isDebuggerCreated = true;
        }
        debugWindow->show();
    }
    else if (romLoaded && isDebuggerCreated) {
        if (debugWindow->isVisible())
            debugWindow->activateWindow();
        else
            debugWindow->show();
    }
}

void MainWindow::openSettings() {
    settingsWindow = new Settings();
    settingsWindow->show();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (dbg)
        dbg->close();

    QSettings settings("adalovegirls", "chip8-qt");
    settings.setValue("emu_geometry", saveGeometry());
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (romLoaded)
        sw->run();
}

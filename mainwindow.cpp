#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SDL2Widget.h"
#include "chip8.h"

std::size_t fileSize;
QByteArray rom;
std::vector<unsigned char> rom_out;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setFixedSize(512, 282);

    // File
    connect(ui->actionOpenROM, SIGNAL(triggered()), this, SLOT(openRom()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));

    //Debug
    connect(ui->actionOpenDebugger, SIGNAL(triggered()), this, SLOT(openDebugger()));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    SDL2Widget::recKey(event->key(), 1);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    SDL2Widget::recKey(event->key(), 0);
}

void MainWindow::openRom() {
    QString filename =  QFileDialog::getOpenFileName(
              this,
              "Open Document",
              QDir::currentPath(),
              "Chip-8 ROM file (*.ch8)");

        if(!filename.isNull()) {
          qDebug() << "selected file path : " << filename.toUtf8();
        }

        QFile file(filename);
           if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
               return;

           while (!file.atEnd()) {
               rom.append(file.readLine());
           }

        fileSize = rom.size();
        for (unsigned int i = 0; i < fileSize; i++)
            rom_out.push_back(rom[i]);
        rom = NULL;

        SDL2Widget::loadRom(rom_out);
}

void MainWindow::openDebugger() {
    debugWindow = new Debugger();
    sendRom(debugWindow);
    debugWindow->show();
}

void MainWindow::sendRom(Debugger *debugWindow) {
    debugWindow->recRom(rom_out);
}

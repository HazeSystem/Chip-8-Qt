#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SDL2Widget.h"
#include "chip8.h"

QByteArray rom;
QString filepath;
std::vector<unsigned char> rom_out;
Chip8 *chip8;
Debugger *dbg;

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
    filepath =  QFileDialog::getOpenFileName(
              this,
              "Open Document",
              QDir::currentPath(),
              "Chip-8 ROM file (*.ch8)");

    if(!filepath.isNull()) {
      qDebug() << "selected file path : " << filepath.toUtf8();
    }

    std::string filename = filepath.toUtf8().constData();

    // Remove directory if present.
    // Do this before extension removal incase directory has a period character.
    const size_t last_slash_idx = filename.find_last_of("\\/");
    if (std::string::npos != last_slash_idx)
    {
        filename.erase(0, last_slash_idx + 1);
    }

    // Remove extension if present.
    const size_t period_idx = filename.rfind('.');
    if (std::string::npos != period_idx)
    {
        filename.erase(period_idx);
    }

    this->setWindowTitle("Chip-8 Qt - " + QString::fromStdString(filename));

    QFile file(filepath);
       if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
           return;

       while (!file.atEnd()) {
           rom.append(file.readLine());
       }

        for (int i = 0; i < rom.size(); i++)
        rom_out.push_back(rom[i]);
    rom = NULL;

    SDL2Widget::loadRom(rom_out);
}

void MainWindow::openDebugger() {
    debugWindow = new Debugger();
    dbg = debugWindow;
    chip8 = SDL2Widget::getContext();
    bool success = !debugWindow->disassembleRom(chip8, filepath);
    if (!success)
        std::cout << "ROM not loaded" << std::endl;
    debugWindow->show();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    dbg->close();
}

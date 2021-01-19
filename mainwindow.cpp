#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SDL2Widget.h"
#include "chip8.h"

QByteArray rom;
QString filepath;
std::vector<unsigned char> rom_out;
Debugger* dbg;
MainWindow* mainctx;

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

    filepath =  QFileDialog::getOpenFileName(
              this,
              tr("Open Document"),
              QDir::currentPath(),
              tr("Chip-8 ROM file (*.ch8)"));

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

    if (!rom_out.empty())
        rom_out.clear();

    for (int i = 0; i < rom.size(); i++)
        rom_out.push_back(rom[i]);

    rom.clear();

    sw = SDL2Widget::getSDLContext();
    sw->running = true;

    sw->loadRom(rom_out);
}

void MainWindow::openDebugger() {
    if (!isDebuggerCreated) {
        debugWindow = new Debugger();
        dbg = debugWindow;

        if (!filepath.isNull()) {
            debugWindow->disassembleRom(filepath);
            isDebuggerCreated = true;
        }
        debugWindow->show();
    }
    else if (isDebuggerCreated) {
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
    if (!rom_out.empty() && !sw->running)
        sw->run();
}

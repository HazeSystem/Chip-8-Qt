#include "debugger.h"
#include "ui_debugger.h"
#include "mainwindow.h"
#include "chip8.h"
#include "SDL2Widget.h"
#include "c8dasm/c8dasm.h"
#include "hexview/qhexview.h"
#include "hexview/document/buffer/qmemorybuffer.h"

//QByteArray rom;
std::vector<std::vector<QString>> disassembly;
std::vector<unsigned char> disassemblyViewbuffer;
size_t ram_size;
C8Dasm c8dasm;
QByteArray hexViewBuffer;

Debugger::Debugger(QWidget *parent) : QMainWindow(parent),  ui(new Ui::Debugger) {
    ui->setupUi(this);

    QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    if(!(f.styleHint() & QFont::Monospace))
    {
        f.setFamily("Monospace"); // Force Monospaced font
        f.setStyleHint(QFont::TypeWriter);
    }

    this->setFont(f);

    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));

    connect(ui->listWidget->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->listWidget_2->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->listWidget_2->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->listWidget_3->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->listWidget_3->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->listWidget_4->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->listWidget_4->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->listWidget->verticalScrollBar(), SLOT(setValue(int)));

//    ui->splitter->setStretchFactor(0, 1);
//    ui->splitter_2->setStretchFactor(1, 1);
//    ui->splitter_3->setStretchFactor(0, 1);

    this->setStyleSheet("QWidget { border: 0; }");
    ui->listWidget->verticalScrollBar()->setStyleSheet("QScrollBar {width:0px;}");
    ui->listWidget_2->verticalScrollBar()->setStyleSheet("QScrollBar {width:0px;}");
    ui->listWidget_3->verticalScrollBar()->setStyleSheet("QScrollBar {width:0px;}");
}

Debugger::~Debugger() {
    delete ui;
}

bool Debugger::disassembleRom(Chip8 *c8, QString filepath) {
    bool empty = c8->memory.empty();
    ram_size = 0x200 + c8->rom_size;

    for (size_t i = 0; i < ram_size; i++)
        hexViewBuffer.push_back(c8->memory[i]);

    if (!empty) {
        for (unsigned int i = 0; i < ram_size; i++)
            disassemblyViewbuffer.push_back(c8->memory[i]);

        disassembly = c8dasm.Disassemble(disassemblyViewbuffer);

        for (unsigned int i = 0; i < disassembly.size(); i++)
                addItem(disassembly[i]);

        this->setWindowTitle("Chip-8 Qt debugger - " + filepath);
    }

    QHexDocument* document = QHexDocument::fromMemory<QMemoryBuffer>(hexViewBuffer);
//    QHexView* hexview = new QHexView();
    ui->hexViewWidget->setDocument(document);                  // Associate QHexEditData with this QHexEdit
//    this->setCentralWidget(hexview);
//    ui->hexViewWidget->set
    return empty;
}

void Debugger::addItem(std::vector<QString> item) {
    for (size_t i = 0; i < 2; i++) {
        QString s;
        s = QString("%1").arg(item[i].toInt(), 4, 16, QChar{'0'}).toUpper();
        item[i] = s;
    }

    ui->listWidget->addItem(item[0]);
    ui->listWidget_2->addItem(item[1]);
    ui->listWidget_3->addItem(item[2]);
    if (!item[3].isEmpty())
        ui->listWidget_4->addItem(item[3]);
    if (item[3].isEmpty())
        ui->listWidget_4->addItem("");
}

void Debugger::on_listWidget_itemSelectionChanged() {
    auto row = ui->listWidget->currentRow();
    ui->listWidget_2->setCurrentRow(row);
    ui->listWidget_3->setCurrentRow(row);
    ui->listWidget_4->setCurrentRow(row);
}

void Debugger::on_listWidget_2_itemSelectionChanged() {
    auto row = ui->listWidget_2->currentRow();
    ui->listWidget->setCurrentRow(row);
    ui->listWidget_3->setCurrentRow(row);
    ui->listWidget_4->setCurrentRow(row);
}

void Debugger::on_listWidget_3_itemSelectionChanged() {
    auto row = ui->listWidget_3->currentRow();
    ui->listWidget->setCurrentRow(row);
    ui->listWidget_2->setCurrentRow(row);
    ui->listWidget_4->setCurrentRow(row);
}

void Debugger::on_listWidget_4_itemSelectionChanged() {
    auto row = ui->listWidget_4->currentRow();
    ui->listWidget->setCurrentRow(row);
    ui->listWidget_2->setCurrentRow(row);
    ui->listWidget_3->setCurrentRow(row);
}

void Debugger::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    // Call slot via queued connection so it's called from the UI thread after this method has returned and the window has been shown
    QMetaObject::invokeMethod(this, "scroll", Qt::ConnectionType::QueuedConnection);
}

void Debugger::scroll() {
    ui->listWidget->scrollToItem(ui->listWidget->item(0x100), QAbstractItemView::PositionAtCenter);
    ui->listWidget->setCurrentRow(0x100);
}

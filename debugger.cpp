#include "debugger.h"
#include "ui_debugger.h"
#include "mainwindow.h"
#include "chip8.h"
#include "c8dasm/c8dasm.h"
#include "hexview/qhexview.h"
#include "hexview/document/buffer/qmemorybuffer.h"

#include <QMainWindow>

std::vector<std::vector<QString>> disassembly;
std::vector<unsigned char> disassemblyViewbuffer;
size_t ram_size;
bool empty;
Chip8 *chip8;
Debugger *dbgctx;
C8Dasm c8dasm;
QByteArray hexViewBuffer;

Debugger::Debugger(QWidget *parent) : QMainWindow(parent),  ui(new Ui::Debugger) {
    ui->setupUi(this);
    dbgctx = this;
    sw = SDL2Widget::getSDLContext();

    QSettings settings("adalovegirls", "chip8-qt");
    restoreGeometry(settings.value("debug_geometry").toByteArray());

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

    this->setStyleSheet("QWidget { border: 0; }");
    ui->listWidget->verticalScrollBar()->setStyleSheet("QScrollBar {width:0px;}");
    ui->listWidget_2->verticalScrollBar()->setStyleSheet("QScrollBar {width:0px;}");
    ui->listWidget_3->verticalScrollBar()->setStyleSheet("QScrollBar {width:0px;}");
}

Debugger::~Debugger() {
    delete ui;
}

void Debugger::updateWidgets() {
    if (chip8)
        addRegisterViewItem();
}

bool Debugger::disassembleRom(QString filepath) {
    chip8 = SDL2Widget::getC8Context();
    empty = chip8->memory.empty();
    ram_size = 0x200 + chip8->rom_size;

    if (!empty) {
        for (size_t i = 0; i < ram_size; i++)
            hexViewBuffer.push_back(chip8->memory[i]);

        for (unsigned int i = 0; i < ram_size; i++)
            disassemblyViewbuffer.push_back(chip8->memory[i]);

        disassembly = c8dasm.Disassemble(disassemblyViewbuffer);

        for (unsigned int i = 0; i < disassembly.size(); i++)
            addDisassemblyViewItem(disassembly[i]);

        addRegisterViewItem();

        this->setWindowTitle(tr("Chip-8 Qt debugger - %1").arg(filepath));
    }

    QHexDocument* document = QHexDocument::fromMemory<QMemoryBuffer>(hexViewBuffer);
    ui->hexViewWidget->setDocument(document);

    return empty;
}

void Debugger::addDisassemblyViewItem(std::vector<QString> item) {
    for (size_t i = 0; i < 2; i++) {
        item[i] = QString("%1").arg(item[i].toInt(), 4, 16, QChar{'0'}).toUpper();
    }

    ui->listWidget->addItem(item[0]);
    ui->listWidget_2->addItem(item[1]);
    ui->listWidget_3->addItem(item[2]);
    if (!item[3].isEmpty())
        ui->listWidget_4->addItem(item[3]);
    if (item[3].isEmpty())
        ui->listWidget_4->addItem("");
}

void Debugger::addRegisterViewItem() {
//    int itemCount = ui->leftRegisterListWidget->count();
//    if (itemCount)
//        ui->leftRegisterListWidget->clear();

    for (int i = 0; i < 16; i++) {
        ui->leftRegisterListWidget->addItem(QString("%1").arg(chip8->V[i], 4, 16, QChar{'0'}).toUpper());
    }
}

Debugger* Debugger::getDebugContext() {
    return dbgctx;
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
    QMetaObject::invokeMethod(this, "init", Qt::ConnectionType::QueuedConnection);
}

void Debugger::closeEvent(QCloseEvent *event) {
    QSettings settings("adalovegirls", "chip8-qt");
    settings.setValue("debug_geometry", saveGeometry());
}

void Debugger::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::ActivationChange)
    {
        if (this->isActiveWindow())
        {
            if (sw->running)
                sw->breakPoint();
        }
    }
}

void Debugger::init() {
    if (!empty) {
        ui->listWidget->scrollToItem(ui->listWidget->item(0x100), QAbstractItemView::PositionAtCenter);
        ui->listWidget->setCurrentRow(0x100);
    }
}

#include "debugger.h"
#include "ui_debugger.h"
#include "mainwindow.h"
#include "chip8.h"
#include "c8asm/assembler.h"
#include "c8dasm/c8dasm.h"
#include "hexview/qhexview.h"
#include "hexview/document/buffer/qmemorybuffer.h"

#include <QInputDialog>
#include <QMainWindow>
#include <algorithm>
#include <iterator>
//#include <chrono>

//double total = 0;

std::vector<std::vector<QString>> disassembly;
std::vector<unsigned char> disassemblyViewBuffer;
std::vector<QString> regBuffer;
bool empty;

std::vector<unsigned char> oldV;
unsigned short pcOld, iOld;
unsigned char spOld, dtOld, stOld;
std::vector<unsigned short> oldStack;

bool regsLoaded = false;
bool stackLoaded = false;
size_t ram_size;

Chip8 *chip8;
Debugger *dbgctx;
SDL2Widget *sdl2;
MainWindow *mainw;

QByteArray hexViewBuffer;
bool documentLoaded = false;

Debugger::Debugger(QWidget *parent) : QMainWindow(parent),  ui(new Ui::Debugger) {
    ui->setupUi(this);
    dbgctx = this;
    sdl2 = SDL2Widget::getSDLContext();

    QSettings settings("adalovegirls", "chip8-qt");
    restoreGeometry(settings.value("debug_geometry").toByteArray());

    QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    if(!(f.styleHint() & QFont::Monospace))
    {
        f.setFamily("Monospace"); // Force Monospaced font
        f.setStyleHint(QFont::TypeWriter);
    }

    this->setFont(f);

    // File
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));

    // Run
    connect(ui->actionRun, SIGNAL(triggered()), sdl2, SLOT(run()));
    connect(ui->actionRun_to_Next_Line, SIGNAL(triggered()), sdl2, SLOT(singleStep()));
    connect(ui->actionAnimate, SIGNAL(triggered()), sdl2, SLOT(animate()));

    connect(ui->listWidget->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->listWidget_2->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->listWidget_2->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->listWidget_3->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->listWidget_3->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->listWidget_4->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->listWidget_4->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->listWidget->verticalScrollBar(), SLOT(setValue(int)));

    this->setStyleSheet("QListWidget { border: 0; }");
    this->setAttribute(Qt::WA_DeleteOnClose, false);

    ui->listWidget->verticalScrollBar()->setStyleSheet("QScrollBar {width:0px;}");
    ui->listWidget_2->verticalScrollBar()->setStyleSheet("QScrollBar {width:0px;}");
    ui->listWidget_3->verticalScrollBar()->setStyleSheet("QScrollBar {width:0px;}");
    ui->splitter->setSizes((QList<int>){310, 358});
}

Debugger::~Debugger() {
    delete ui;
}

void Debugger::disassembleRom(QString filepath) {
    C8Dasm c8dasm;
    chip8 = SDL2Widget::getC8Context();
    ram_size = 0x200 + chip8->rom_size;

    this->setWindowTitle(tr("Chip-8 Qt debugger - %1").arg(filepath));

    if (!hexViewBuffer.isEmpty())
        hexViewBuffer.clear();
    if (!disassemblyViewBuffer.empty())
        disassemblyViewBuffer.clear();

    for (size_t i = 0; i < ram_size; i++)
        hexViewBuffer.push_back(chip8->memory[i]);

    for (unsigned int i = 0; i < ram_size; i++)
        disassemblyViewBuffer.push_back(chip8->memory[i]);

    disassembly = c8dasm.Disassemble(disassemblyViewBuffer);

    addDisassemblyViewItems(disassembly);
    updateWidgets();
    updateCurrentLine();

    QHexDocument* document = QHexDocument::fromMemory<QMemoryBuffer>(hexViewBuffer);
    ui->hexViewWidget->setDocument(document);

    loaded = true;
}

void Debugger::updateWidgets() {
    addRegisterViewItems();
    addStackViewItems();
}

void Debugger::updateCurrentLine() {
    ui->listWidget->scrollToItem(ui->listWidget->item(chip8->pc / 2), QAbstractItemView::EnsureVisible);
    ui->listWidget->setCurrentRow(chip8->pc / 2);
}

void Debugger::updateDisassemblyViewItem(int index, QString text) {
    size_t found  = text.indexOf(' ');
    QString op    = text.left(found);
    QString arg   = text.mid(found+1, text.size());

    ui->listWidget_3->item(index)->setText(op);
    ui->listWidget_4->item(index)->setText(arg);
}

void Debugger::addDisassemblyViewItems(std::vector<std::vector<QString>> disasm) {
    if (ui->listWidget->count()) {
        ui->listWidget->clear();
        ui->listWidget_2->clear();
        ui->listWidget_3->clear();
        ui->listWidget_4->clear();
    }

    for (auto& item : disassembly) {
        for (size_t i = 0; i < 2; i++)
            item[i] = QString("%1").arg(item[i].toInt(), 4, 16, QChar{'0'}).toUpper();

        ui->listWidget->addItem(item[0]);
        ui->listWidget_2->addItem(item[1]);
        ui->listWidget_3->addItem(item[2]);

        if (!item[3].isEmpty())
            ui->listWidget_4->addItem(item[3]);
        if (item[3].isEmpty())
            ui->listWidget_4->addItem("");
    }
}

void Debugger::addRegisterViewItems() {
//    using namespace std::chrono;
//    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    for (int i = 0; i < 16; i++) {
        unsigned char v = chip8->V[i];
        if (!regsLoaded)
            ui->leftRegisterListWidget->addItem(QString("V%1= ").arg(i, 0, 16).toUpper() + QString("%1").arg(v, 2, 16, QChar{'0'}).toUpper());
        else {
            if (v != oldV[i]) {
                ui->leftRegisterListWidget->item(i)->setText(QString("V%1= ").arg(i, 0, 16).toUpper() + QString("%1").arg(v, 2, 16, QChar{'0'}).toUpper());
                oldV[i] = v;
            }
        }
    }

    if (!regsLoaded) {
        ui->rightRegisterListWidget->addItem(QString("I=  ") + QString("%1").arg(chip8->I,  4, 16, QChar{'0'}).toUpper());
        ui->rightRegisterListWidget->addItem(QString("SP= ") + QString("%1").arg(chip8->sp, 2, 16, QChar{'0'}).toUpper());
        ui->rightRegisterListWidget->addItem(QString("PC= ") + QString("%1").arg(chip8->pc, 4, 16, QChar{'0'}).toUpper());
        ui->rightRegisterListWidget->addItem(QString("DT= ") + QString("%1").arg(chip8->delay_timer, 2, 16, QChar{'0'}).toUpper());
        ui->rightRegisterListWidget->addItem(QString("ST= ") + QString("%1").arg(chip8->sound_timer, 2, 16, QChar{'0'}).toUpper());
    }
    else {
        if (chip8->I != iOld) {
            ui->rightRegisterListWidget->item(0)->setText(QString("I=  ") + QString("%1").arg(chip8->I,  4, 16, QChar{'0'}).toUpper());
            iOld = chip8->I;
        }
        if (chip8->sp != spOld) {
            ui->rightRegisterListWidget->item(1)->setText(QString("SP= ") + QString("%1").arg(chip8->sp,  2, 16, QChar{'0'}).toUpper());
            spOld = chip8->sp;
        }
        if (chip8->pc != pcOld) {
            ui->rightRegisterListWidget->item(2)->setText(QString("PC= ") + QString("%1").arg(chip8->pc,  4, 16, QChar{'0'}).toUpper());
            pcOld = chip8->pc;
        }
        if (chip8->delay_timer != dtOld) {
            ui->rightRegisterListWidget->item(3)->setText(QString("DT= ") + QString("%1").arg(chip8->delay_timer,  2, 16, QChar{'0'}).toUpper());
            dtOld = chip8->delay_timer;
        }
        if (chip8->sound_timer != stOld) {
            ui->rightRegisterListWidget->item(4)->setText(QString("ST= ") + QString("%1").arg(chip8->sound_timer,  2, 16, QChar{'0'}).toUpper());
            stOld = chip8->sound_timer;
        }
    }

    if (!regsLoaded) {
        std::copy(chip8->V.begin(), chip8->V.end(), back_inserter(oldV));
        iOld = chip8->I;
        spOld = chip8->sp;
        pcOld = chip8->pc;
        dtOld = chip8->delay_timer;
        stOld = chip8->sound_timer;
    }
    regsLoaded = true;

//    high_resolution_clock::time_point t2 = high_resolution_clock::now();

//    duration<double, std::milli> time_span = t2 - t1;

//    total += time_span.count();
//    if (total)
//        qDebug() << "Total till now: " << total << " milliseconds.";
}

void Debugger::addStackViewItems() {

    for (int i = 0; i < 16; i++) {
        unsigned short s = chip8->stack[i];
        if (!stackLoaded)
            ui->stackListWidget->addItem(QString("%1  ").arg(i, 2, 16, QChar{'0'}).toUpper() + QString("%1").arg(s, 4, 16, QChar{'0'}).toUpper());
        else {
            if (s != oldStack[i]) {
                ui->stackListWidget->item(i)->setText(QString("%1  ").arg(i, 2, 16, QChar{'0'}).toUpper() + QString("%1").arg(s, 4, 16, QChar{'0'}).toUpper());
                oldStack[i] = s;
            }
        }
    }

    if (!stackLoaded)
        std::copy(chip8->stack.begin(), chip8->stack.end(), back_inserter(oldStack));
    stackLoaded = true;
}

Debugger* Debugger::getDebugContext() {
    return dbgctx;
}

//void Debugger::ActivitiesItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
//    if(option.state & QStyle::State_MouseOver){
//        painter->fillRect(option.rect, QColor(Qt::white));
//        painter->drawText(option.rect.adjusted(3,1,0,0), text);
//        return;
//    }
//}

////default
//QStyledItemDelegate::paint(painter, option, index);
//}


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

void Debugger::on_listWidget_3_itemDoubleClicked(QListWidgetItem *item) {
    Assembler c8asm;
    std::string instruction;

    int index = ui->listWidget_3->row(item);

    bool ok;
    QString text = QInputDialog::getText(this, tr("Edit instruction"),
                                         tr("Enter instruction"), QLineEdit::Normal,
                                         "", &ok);

    if (ok && !text.isEmpty()) {
        QString toAssembler = text + "\n";
        instruction = toAssembler.toUtf8().constData();

        uint16_t opcode = c8asm.compile(instruction);
        unsigned short addr = index * 2;
        chip8->memory[addr]   = (opcode >> 8);
        chip8->memory[addr+1] = (opcode & 0xFF);

        updateDisassemblyViewItem(index, text);
    }
}

void Debugger::on_listWidget_4_itemDoubleClicked(QListWidgetItem *item) {
    Assembler c8asm;
    std::string instruction;

    int index = ui->listWidget_4->row(item);

    bool ok;
    QString text = QInputDialog::getText(this, tr("Edit instruction"),
                                         tr("Enter instruction"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        QString toAssembler = text + "\n";
        instruction = toAssembler.toUtf8().constData();

        uint16_t opcode = c8asm.compile(instruction);
        unsigned short addr = index * 2;
        chip8->memory[addr]   = (opcode >> 8);
        chip8->memory[addr+1] = (opcode & 0xFF);

        updateDisassemblyViewItem(index, text);
    }
}

void Debugger::on_leftRegisterListWidget_itemDoubleClicked(QListWidgetItem *item) {
    int index = ui->leftRegisterListWidget->row(item);

    bool ok;
    QString text = QInputDialog::getText(this, tr("Edit register"),
                                         tr("Enter hex ('0x' optional)"), QLineEdit::Normal,
                                         "", &ok);

    if (ok && !text.isEmpty()) {
        unsigned char byte = std::stoul(text.toUtf8().constData(), nullptr, 16);
        text = QString("V%1= ").arg(index, 0, 16).toUpper() + QString("%1").arg(byte, 2, 16, QChar{'0'}).toUpper();

        chip8->V[index] = byte;
        ui->leftRegisterListWidget->takeItem(index);
        ui->leftRegisterListWidget->insertItem(index, text);
    }
}

void Debugger::on_rightRegisterListWidget_itemDoubleClicked(QListWidgetItem *item) {
    int index = ui->rightRegisterListWidget->row(item);

    bool ok;
    QString text = QInputDialog::getText(this, tr("Edit register"),
                                         tr("Enter hex ('0x' optional)"), QLineEdit::Normal,
                                         "", &ok);

    if (ok && !text.isEmpty()) {
        unsigned short byte = std::stoul(text.toUtf8().constData(), nullptr, 16);

        switch (index) {
            case 0:
                chip8->I = byte;
                ui->rightRegisterListWidget->takeItem(index);
                ui->rightRegisterListWidget->insertItem(index, QString("I=  ") + QString("%1").arg(byte, 4, 16, QChar{'0'}).toUpper());
                break;
            case 1:
                if (byte > 0xFF)
                    byte = 0x0;
                chip8->sp = byte;
                ui->rightRegisterListWidget->takeItem(index);
                ui->rightRegisterListWidget->insertItem(index, QString("SP= ") + QString("%1").arg(byte, 2, 16, QChar{'0'}).toUpper());
                break;
            case 2:
                chip8->pc = byte;
                ui->rightRegisterListWidget->takeItem(index);
                ui->rightRegisterListWidget->insertItem(index, QString("PC= ") + QString("%1").arg(byte, 4, 16, QChar{'0'}).toUpper());
                break;
            case 3:
                if (byte > 0xFF)
                    byte = 0x0;
                chip8->delay_timer = byte;
                ui->rightRegisterListWidget->takeItem(index);
                ui->rightRegisterListWidget->insertItem(index, QString("DT= ") + QString("%1").arg(byte, 2, 16, QChar{'0'}).toUpper());
                break;
            case 4:
                if (byte > 0xFF)
                    byte = 0x0;
                chip8->sound_timer = byte;
                ui->rightRegisterListWidget->takeItem(index);
                ui->rightRegisterListWidget->insertItem(index, QString("ST= ") + QString("%1").arg(byte, 2, 16, QChar{'0'}).toUpper());
                break;
        }
    }
}

void Debugger::on_stackListWidget_itemDoubleClicked(QListWidgetItem *item) {
    int index = ui->stackListWidget->row(item);

    bool ok;
    QString text = QInputDialog::getText(this, tr("Edit register"),
                                         tr("Enter hex ('0x' optional)"), QLineEdit::Normal,
                                         "", &ok);

    if (ok && !text.isEmpty()) {
        unsigned short byte = std::stoul(text.toUtf8().constData(), nullptr, 16);
        text = QString("%1= ").arg(index, 2, 16).toUpper() + QString("%1").arg(byte, 4, 16, QChar{'0'}).toUpper();

        chip8->stack[index] = byte;
        ui->stackListWidget->takeItem(index);
        ui->stackListWidget->insertItem(index, text);
    }
}

void Debugger::closeEvent(QCloseEvent *event) {
    QSettings settings("adalovegirls", "chip8-qt");
    settings.setValue("debug_geometry", saveGeometry());
}

void Debugger::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::ActivationChange) {
        if (this->isActiveWindow()) {
            sdl2->breakPoint();
            animate = false;
            if (chip8) {
                updateWidgets();
                updateCurrentLine();
            }
        }
    }
}

#include "debugger.h"
#include "ui_debugger.h"
#include "mainwindow.h"
#include "chip8.h"
#include "c8dasm/c8dasm.h"
#include "hexview/qhexview.h"
#include "hexview/document/buffer/qmemorybuffer.h"

//QByteArray rom;
std::vector<std::vector<QString>> disassembly;
C8Dasm c8dasm;

Debugger::Debugger(QWidget *parent) : QMainWindow(parent),  ui(new Ui::Debugger) {
    ui->setupUi(this);

    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
//    ui->splitter->setStretchFactor(0, 1);
//    ui->splitter_2->setStretchFactor(1, 1);
//    ui->splitter_3->setStretchFactor(0, 1);
    this->setStyleSheet("QWidget { border: 0; }");
}

Debugger::~Debugger()
{
    delete ui;
}

void Debugger::recRom(std::vector<unsigned char> array) {
    disassembly = c8dasm.Disassemble(array);

    for (unsigned int i = 0; i < disassembly.size(); i++) {
//        if (disassembly[i][1].compare("label"))
            addItem(disassembly[i]);
    }

//    QHexDocument* document = QHexDocument::fromMemory<QMemoryBuffer>(array);
//    QHexView* hexview = new QHexView();
//    hexview->setDocument(document);                  // Associate QHexEditData with this QHexEdit
//    this->setCentralWidget(hexview);
}

void Debugger::addItem(std::vector<QString> item) {
    auto label = item[1].compare("label");

    for (size_t i = 0; i < 2; i++) {
        QString s;
        if (label)
            s = QString("%1").arg(item[i].toInt(), 4, 16, QChar{'0'}).toUpper();
        else
            s = item[i];
        item[i] = s;
    }

    if (!label)
        ui->listWidget->addItem(item[2] + ": (" + item[0] + ')');
    else
        ui->listWidget->addItem(item[0]);

    ui->listWidget_2->addItem(item[1]);
    ui->listWidget_3->addItem(item[2]);
    ui->listWidget_4->addItem(item[3]);
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

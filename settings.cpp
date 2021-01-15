#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) : QWidget(parent), ui(new Ui::Settings) {
    ui->setupUi(this);

    QSettings settings("adalovegirls", "chip8-qt");
    QString language;
    language = settings.value("language").toString();

    if (!language.compare("chip8-qt_en"))
        ui->comboBox->setCurrentText("English");
    else if (!language.compare("chip8-qt_eu"))
        ui->comboBox->setCurrentText("Euskara");
}

Settings::~Settings() {
    delete ui;    
}

void Settings::on_comboBox_currentTextChanged(const QString &arg1) {
    QSettings settings("adalovegirls", "chip8-qt");
    if (!arg1.compare("English"))
        settings.setValue("language", "chip8-qt_en");
    else if (!arg1.compare("Euskara"))
        settings.setValue("language", "chip8-qt_eu");
}

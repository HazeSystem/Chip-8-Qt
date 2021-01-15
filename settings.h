#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QWidget>

namespace Ui {
class Settings;
}

class Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();

private slots:
    void on_comboBox_currentTextChanged(const QString &arg1);

private:
    Ui::Settings *ui;
    QSettings settings;
};

#endif // SETTINGS_H

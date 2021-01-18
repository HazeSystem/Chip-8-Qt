#include "mainwindow.h"

#undef main
int main(int argc,char* argv[])
{
    QApplication app(argc,argv);    
    QTranslator translator;
    QSettings settings("adalovegirls", "chip8-qt");
    QString language = settings.value("language").toString();

    translator.load("translations/" + language);
    app.installTranslator(&translator);

    app.setStyleSheet("QListWidget::item:hover {background: transparent;}");
    app.setStyleSheet("QListWidget::item:disabled:hover {background: transparent;}");
    app.setStyleSheet("QListWidget::item:hover:!active {background: transparent;}");

    MainWindow window;
    window.show();

	return app.exec();
}

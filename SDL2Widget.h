#ifndef SDL2WIDGET_H
#define SDL2WIDGET_H

#include "chip8.h"
#include "SDL.h"

#include <QDebug>
#include <QWidget>
#include <QTimer>
#include <vector>

class SDL2Widget : public QWidget
{
	Q_OBJECT    
public:
    SDL2Widget(QWidget* parent = nullptr);
	~SDL2Widget();
    static void loadRom(std::vector<unsigned char> rom);
    static void recKey(int key, bool state);
    static Chip8* getC8Context();
    static SDL2Widget* getSDLContext();
    void run();
    void breakPoint();

    bool running;

private:
    QTimer* timer;

private slots:
    void mainLoop();
};
#endif

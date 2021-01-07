#ifndef SDL2WIDGET_H
#define SDL2WIDGET_H

#include "chip8.h"

#include <SDL.h>
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
    static Chip8* getContext();

private:
    QTimer* timer;
    int position;
    int dir;

private slots:
    void mainLoop();
};


#endif

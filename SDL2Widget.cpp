#include "SDL2Widget.h"
#include <QPointer>
#include <chrono>

SDL_AudioSpec wavSpec;
Uint32 wavLength;
Uint8* wavBuffer;
SDL_AudioDeviceID deviceId;

SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Event e;

const int SCREEN_FPS = 240;

Chip8 c8;
SDL2Widget *sw;
Debugger *debug;
bool loaded = false;
bool animate = false;

SDL2Widget::SDL2Widget(QWidget* parent) : QWidget(parent) {
    sw = this;
    // Turn off double buffering for this widget. Double buffering
    // interferes with the ability for SDL to be properly displayed
    // on the QWidget.
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);

    /* setting setUpdatesEnabled to false is MOST important here!
     * if you set it to true or comment it out entirely then when you
     * resize the SDL2Widget by dragging it from corners you will
     * notice the widget will flicker with white frames which are
     * produced by the widget updates.
     */
    setUpdatesEnabled(false);

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        qDebug() << "Failed to initialize SDL video: " << SDL_GetError();
    sdlWindow = SDL_CreateWindowFrom((void*)this->winId());
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (SDL_Init(SDL_INIT_AUDIO) != 0)
        qDebug() << "Failed to initialize SDL audio: " << SDL_GetError();

    if (!SDL_LoadWAV("E3.wav", &wavSpec, &wavBuffer, &wavLength))
        qDebug() << "error loading audio";
    deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(mainLoop()));
//    timer->start(1000 / SCREEN_FPS);
}

SDL2Widget::~SDL2Widget() {
    SDL_CloseAudioDevice(deviceId);
    SDL_FreeWAV(wavBuffer);
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(sdlWindow);
    delete timer;

    sdlRenderer = nullptr;
    sdlWindow = nullptr;
    timer = nullptr;
}

void drawPixel(unsigned char p, short x, short y) {
    SDL_Rect fillRect = { x, y, 8, 8 };
    SDL_SetRenderDrawColor(sdlRenderer, p ? 0xFF : 0x00, \
        p ? 0xFF : 0x00, \
        p ? 0xFF : 0x00, 0xFF);
    SDL_RenderFillRect(sdlRenderer, &fillRect);
}

void drawGraphics() {
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 64; ++x) {
            drawPixel(c8.gfx[x + (64 * y)], x * 8, y * 8);
        }
    }
    SDL_RenderPresent(sdlRenderer);
    c8.draw = false;
}

void keydown(int k) {
    switch (k) {
    case SDLK_1:
        c8.key[0x01] = 1;
        break;

    case SDLK_2:
        c8.key[0x02] = 1;
        break;

    case SDLK_3:
        c8.key[0x03] = 1;
        break;

    case SDLK_4:
        c8.key[0x0C] = 1;
        break;

    case SDLK_q - 0x20:
        c8.key[0x04] = 1;
        break;

    case SDLK_w - 0x20:
        c8.key[0x05] = 1;
        break;

    case SDLK_e - 0x20:
        c8.key[0x06] = 1;
        break;

    case SDLK_r - 0x20:
        c8.key[0x0D] = 1;
        break;

    case SDLK_a - 0x20:
        c8.key[0x07] = 1;
        break;

    case SDLK_s - 0x20:
        c8.key[0x08] = 1;
        break;

    case SDLK_d - 0x20:
        c8.key[0x09] = 1;
        break;

    case SDLK_f - 0x20:
        c8.key[0x0E] = 1;
        break;

    case SDLK_z - 0x20:
        c8.key[0x0A] = 1;
        break;

    case SDLK_x - 0x20:
        c8.key[0x00] = 1;
        break;

    case SDLK_c - 0x20:
        c8.key[0x0B] = 1;
        break;

    case SDLK_v - 0x20:
        c8.key[0x0F] = 1;
        break;
    }
}

void keyup(int k) {
    switch (k) {
    case SDLK_1:
        c8.key[0x01] = 0;
        break;

    case SDLK_2:
        c8.key[0x02] = 0;
        break;

    case SDLK_3:
        c8.key[0x03] = 0;
        break;

    case SDLK_4:
        c8.key[0x0C] = 0;
        break;

    case SDLK_q - 0x20:
        c8.key[0x04] = 0;
        break;

    case SDLK_w - 0x20:
        c8.key[0x05] = 0;
        break;

    case SDLK_e - 0x20:
        c8.key[0x06] = 0;
        break;

    case SDLK_r - 0x20:
        c8.key[0x0D] = 0;
        break;

    case SDLK_a - 0x20:
        c8.key[0x07] = 0;
        break;

    case SDLK_s - 0x20:
        c8.key[0x08] = 0;
        break;

    case SDLK_d - 0x20:
        c8.key[0x09] = 0;
        break;

    case SDLK_f - 0x20:
        c8.key[0x0E] = 0;
        break;

    case SDLK_z - 0x20:
        c8.key[0x0A] = 0;
        break;

    case SDLK_x - 0x20:
        c8.key[0x00] = 0;
        break;

    case SDLK_c - 0x20:
        c8.key[0x0B] = 0;
        break;

    case SDLK_v - 0x20:
        c8.key[0x0F] = 0;
        break;
    }
}

void playBeep(SDL_AudioDeviceID deviceId) {
    if (!SDL_QueueAudio(deviceId, wavBuffer, wavLength)) {
        SDL_PauseAudioDevice(deviceId, 0);
        SDL_Delay(200);
    }
}

Chip8* SDL2Widget::getC8Context() {
    return &c8;
}

SDL2Widget* SDL2Widget::getSDLContext() {
    return sw;
}

void SDL2Widget::mainLoop() {
    debug = Debugger::getDebugContext();

    if (loaded)
        c8.emulateCycle();

    if (debug) {
        if (loaded && debug->loaded && animate) {
            debug->updateWidgets();
            debug->updateCurrentLine();
        }
    }

    if (loaded && c8.draw)
        drawGraphics();

    if (loaded && c8.beep) {
        playBeep(deviceId);
        c8.beep = false;
    }
}

void SDL2Widget::recKey(int k, bool state) {
    if (state)
        keydown(k);
    else
        keyup(k);
}

void SDL2Widget::loadRom(std::vector<unsigned char> rom) {
    c8.init(rom);
    loaded = true;
}

void SDL2Widget::run() {
    if (loaded) {
        animate = false;
        this->activateWindow();
        timer->start(1000 / SCREEN_FPS);
    }
}

void SDL2Widget::breakPoint() {
    if (loaded) {
        timer->stop();
    }
}

void SDL2Widget::singleStep() {
    if (loaded) {
        animate = true;
        mainLoop();
    }
}

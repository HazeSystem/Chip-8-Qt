#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <vector>

class Chip8 {
public:
	bool draw;
    bool beep;
    size_t rom_size;
    bool debug;

    void init(std::vector<unsigned char> rom);
    void emulateCycle();

	// Chip8    
    std::vector<unsigned char> memory;	// Memory (size = 4k)
    std::vector<unsigned char> gfx;	// Total amount of pixels: 2048
    std::vector<unsigned char> key;

    unsigned short pc;				// Program counter
    unsigned short I;				// Index register
    unsigned char  sp;				// Stack pointer

    std::vector<unsigned char>  V;			// V-regs (V0-VF)
    std::vector<unsigned short> stack;		// Stack (16 levels)

    unsigned char  delay_timer;		// Delay timer
    unsigned char  sound_timer;		// Sound timer

private:
    unsigned short opcode;			// Current opcode
};

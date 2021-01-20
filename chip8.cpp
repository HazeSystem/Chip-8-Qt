#include "chip8.h"

#include <iostream>
#include <QDebug>

std::default_random_engine generator;
std::uniform_int_distribution<int> distribution(0, 255);

std::vector<unsigned char> font {
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
	0xF0, 0x90, 0xF0, 0x90, 0x90, //A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
	0xF0, 0x80, 0x80, 0x80, 0xF0, //C
	0xE0, 0x90, 0x90, 0x90, 0xE0, //D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

void Chip8::init(std::vector<unsigned char> rom) {
    debug = false;

	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;
    rom_size = rom.size();

    if (!gfx.empty() && !stack.empty() && !V.empty() && !memory.empty() && !key.empty()) {
        gfx.clear();
        stack.clear();
        V.clear();
        memory.clear();
        key.clear();
    }

    for (int i = 0; i < 2048; i++)
        gfx.push_back(0);

	for (int i = 0; i < 16; i++)
        stack.push_back(0);

	for (int i = 0; i < 16; i++)
        V.push_back(0);

	for (int i = 0; i < 4096; i++)
        memory.push_back(0);

	for (int i = 0; i < 16; i++)
        key.push_back(0);

	for (int i = 0; i < 80; i++)
		memory[0x50 + i] = font[i];

    for (unsigned int i = 0; i < rom_size; i++)
        memory[i+0x200] = rom[i];

    delay_timer = 0;
	sound_timer = 0;
	draw = true;
}

void Chip8::emulateCycle() {
	opcode = memory[pc] << 8 | memory[pc + 1];
    unsigned char x = (opcode & 0x0F00) >> 8;
    unsigned char y = (opcode & 0x00F0) >> 4;
    unsigned char n = opcode & 0x000F;
    unsigned short nn = opcode & 0x00FF;
    unsigned short nnn = opcode & 0x0FFF;
    unsigned char pixel;
	bool keyPress = false;

    if (debug)
        qDebug() << Qt::hex << "$" << pc << " " << opcode << Qt::endl;

	switch (opcode & 0xF000) {
	case 0x0000:
        switch (nn) {
        case 0x00: // 0000: Does nothing for one cycle
            break;

        case 0xE0: // 00E0: Clears the screen.
            for (int i = 0; i < 2048; i++)
				gfx[i] = 0;
			draw = true;
			pc += 2;

            if (debug)
                qDebug() << "  " << "Screen cleared";
			break;

        case 0xEE: // 00EE: Returns from a subroutine.
            pc = stack[--sp];

            if (debug)
                qDebug() << "  " << "Returned to pc = " << Qt::hex << pc;
			break;
		}
		break;

	case 0x1000: // 1NNN: Jumps to address NNN
        pc = nnn;

        if (debug)
            qDebug() << "  " << "pc = " << Qt::hex << pc;
		break;

	case 0x2000: // 2NNN: Calls subroutine at NNN
        stack[sp++] = pc + 2;
        pc = nnn;

        if (debug)
            qDebug() << "  " << "Push " << Qt::hex << stack[sp - 1] << " and call " << pc;
		break;

	case 0x3000: // 3XNN: Skips the next instruction if VX equals NN
        pc += (V[x] == nn) ? 4 : 2;

//        if (debug)
//            qDebug() << "  " << "pc = "
		break;

	case 0x4000: // 4XNN: Skips the next instruction if VX doesn't equal NN
        pc += (V[x] != nn) ? 4 : 2;
		break;

	case 0x5000: // 5XY0: Skips the next instruction if VX equals VY
        pc += (V[x] == V[y]) ? 4 : 2;
		break;

	case 0x6000: // 6XNN: Sets VX to NN
        V[x] = nn;
		pc += 2;

        if (debug)
            qDebug() << "  " << "V" << Qt::hex << x << " = " << Qt::hex << V[x];
		break;

	case 0x7000: // 7XNN: Adds NN to VX. (Carry flag is not changed)
        if (debug)
            qDebug() << "  " << "V" << x << " = " << V[x] << " + " << nn << " = ";
        V[x] += nn;
		pc += 2;

        if (debug)
            qDebug() << "  " << V[x];
		break;

	case 0x8000:
        switch (n) {
        case 0x0: // 8XY0: Sets VX to the value of VY
            V[x] = V[y];
			pc += 2;
			break;

        case 0x1: // 8XY1: Sets VX to VX or VY
            V[x] = V[x] | V[y];
			pc += 2;
			break;

        case 0x2: // 8XY2: Sets VX to VX and VY
            V[x] = V[x] & V[y];
			pc += 2;
			break;

        case 0x3: // 8XY3: Sets VX to VX xor VY
            V[x] = V[x] ^ V[y];
			pc += 2;
			break;

        case 0x4: // 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
            V[0xF] = (V[x] + V[y]) > 0xFF ? 1 : 0;
            V[x] += V[y];
			pc += 2;
			break;

        case 0x5: // 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
            V[0xF] = (V[x] > V[y]) ? 1 : 0;
            V[x] -= V[y];
			pc += 2;
			break;

        case 0x6: // 8XZ6: Stores the least significant bit of VX in VF and then shifts VX to the right by 1
            V[0xF] = V[x] & 0x01;
            V[x] >>= 1;
			pc += 2;

			//if (debug)
            //	qDebug() << "  " + "Stored lsb of V" << Qt::hex << x + " = " + V[x] + " in VF as " + V[0xF]);
			break;

        case 0x7: // 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
            V[0xF] = (V[x] > V[y]) ? 0 : 1;
            V[x] = V[y] - V[x];
			pc += 2;
			break;

        case 0xE: // 8XZE: Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
            V[0xF] = V[x] >> 7;
            V[x] <<= 1;
			pc += 2;

			//if (debug)
            //	qDebug() << "  " + "Stored msb of V" << Qt::hex << x + " = " + V[x] + " in VF as " + V[0xF]);
			break;
		}
		break;

	case 0x9000: // 9XY0: Skips the next instruction if VX doesn't equal VY
        pc += (V[x] != V[y]) ? 4 : 2;
        break;

	case 0xA000: // ANNN: Sets I to the address NNN
        I = nnn;
		pc += 2;

		//if (debug)
        //	qDebug() << "  " + "I = " << Qt::hex << I);
		break;

	case 0xB000: // BNNN: Jumps to the address NNN plus V0
        pc = V[0x0] + nnn;
		break;

	case 0xC000: // CXNN: Sets VX to the result of a bitwise and operation on a random number and NN.
        V[x] = (nn) & distribution(generator);
		pc += 2;

		//if (debug)
        //	qDebug() << "  " + "V" + (x) + " = " + V[x]);
		break;

	case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N+1 pixels
		V[0xF] = 0;
        for (int h = 0; h < n; h++) {
			pixel = memory[I + h];

			for (int w = 0; w < 8; w++) {
				if ((pixel & (0x80 >> w)) != 0) {
                    if (gfx[(V[x] + w + ((V[y] + h) * 64))] == 1)
						V[0xF] = 1;
                    gfx[(V[x] + w + ((V[y] + h) * 64))] ^= 1;
				}
			}
		}
		draw = true;
		pc += 2;

		//if (debug)
        //	qDebug() << "  " + "Draw sprite at (" + x + "," + y + ")" + " with height " + height);
		break;

	case 0xE000:
        switch (nn) {
        case 0x9E: // EX9E: Skips the next instruction if the key stored in VX is pressed
            pc += (key[V[x]] == 1) ? 4 : 2;
            break;

        case 0xA1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
            pc += (key[V[x]] == 0) ? 4 : 2;
			break;
		}
		break;

	case 0xF000:
        switch (nn) {
        case 0x07: // FX07: Sets VX to the value of the delay timer
            V[x] = delay_timer;
			pc += 2;
			break;

        case 0x0A: // FX0A: A key press is awaited, and then stored in VX
			for (int i = 0; i < 16; ++i) {
				if (key[i] != 0) {
                    V[x] = i;
					keyPress = true;
				}
			}

			if (!keyPress)
				return;

			pc += 2;
			break;

        case 0x15: // FX15: Sets the delay timer to VX
            delay_timer = V[x];
			pc += 2;
			break;

        case 0x18: // FX18: Sets the sound timer to VX
            sound_timer = V[x];
			pc += 2;
			break;

        case 0x1E: // FX1E: Adds VX to I
            I += V[x];
			pc += 2;

			//if (debug)
            //	qDebug() << "  " + "Added V" << Qt::hex << x + " = " + V[x] + " to I: " << Qt::hex << I);
			break;

        case 0x29: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0x0-0xF are represented by a 4x5 font
            I = 0x50 + ((V[x]) * 5);
			pc += 2;

			//if (debug)
            //	qDebug() << "  " + "Set sprite addr I to " << Qt::hex << I);
			break;

        case 0x33: // FX33: Stores the binary-coded decimal representation of VX
            memory[I] = (V[x] / 100);
            memory[I + 1] = (V[x] / 10) % 10;
            memory[I + 2] = (V[x] % 100) % 10;
			pc += 2;
			break;

        case 0x55: // FX55: Stores V0 to VX in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified
            for (int i = 0; i < (x) + 1; ++i)
				memory[I + i] = V[i];

            I += (x) + 1;
			pc += 2;

			//if (debug)
            //	qDebug() << "  " + "Stored V0-V" << Qt::hex << x + ": " + V + " starting at I = " << Qt::hex << I);
			break;

        case 0x65: // FX65: Fills V0 to VX with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified
            for (int i = 0; i < (x) + 1; ++i)
				V[i] = memory[I + i];

            I += (x) + 1;
			pc += 2;

			//if (debug)
            //	qDebug() << "  " + "Filled V0-V" << Qt::hex << x + ": " + V + " starting at I = " << Qt::hex << I);
			break;
		}
		break;

	default:
        //qDebug() << "Unimplemented opcode: " << Qt::hex << opcode);
		printf("Unimplemented opcode: %x", opcode);
	}

	if (delay_timer > 0)
		delay_timer--;

	if (sound_timer > 0) {
		if (sound_timer == 1) {
			beep = true;
		}
		sound_timer--;
	}
}

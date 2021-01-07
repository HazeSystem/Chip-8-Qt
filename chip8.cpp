#include "chip8.h"

bool running;
std::default_random_engine generator;
std::uniform_int_distribution<int> distribution(0, 255);

// Maze.ch8
//unsigned char rom[34] = {
//    0xA2, 0x1E, 0xC2, 0x01, 0x32, 0x01, 0xA2, 0x1A, 0xD0, 0x14, 0x70, 0x04,
//    0x30, 0x40, 0x12, 0x00, 0x60, 0x00, 0x71, 0x04, 0x31, 0x20, 0x12, 0x00,
//    0x12, 0x18, 0x80, 0x40, 0x20, 0x10, 0x20, 0x40, 0x80, 0x10
//};

std::vector<unsigned char> font
{
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
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;
    rom_size = rom.size();

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
	unsigned short x = V[(opcode & 0x0F00) >> 8];
	unsigned short y = V[(opcode & 0x00F0) >> 4];
	unsigned short height = opcode & 0x000F;
	unsigned short pixel;
	bool keyPress = false;

	/*if (debug)
		console.log("$" + Number(pc).toString(16) + " " + Number(opcode).toString(16));*/

	switch (opcode & 0xF000) {
	case 0x0000:
		switch (opcode & 0x00FF) {
        case 0x0000: // 0000: Does nothing for one cycle
            break;

		case 0x00E0: // 00E0: Clears the screen.
			for (int i = 0; i < 2048; ++i)
				gfx[i] = 0;
			draw = true;
			pc += 2;

			/*if (debug)
				console.log("  " + "Screen cleared");*/
			break;

		case 0x00EE: // 00EE: Returns from a subroutine.
			--sp;
			pc = stack[sp];
			pc += 2;

			/*if (debug)
				console.log("  " + "Returned to pc = " + Number(pc).toString(16));*/
			break;
		}

		break;

	case 0x1000: // 1NNN: Jumps to address NNN
		pc = opcode & 0x0FFF;

		/*if (debug)
			console.log("  " + "pc = " + Number(pc).toString(16));*/
		break;

	case 0x2000: // 2NNN: Calls subroutine at NNN
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;

		/*if (debug)
			console.log("  " + "Push " + Number(stack[sp - 1]).toString(16) + " and call " + Number(pc).toString(16));*/
		break;

	case 0x3000: // 3XNN: Skips the next instruction if VX equals NN
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
			pc += 4;
			//if (debug)
			//	console.log("  " + "skipped = " + true);
		}
		else {
			pc += 2;
			/*			if (debug)
							console.log("  " + "skipped = " + false); */
		}
		break;

	case 0x4000: // 4XNN: Skips the next instruction if VX doesn't equal NN
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
			pc += 4;
			/*if (debug)
				console.log("  " + "skipped = " + true);*/
		}
		else {
			pc += 2;
			/*if (debug)
				console.log("  " + "skipped = " + false);*/
		}
		break;

	case 0x5000: // 5XY0: Skips the next instruction if VX equals VY
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
			pc += 4;
			/*if (debug)
				console.log("  " + "skipped = " + true);*/
		}
		else {
			pc += 2;
			/*if (debug)
				console.log("  " + "skipped = " + false);*/
		}

		break;

	case 0x6000: // 6XNN: Sets VX to NN
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		pc += 2;

		/*if (debug)
			console.log("  " + "V" + Number((opcode & 0x0F00) >> 8).toString(16) + " = " + Number(V[(opcode & 0x0F00) >> 8]).toString(16));*/
		break;

	case 0x7000: // 7XNN: Adds NN to VX. (Carry flag is not changed)
	/*	if (debug)
			console.log("  " + "V" + ((opcode & 0x0F00) >> 8) + " = " + V[(opcode & 0x0F00) >> 8] + " + " + (opcode & 0x00FF) + " = ");*/
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		pc += 2;

		//if (debug)
		//	console.log("  " + V[((opcode & 0x0F00) >> 8)]);
		break;

	case 0x8000:
		switch (opcode & 0x000F) {
		case 0x0000: // 8XY0: Sets VX to the value of VY
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0001: // 8XY1: Sets VX to VX or VY
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0002: // 8XY2: Sets VX to VX and VY
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0003: // 8XY3: Sets VX to VX xor VY
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0004: // 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
			if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
				V[0xF] = 1; //carry
			else
				V[0xF] = 0;

			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0005: // 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
				V[0xF] = 0;
			else
				V[0xF] = 1;

			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0006: // 8XZ6: Stores the least significant bit of VX in VF and then shifts VX to the right by 1
			V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x01;
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;

			//if (debug)
			//	console.log("  " + "Stored lsb of V" + Number((opcode & 0x0F00) >> 8).toString(16) + " = " + V[(opcode & 0x0F00) >> 8] + " in VF as " + V[0xF]);
			break;

		case 0x0007: // 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
			if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
				V[0xF] = 0;
			else
				V[0xF] = 1;

			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x000E: // 8XZE: Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
			V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
			V[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;

			//if (debug)
			//	console.log("  " + "Stored msb of V" + Number((opcode & 0x0F00) >> 8).toString(16) + " = " + V[(opcode & 0x0F00) >> 8] + " in VF as " + V[0xF]);
			break;
		}

		break;

	case 0x9000: // 9XY0: Skips the next instruction if VX doesn't equal VY
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
			pc += 4;
			//if (debug)
			//	console.log("  " + "skipped = " + true);
		}
		else {
			pc += 2;
			//if (debug)
			//	console.log("  " + "skipped = " + false);
		}

		break;

	case 0xA000: // ANNN: Sets I to the address NNN
		I = opcode & 0x0FFF;
		pc += 2;

		//if (debug)
		//	console.log("  " + "I = " + Number(I).toString(16));
		break;

	case 0xB000: // BNNN: Jumps to the address NNN plus V0
		pc = V[0x0] + (opcode & 0x0FFF);
		break;

	case 0xC000: // CXNN: Sets VX to the result of a bitwise and operation on a random number and NN.
		V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF) & distribution(generator);
		pc += 2;

		//if (debug)
		//	console.log("  " + "V" + ((opcode & 0x0F00) >> 8) + " = " + V[(opcode & 0x0F00) >> 8]);
		break;

	case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N+1 pixels
		V[0xF] = 0;
		for (int h = 0; h < height; h++) {
			pixel = memory[I + h];

			for (int w = 0; w < 8; w++) {
				if ((pixel & (0x80 >> w)) != 0) {
					if (gfx[(x + w + ((y + h) * 64))] == 1)
						V[0xF] = 1;
					gfx[(x + w + ((y + h) * 64))] ^= 1;
				}
			}
		}

		draw = true;
		pc += 2;

		//if (debug)
		//	console.log("  " + "Draw sprite at (" + x + "," + y + ")" + " with height " + height);
		break;

	case 0xE000:
		switch (opcode & 0x00FF) {
		case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
			if (key[V[(opcode & 0x0F00) >> 8]] == 1)
				pc += 4;
			else
				pc += 2;
			break;

		case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
			if (key[V[(opcode & 0x0F00) >> 8]] == 0)
				pc += 4;
			else
				pc += 2;
			break;
		}

		break;

	case 0xF000:
		switch (opcode & 0x00FF) {
		case 0x0007: // FX07: Sets VX to the value of the delay timer
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

		case 0x000A: // FX0A: A key press is awaited, and then stored in VX 
			for (int i = 0; i < 16; ++i) {
				if (key[i] != 0) {
					V[(opcode & 0x0F00) >> 8] = i;
					keyPress = true;
				}
			}

			if (!keyPress)
				return;

			pc += 2;
			break;

		case 0x0015: // FX15: Sets the delay timer to VX
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0018: // FX18: Sets the sound timer to VX
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x001E: // FX1E: Adds VX to I
			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;

			//if (debug)
			//	console.log("  " + "Added V" + Number((opcode & 0x0F00) >> 8).toString(16) + " = " + V[(opcode & 0x0F00) >> 8] + " to I: " + Number(I).toString(16));
			break;

		case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0x0-0xF are represented by a 4x5 font
			I = 0x50 + ((V[(opcode & 0x0F00) >> 8]) * 5);
			pc += 2;

			//if (debug)
			//	console.log("  " + "Set sprite addr I to " + Number(I).toString(16));
			break;

		case 0x0033: // FX33: Stores the binary-coded decimal representation of VX
			memory[I] = (V[(opcode & 0x0F00) >> 8] / 100);
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
			pc += 2;
			break;

		case 0x0055: // FX55: Stores V0 to VX in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified
			for (int i = 0; i < ((opcode & 0x0F00) >> 8) + 1; ++i)
				memory[I + i] = V[i];

			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;

			//if (debug)
			//	console.log("  " + "Stored V0-V" + Number((opcode & 0x0F00) >> 8).toString(16) + ": " + V + " starting at I = " + Number(I).toString(16));
			break;

		case 0x0065: // FX65: Fills V0 to VX with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified
			for (int i = 0; i < ((opcode & 0x0F00) >> 8) + 1; ++i)
				V[i] = memory[I + i];

			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;

			//if (debug)
			//	console.log("  " + "Filled V0-V" + Number((opcode & 0x0F00) >> 8).toString(16) + ": " + V + " starting at I = " + Number(I).toString(16));
			break;
		}

		break;
	default:
		//console.log("Unimplemented opcode: " + Number(opcode).toString(16));
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

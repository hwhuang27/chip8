#include<iostream>
#include<vector>
#include<fstream>
#include<iomanip>
#include "chip8.h"

std::vector<uint8_t> fontset
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, 
	0x20, 0x60, 0x20, 0x20, 0x70,
	0xF0, 0x10, 0xF0, 0x80, 0xF0,
	0xF0, 0x10, 0xF0, 0x10, 0xF0,
	0x90, 0x90, 0xF0, 0x10, 0x10,
	0xF0, 0x80, 0xF0, 0x10, 0xF0,
	0xF0, 0x80, 0xF0, 0x90, 0xF0,
	0xF0, 0x10, 0x20, 0x40, 0x40,
	0xF0, 0x90, 0xF0, 0x90, 0xF0,
	0xF0, 0x90, 0xF0, 0x10, 0xF0,
	0xF0, 0x90, 0xF0, 0x90, 0x90,
	0xE0, 0x90, 0xE0, 0x90, 0xE0,
	0xF0, 0x80, 0x80, 0x80, 0xF0,
	0xE0, 0x90, 0x90, 0x90, 0xE0,
	0xF0, 0x80, 0xF0, 0x80, 0xF0,
	0xF0, 0x80, 0xF0, 0x80, 0x80 
};

Chip8::Chip8()
	: drawFlag{ false }
	, gfx(64 * 32)
	, key(16)
	, opcode{ 0 }
	, pc{ 0x200 }
	, sp{ 0 }
	, I{ 0 }
	, delay_timer{ 0 }
	, sound_timer{ 0 }
	, stack(16)
	, V(16)
	, memory(4096) 
{
	for (int i = 0; i < fontset.size(); i++) {
		memory[i] = fontset[i];		// load fontset into memory
	}

	std::cout << "Chip 8 initialized. \n" << "\n";
};

void Chip8::fetchOpcode() {
	opcode = memory[pc] << 8 | memory[pc + 1];
	pc += 2;
}

void Chip8::decodeAndExecute() {
	uint8_t left_nib = opcode >> 12;
	switch (left_nib) {
		case 0x0:
		{
			uint8_t second_byte = opcode & 0xff;
			switch (second_byte) 
			{
				case 0xe0:	// 00E0 - clear screen
				{
					std::fill(gfx.begin(), gfx.end(), 0);

				} break;
				case 0xee:	// 00EE - return
				{
					std::cout << "return\n";
				} break;
			} break;
		}
		case 0x1:	// 1NNN - JMP to address NNN
		{
			uint16_t addr = opcode & 0x3ff;
			pc = addr;
		} break;
		case 0x6:	// 6XNN - Store NN in register VX
		{
			uint8_t reg = (opcode >> 8) & 0xf;
			uint8_t byte = opcode & 0xff;
			V[reg] = byte;
			//std::cout << "Reg 0 before: " << std::hex << std::setfill('0') << std::setw(2) << (uint16_t)V[reg] << "\n";
			//std::cout << "Reg 0 after: " << std::hex << std::setfill('0') << std::setw(2) << (uint16_t)V[reg] << "\n";
		} break;
		case 0x7:	// 7XNN - Add NN to register VX
		{
			uint8_t reg = (opcode >> 8) & 0xf;
			uint8_t byte = opcode & 0xff;
			V[reg] += byte;
		} break;
		case 0xa:	// ANNN	- Set index register to NNN
		{
			uint16_t addr = opcode & 0x3ff;
			I = addr;
		} break;
		case 0xd:	// DXYN - Display/draw
		{
			uint16_t n = opcode & 0xf;
			uint16_t x = V[(opcode >> 8) & 0xf] % 64;
			uint16_t y = V[(opcode >> 4) & 0xf] % 32;
			V[0xF] = 0;

			uint16_t sprite_row{};
			uint16_t index{};
			for (int i = 0; i < n; i++) {
				sprite_row = memory[I + i];

				index = (y + i)* 64 + x;

				for (int j = 7; j >= 0 ; j--) {
					if (sprite_row % 2 == 1) {
						if (gfx[index + j]) V[0xF] = 1;		// set carry flag if both are on
						gfx[index + j] ^= sprite_row % 2;	// gfx index XOR sprite row bit
					};
					sprite_row >>= 1;
				};
			};
		} break;
	}	// end of main switch
}

void Chip8::emulateCycle() {
	fetchOpcode();		// Fetch Opcode (Increment PC here)
	decodeAndExecute();	// Decode + Execute Opcode
	
						// Update Timers
};

bool Chip8::loadApplication(const char* filename) {
	// Load bytes from application into memory
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	std::ifstream::pos_type filesize = file.tellg();
	std::vector<uint8_t> buffer(filesize);
	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(buffer.data()), filesize);

	for (int i = 0; i < filesize; i++) {
		memory[i + 512] = buffer[i];
	}

	return true;
};

void Chip8::testMemory() {
	//std::cout << std::hex << "\n";
	//for (int i = 512; i < memory.size()/6; i +=2) {
	//	std::cout << std::uppercase << std::setfill('0') << std::setw(4) << i;
	//	uint16_t opcode = memory[i] << 8 | memory[i + 1];
	//	std::cout << " 0x" << std::nouppercase << std::setfill('0') << std::setw(4) << opcode << "\n";
	//}

	for (int i = 0; i < gfx.size(); i++) {
		if (i % 64 == 0) {
			std::cout << "\n";
		}
		std::cout << (uint16_t) gfx[i];
	};

	std::cout << "\n";
}
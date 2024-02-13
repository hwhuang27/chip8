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
	// load fontset into memory
	for (int i = 0; i < fontset.size(); i++) {
		memory[i] = fontset[i];
	}

	std::cout << "Chip 8 initialized. \n" << "\n";
};

void Chip8::emulateCycle() {
	// Fetch Opcode (Increment PC here)
	// Decode Opcode
	// Execute Opcode

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
	std::cout << std::hex;
	for (int i = 0; i < memory.size()/6; i++) {
		std::cout << i << " 0x" << (uint16_t)memory[i] << "\n";
	}
}
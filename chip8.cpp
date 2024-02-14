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
		case 0x0:	// 0--- Instructions
		{
			uint8_t second_byte = opcode & 0xff;
			switch (second_byte)
			{
				case 0xe0:	// 00E0 - clear screen
				{
					for (int i = 0; i < gfx.size(); i++) {
						gfx[i] = 0;
					};
				} break;
				case 0xee:	// 00EE - return from subroutine
				{
					sp -= 1;
					uint16_t addr = stack[sp];
					pc = addr;
				} break;
			};
		} break;
		case 0x1:	// 1NNN - JMP to address NNN
		{
			uint16_t addr = opcode & 0x3ff;
			pc = addr;
		} break;
		case 0x2:	// 2NNN - Execute subroutine at address NNN
		{
			uint16_t addr = opcode & 0x3ff;
			stack[sp] = pc;
			sp += 1;
			pc = addr;
		} break;
		case 0x3:	// 3XNN - Skip next instruction if VX == NN
		{
			uint8_t x = (opcode >> 8) & 0xf;
			uint8_t byte = opcode & 0xff;
			if (V[x] == byte) {
				pc += 2;
			};
		} break;
		case 0x4:	// 4XNN - Skip next instruction if VX != NN
		{
			uint8_t x = (opcode >> 8) & 0xf;
			uint8_t byte = opcode & 0xff;
			if (V[x] != byte) {
				pc += 2;
			};
		} break;
		case 0x5: // 5XY0 - Skip next instruction if VX == VY
		{
			uint8_t x = (opcode >> 8) & 0xf;
			uint8_t y = (opcode >> 4) & 0xf;
			if (V[x] == V[y]) {
				pc += 2;
			};
		} break;
		case 0x6:	// 6XNN - Store NN in register VX
		{
			uint8_t x = (opcode >> 8) & 0xf;
			uint8_t byte = opcode & 0xff;
			V[x] = byte;
		} break;
		case 0x7:	// 7XNN - Add NN to register VX
		{
			uint8_t x = (opcode >> 8) & 0xf;
			uint8_t byte = opcode & 0xff;
			V[x] += byte;
		} break;
		case 0x8:	// 8--- Instructions
		{
			uint8_t right_nib = opcode & 0xf;
			uint8_t x = (opcode >> 8) & 0xf;
			uint8_t y = (opcode >> 4) & 0xf;
			switch (right_nib) {
				case 0x0: { V[x] = V[y]; } break;	// 8XY0 - VX <- VY
				case 0x1: { V[x] |= V[y]; } break;	// 8XY1 - VX = VX | VY
				case 0x2: { V[x] &= V[y]; } break;	// 8XY2 - VX = VX & VY
				case 0x3: { V[x] ^= V[y]; } break;	// 8XY3 - VX = VX ^ VY
				case 0x4:	// 8XY4	- Add & set carry (VF)
				{
					uint16_t result = V[x] + V[y];
					if ((result & 0xff00) == 1) {
						V[0xF] = 1;
					}
					else {
						V[0xF] = 0;
					};
					V[x] = result & 0xff;
				} break;
				case 0x5:	// 8XY5	- Sub & set carry (VF)
				{
					uint16_t result = V[x] - V[y];
					if (V[x] > V[y]) {
						V[0xF] = 1;
					}
					else {
						V[0xF] = 0;
					};
					V[x] = result & 0xff;
				} break;
				case 0x6:	// 8XY6 - Shift right
				{
					V[x] = V[y];
					V[0xF] = (V[x] & 0x1) == 1;
					V[x] >>= 1;
				} break;
				case 0x7:  // 8XY7 - Sub & set carry (VF)
				{
					uint16_t result = V[y] - V[x];
					if (V[y] > V[x]) {
						V[0xF] = 1;
					}
					else {
						V[0xF] = 0;
					};
					V[x] = result & 0xff;
				} break;
				case 0xE:  // 8XYE - Shift left
				{
					V[x] = V[y];
					V[0xF] = (V[x] & 0x80) == 1;
					V[x] <<= 1;
				} break;
			};	// end of 0x8 switch
		} break;
		case 0x9: // 9XY0 - Skip next instruction if VX != VY
		{
			uint8_t x = (opcode >> 8) & 0xf;
			uint8_t y = (opcode >> 4) & 0xf;
			if (V[x] != V[y]) {
				pc += 2;
			};
		} break;
		case 0xa:	// ANNN	- Set index register to NNN
		{
			uint16_t addr = opcode & 0x3ff;
			I = addr;
		} break;
		case 0xb:	// BNNN	- Jump to address NNN + V0
		{
			uint16_t addr = opcode & 0x3ff;
			pc = addr + V[0];
		} break;
		case 0xc:	// CXNN - VX <- random number AND NN
		{
			uint8_t x = (opcode >> 8) & 0xf;
			//uint8_t rand =  (std::rand() % 256) & (opcode & 0xff);
			uint8_t rand = ((uint8_t) (std::rand() % 256)) & ((uint8_t) (opcode & 0xff));
			V[x] = rand;
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
		case 0xe:	// E--- Instructions
		{
			std::cout << "0xE instruction\n";
		} break;
		case 0xf:	// F--- Instructions
		{
			uint8_t right_byte = opcode & 0xff;
			uint8_t x = (opcode >> 8) & 0xf;
			switch (right_byte) {
				case 0x33:	// FX33 - Binary-coded decimal conversion	
				{
					uint16_t value = V[x];
					memory[I + 2] = (uint8_t) value % 10;
					value = value / 10;					
					memory[I + 1] = (uint8_t) value % 10;
					value = value / 10;
					memory[I] = (uint8_t) value % 10;
				} break;
				case 0x55:	// FX55 - Store registers into memory
				{
					for (int i = 0; i < 16; i++) {
						memory[I + i] = V[i];
					};
				} break;
				case 0x65:	// FX65 - Load registers from memory
				{
					std::cout << "FX65 called\n";
				} break;
			};	// end of 0xf switch
		} break;
	};	// end of main switch
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

	//std::cout << "Reg 0 before: " << std::hex << std::setfill('0') << std::setw(2) << (uint16_t)V[reg] << "\n";
	//std::cout << "Reg 0 after: " << std::hex << std::setfill('0') << std::setw(2) << (uint16_t)V[reg] << "\n";

	for (int i = 0; i < gfx.size(); i++) {
		if (i % 64 == 0) {
			std::cout << "\n";
		}
		if (gfx[i]) {
			std::cout << "0";
		}
		else {
			std::cout << " ";
		}
	};
	std::cout << "\n";
}
#include "chip8.h"

std::vector<uint8_t> fontset
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8()
	: drawFlag{ false }
	, gfx{ 2048 }
	, keys{ 16 }
	, opcode{ 0 }
	, pc{ 0x200 }
	, sp{ 0 }
	, I{ 0 }
	, delay_timer{ 0 }
	, sound_timer{ 0 }
	, stack(32)
	, V(16)
	, memory(4096) 
{
	// load fontset into memory
	for (int i{ 0 }; i < fontset.size(); ++i) {
		memory[i] = fontset[i];
	};
};

void Chip8::fetchOpcode() {
	opcode = memory[pc] << 8 | memory[pc + 1];
	pc += 2;
}

void Chip8::decodeAndExecute() {
	uint8_t left_nib = (opcode >> 12) & 0xf;
	switch (left_nib) {
		case 0x0:	// 0--- Instructions
		{
			uint8_t second_byte = opcode & 0xff;
			switch (second_byte)
			{
				case 0xe0:	// 00E0 - clear screen
				{
					for (int i{ 0 }; i < 2048; ++i) {
						gfx[i] = 0;
					};

					//std::fill(gfx, gfx + 2048, 0);
				} break;
				case 0xee:	// 00EE - return from subroutine
				{
					sp -= 1;
					pc = stack[sp];
				} break;
				default:	// 0NNN - skip this instruction
					break;
			};
		} break;

		case 0x1:	// 1NNN - JMP to address NNN
		{
			uint16_t addr = opcode & 0xfff;
			pc = addr;
		} break;

		case 0x2:	// 2NNN - Execute subroutine at address NNN
		{
			uint16_t addr = opcode & 0xfff;
			stack[sp] = pc;
			sp += 1;
			pc = addr;
		} break;

		case 0x3:	// 3XNN - Skip next instruction if VX == NN
		{
			uint8_t x = (opcode >> 8) & 0xf;
			uint8_t byte = opcode & 0xff;
			if (V[x] == byte)
				pc += 2;
		} break;

		case 0x4:	// 4XNN - Skip next instruction if VX != NN
		{
			uint8_t x = (opcode >> 8) & 0xf;
			uint8_t byte = opcode & 0xff;
			if (V[x] != byte)
				pc += 2;
		} break;

		case 0x5:	// 5XY0 - Skip next instruction if VX == VY
		{
			uint8_t x = (opcode >> 8) & 0xf;
			uint8_t y = (opcode >> 4) & 0xf;
			if (V[x] == V[y])
				pc += 2;
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
			uint16_t result{};
			uint8_t temp{};	// for carry flag V[F]
			switch (right_nib) {
				case 0x0: { V[x] = V[y]; } break;	// 8XY0 - VX <- VY
				case 0x1: { V[x] |= V[y]; } break;	// 8XY1 - VX = VX | VY
				case 0x2: { V[x] &= V[y]; } break;	// 8XY2 - VX = VX & VY
				case 0x3: { V[x] ^= V[y]; } break;	// 8XY3 - VX = VX ^ VY
				case 0x4:	// 8XY4	- Add & set carry (VF)
				{
					result = V[x] + V[y];
					if (result > 0x00ff)
						temp = 1;
					else
						temp = 0;
					V[x] = result & 0xff;
					V[0xF] = temp;
				} break;
				case 0x5:	// 8XY5	- Sub & set carry (VF)
				{
					result = V[x] - V[y];
					if (V[x] >= V[y])
						temp = 1;
					else
						temp = 0;
					V[x] = result & 0xff;
					V[0xF] = temp;
					
				} break;
				case 0x6:	// 8XY6 - Shift right
				{
					V[x] = V[y];
					temp = (V[x] & 0x1) == 1;
					V[x] >>= 1;
					V[0xF] = temp;
				} break;
				case 0x7:  // 8XY7 - Sub & set carry (VF)
				{
					result = V[y] - V[x];
					if (V[y] >= V[x])
						temp = 1;
					else
						temp = 0;
					V[x] = result & 0xff;
					V[0xF] = temp;
				} break;
				case 0xE:  // 8XYE - Shift left
				{
					V[x] = V[y];
					temp = (V[x] >> 7) == 1;
					V[x] <<= 1;
					V[0xF] = temp;
				} break;
			};	// end of 0x8 switch
		} break;

		case 0x9:	// 9XY0 - Skip next instruction if VX != VY
		{
			uint8_t x = (opcode >> 8) & 0xf;
			uint8_t y = (opcode >> 4) & 0xf;
			if (V[x] != V[y])
				pc += 2;
		} break;

		case 0xa:	// ANNN	- Set index register to NNN
		{
			uint16_t addr = opcode & 0x0fff;
			I = addr;
		} break;

		case 0xb:	// BNNN	- Jump to address NNN + V0
		{
			uint16_t addr = opcode & 0xfff;
			pc = V[0] + addr;
		} break;

		case 0xc:	// CXNN - VX <- random number AND NN
		{
			uint8_t x = (opcode >> 8) & 0xf;
			uint8_t rand =  (std::rand() % 256) & (opcode & 0xff);
			//uint8_t rand = ((uint8_t) (std::rand() % 256)) & ((uint8_t) (opcode & 0xff));
			V[x] = rand;
		} break;

		case 0xd:	// DXYN - Display/draw
		{
			uint8_t x = V[(opcode >> 8) & 0xf] % 64;
			uint8_t y = V[(opcode >> 4) & 0xf] % 32;
			uint8_t n = opcode & 0xf;
			uint8_t pixel{};
			int index{};
			V[0xF] = 0;

			for (int i{ 0 }; i < n; ++i) {
				pixel = memory[I + i];

				for (int j{ 0 }; j < 8; ++j) {
					index = (64 * (y + i)) + x + j;					
					if (j > 0 && (index % 64 == 0)) { break; };	// stop drawing if on right edge

					if ((pixel & (0x80 >> j)) == (0x80 >> j)) {	// check if sprite bit is on
						if (gfx[index] == 0xFFFFFFFF) {
							V[0xF] = 1;					
						}
												
						gfx[index] ^= 0xFFFFFFFF;	
					};

				};
				if ((y + i) > 32) { break; };	// stop if on bottom of screen
			};
		} break;

		case 0xe:	// E--- Instructions
		{
			uint8_t right_byte = opcode & 0xff;
			uint8_t x = (opcode >> 8) & 0xf;
			switch (right_byte) {
				case 0x9e:	// EX9E - skip next opcode if key in VX is being pressed
				{
					if (keys[V[x]])
						pc += 2;
				}break;
				case 0xa1:	// EX9E - skip next opcode if key in VX is NOT being pressed
				{
					if (!(keys[V[x]]))
						pc += 2;
				}break;
			};
		} break;

		case 0xf:	// F--- Instructions
		{
			uint8_t right_byte = opcode & 0xff;
			uint8_t x = (opcode >> 8) & 0xf;
			switch (right_byte) {
				case 0x07:{ V[x] = delay_timer; } break;	// FX07 - store delay timer into VX
				case 0x0a:	// FX0A - wait for keypress and store into VX
				{
					if (keys[0])
						V[x] = 0;
					else if (keys[1])
						V[x] = 1;
					else if (keys[2])
						V[x] = 2;
					else if (keys[3])
						V[x] = 3;
					else if (keys[4])
						V[x] = 4;
					else if (keys[5])
						V[x] = 5;
					else if (keys[6])
						V[x] = 6;
					else if (keys[7])
						V[x] = 7;
					else if (keys[8])
						V[x] = 8;
					else if (keys[9])
						V[x] = 9;
					else if (keys[10])
						V[x] = 10;
					else if (keys[11])
						V[x] = 11;
					else if (keys[12])
						V[x] = 12;
					else if (keys[13])
						V[x] = 13;
					else if (keys[14])
						V[x] = 14;
					else if (keys[15])
						V[x] = 15;
					else
						pc -= 2;
				} break;
				case 0x15: { delay_timer = V[x]; } break;
				case 0x18: { sound_timer = V[x]; } break;
				case 0x1e: { I += V[x]; } break;
				case 0x29:	// FX29 - Font character
				{
					I = memory[V[x] * 5];
				} break;
				case 0x33:	// FX33 - Binary-coded decimal conversion	
				{
					uint16_t value = V[x];
					memory[I + 2] = value % 10;	
					value /= 10;				
					memory[I + 1] = value % 10;
					value /= 10;
					memory[I] = value % 10;
				} break;
				case 0x55:	// FX55 - Store registers V[0] to V[X] into memory
				{
					for (uint8_t i{ 0 }; i <= x; ++i) {		
						memory[I + i] = V[i];
					};
				} break;
				case 0x65:	// FX65 - Load registers V[0] to V[X] from memory
				{
					for (uint8_t i{ 0 }; i <= x; ++i) {
						V[i] = memory[I + i];
					};

				} break;
			};	// end of 0xf switch
		} break;
	};	// end of main switch
}

void Chip8::updateTimers() {
	if (delay_timer > 0)
		--delay_timer;
	
	// also emit a sound if non-zero
	if (sound_timer > 0)	
		--sound_timer;
}

void Chip8::emulateCycle() {
	fetchOpcode();		// Fetch Opcode (Increment PC here)
	decodeAndExecute();	// Decode + Execute Opcode
	updateTimers();		// Update Timers
};

bool Chip8::loadApplication(const char* filename) {
	// open file as binary stream and seek to the end
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open()) {
		// make a buffer with filesize
		std::streampos size = file.tellg();
		char* buffer = new char[size];

		// move file pointer to start
		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		// load ROM into memory starting at 0x200
		for (int i{ 0 }; i < size; ++i) {
			memory[0x200 + i] = buffer[i];
		}
		delete[] buffer;
	}
	else {
		return false;
	}

	return true;
};

void Chip8::testMemory() {
	// Print bytes of loaded ROM
	std::cout << std::hex;
	for (long i = 512; i < memory.size()/6; i +=2) {
		std::cout << std::uppercase << std::setfill('0') << std::setw(4) << i;
		uint16_t opcode = memory[i] << 8 | memory[i + 1];
		std::cout << " 0x" << std::nouppercase << std::setfill('0') << std::setw(4) << opcode << "\n";
	}
}

void Chip8::testDisplay() {
	// Print display array to terminal
	for (int i{ 0 }; i < 2048; ++i) {
		if (i % 64 == 0)
			std::cout << "\n";

		if (gfx[i] == 1)
			std::cout << "8";
		else if (gfx[i] == 0)
			std::cout << "-";
	};
	std::cout << "\n\n";
}
#pragma once
#include<vector>
#include<iostream>
#include<fstream>
#include<iomanip>

class Chip8 {
public:
	Chip8();
	void emulateCycle();
	bool loadApplication(const char* filename);
	void testMemory();
	void testDisplay();

	bool drawFlag{};			// update screen
	uint32_t gfx[2048]{};		// graphics array 64 * 32 = [2048]
	uint8_t keys[16]{};			// keypress array [16]

private:
	void fetchOpcode();
	void decodeAndExecute();
	void updateTimers();

	uint16_t opcode{};		// current opcode
	uint16_t pc{};			// program counter
	uint8_t sp{};			// stack pointer
	uint16_t I{};			// index register
	uint8_t delay_timer{};	// delay timer
	uint8_t sound_timer{};	// sound timer

	std::vector<uint16_t> stack;	// call stack [16] 
	std::vector<uint8_t> V;			// registers V0 through VF [16]
	std::vector<uint8_t> memory;	// memory buffer [4096]
};
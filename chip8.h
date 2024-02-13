#pragma once
#include<vector>

class Chip8 {
public:
	Chip8();
	void emulateCycle();
	bool loadApplication(const char* filename);
	void testMemory();

	bool drawFlag{};			// update screen
	std::vector<uint8_t> gfx;	// graphics array (64 * 32) = (2048)
	std::vector<uint8_t> key;	// keypress array (16)

private:
	uint16_t opcode{};		// current opcode
	uint16_t pc{};			// program counter
	uint16_t sp{};			// stack pointer
	uint16_t I{};			// index register
	uint8_t delay_timer{};	// delay timer
	uint8_t sound_timer{};	// sound timer

	std::vector<uint16_t> stack;	// call stack (16) 
	std::vector<uint8_t> V;			// registers V0 through VF (16)
	std::vector<uint8_t> memory;	// memory buffer (4096)
};
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <string_view>
#include "chip8.h"

int main(int argc, char* argv[])
{
	Chip8 chip8{};
	
	if (!chip8.loadApplication(argv[1])) {
		std::cout << "Unable to load file\n";
		return 1;
	}

	chip8.testMemory();

	return 0;
}

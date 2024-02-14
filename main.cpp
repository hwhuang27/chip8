#include <iostream>
#include <SDL.h>
#include "chip8.h"

int main(int argc, char* argv[])
{
	Chip8 chip8{};
	
	if (!chip8.loadApplication(argv[1])) {
		std::cout << "Unable to load file\n";
		return 1;
	}
	int done{ 20 };
	while (done > 0) {
		chip8.emulateCycle();
		done--;
	}
	chip8.testMemory();

	return 0;
}

#include <iostream>
//#include <SDL.h>
#include "chip8.h"

int main(int argc, char* argv[])
{
	Chip8 chip8{};
	
	if (!chip8.loadApplication(argv[1])) {
		std::cout << "Unable to load file\n";
		return 1;
	}
	int done{ 600 };
	while (done > 0) {
		if (done % 200 == 0) {
			chip8.testMemory();
		}
		chip8.emulateCycle();
		done--;
	}
	
	return 0;
}

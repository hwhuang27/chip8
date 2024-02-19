#include <string>
#include <chrono>
#include "chip8.h"
#include "platform.h"

int main(int argc, char* argv[])
{
	if (argc < 4) {
		std::cerr << "Usage: " << argv[0] << " <ROM> <Scale> <Delay>\n";
		std::exit(EXIT_FAILURE);
	}

	int displayScale{ std::stoi(argv[1]) };
	int cycleDelay{ std::stoi(argv[2]) };
	const char* romFilepath{ argv[3] };

	int defaultWidth = 64;
	int defaultHeight = 32;
	
	Platform platform(
		"CHIP-8 EMULATOR",
		defaultWidth * displayScale,
		defaultHeight * displayScale,
		defaultWidth,
		defaultHeight);

	Chip8 chip8{};	
	if (!chip8.loadApplication(romFilepath)) {
		std::cerr << "Unable to load ROM\n";
		std::exit(EXIT_FAILURE);
	}
	
	// pitch: size of pixel in bytes * num pixels per row
	int pitch = sizeof(chip8.gfx[0]) * defaultWidth;
	auto prevCycleTime = std::chrono::steady_clock::now();
	bool quit = false;

	while (!quit) {
		quit = platform.handleInput(chip8.keys);
		auto currentTime = std::chrono::steady_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - prevCycleTime).count();

		if (dt > cycleDelay) {
			prevCycleTime = currentTime;
			chip8.emulateCycle();
			platform.update(chip8.gfx, pitch);
		};
	};
	
	return 0;
}

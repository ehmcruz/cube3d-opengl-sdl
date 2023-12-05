#include <iostream>
#include <exception>

#include <cstdlib>

#include "graphics.h"

// -------------------------------------------

namespace App {

// -------------------------------------------

using namespace Graphics;

// -------------------------------------------

inline constexpr Color config_background_color = {
	.r = 0.0f,
	.g = 0.0f,
	.b = 0.0f,
	.a = 1.0f
};

void main (const int argc, const char **argv)
{
}

// -------------------------------------------

}

// -------------------------------------------

int main (const int argc, const char **argv)
{
	try {
		App::main(argc, argv);
	}
	catch (const std::exception& e) {
		std::cout << "Exception happenned!" << std::endl << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...) {
		std::cout << "Unknown exception happenned!" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

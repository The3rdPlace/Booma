#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "booma.h"

int main(int argc, char** argv) {

	// Initialize Booma
	BoomaApplication app(argc, argv, true);

    // Run initial receiver (if any configured)
    app.Run();

    // Read commands
    std::string cmd;
    do {
        std::cout << "Booma# ";
        std::cin >> cmd;

        std::cout << "read " << cmd << std::endl;
    }
    while( cmd != "q" && cmd != "quit" );

    // Stop a running receiver (if any)
    app.Halt();

    // Leave peacefully
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "main.h"
#include "booma.h"

int main(int argc, char** argv) {

	// Initialize Booma
	std::stringstream ss;
	ss << "version " << BOOMAREMOTE_MAJORVERSION << "." << BOOMAREMOTE_MINORVERSION << "." << BOOMAREMOTE_BUILDNO;
	BoomaApplication app("Booma-Remote", ss.str(), argc, argv, false);

	// Run initial receiver (if any configured)
	app.Run();
	std::cout << "booma-remote " << ss.str() << " running. Press ctrl+c to quit" << std::endl;
	app.Wait();

	// Leave peacefully
	return 0;
}

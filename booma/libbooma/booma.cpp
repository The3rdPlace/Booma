#include <stdlib.h>
#include <iostream>

#include <hardtapi.h>

void BoomaInit(bool verbose) {

    // Initialize the Hardt toolkit.
    // Set the last argument to 'true' to enable verbose output instead of logging to a local file
    HInit(std::string("Booma"), verbose);

    // Show library name and and Hardt version.
    std::cout << "booma: using Hardt " + getversion() << std::endl;

}

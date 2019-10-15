#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "booma.h"

int main(int argc, char** argv) {
	BoomaInit(argc, argv, true);

    BoomaRun();

	return 0;
}

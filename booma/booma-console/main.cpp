#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "booma.h"

int main(int argc, char** argv) {
	ConfigOptions* opts = BoomaInit(argc, argv, true);

    BoomaRun(opts);

    int ch;
    while( (ch = getchar()) ) {
        printf("Read %c\n", (char) ch);
        if( (char) ch == 'q' ) {
            printf("exiting\n");
            BoomaHaltReceiver();
            break;
        }
    }
	return 0;
}

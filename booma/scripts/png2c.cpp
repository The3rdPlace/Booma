#include <stdio.h>
#include <stdlib.>

FILE* file = fopen("/home/henrik/Git/Booma/site/booma.png", "rb");
FILE* of = fopen("/home/henrik/Git/Booma/booma/booma-gui/include/splash.h", "wb");
int j = 0;
fprintf(of, "static const unsigned char Splash[] = {\n    ");
while( !feof(file) ) {
    unsigned char in[1];
    fread(in, 1, 1, file);
    fprintf(of, "0x%02X,", in[0]);
    buf[i++] = in[0];
    if( ++j > 50 ) {
        fprintf(of, "\n    ");
        j = 0;
    }
}
fprintf(of, "\n}\n");
fclose(file);
fclose(of);
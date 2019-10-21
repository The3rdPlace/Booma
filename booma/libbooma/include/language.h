#ifndef __LANGUAGE_H
#define __LANGUAGE_H

#include <stdlib.h>
#include <iostream>

/** Hook for when we later needs to add translation */
inline std::string tr(std::string source) {
    return source;
}

#endif

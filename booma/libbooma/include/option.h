#ifndef __BOOMA_OPTION_H
#define __BOOMA_OPTION_H

#include <stdio.h>
#include <string>
#include <vector>

struct OptionValue {
    
    std::string Name;
    std::string Description;
    int Value;
};

struct Option {
    
    std::string Name;
    std::string Description;

    std::vector<OptionValue> Values;

    int CurrentValue;
};

#endif
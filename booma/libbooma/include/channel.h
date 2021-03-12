#ifndef __CHANNEL_H
#define __CHANNEL_H

#include <string>
#include <sstream>
#include <iostream>
#include "boomaconfigurationexception.h"

class Channel {

    public:

        Channel(std::string definition) {
            std::istringstream stream(definition);
            std::string storedName;
            std::string frequency;
            if( !std::getline(stream, storedName, ':') ) {
                throw new BoomaConfigurationException("No name in channel definition");
            }
            std::replace(storedName.begin(), storedName.end(), ' ', '_');
            Name = storedName;
            if( !std::getline(stream, frequency, ':') ) {
                std::cout << definition << std::endl;
                throw new BoomaConfigurationException("No frequency in channel definition");
            }
            Frequency = std::stol(frequency);
        }

        Channel(std::string name, long int frequency) {
            if( name.find(',') != std::string::npos ) {
                throw new BoomaConfigurationException("Invalid character in name ','");
            }
            if( name.find(':') != std::string::npos ) {
                throw new BoomaConfigurationException("Invalid character in name ':'");
            }
            Name = name;
            Frequency = frequency;
        }

        std::string Name;
        long int Frequency;

        std::string GetDefinition() {
            std::string storedName = Name;
            std::replace(storedName.begin(), storedName.end(), ' ', '_');
            return storedName  + ":" + std::to_string(Frequency);
        }

        static struct ChannelComparator {
            bool operator() (Channel* i, Channel* j) { return (i->Frequency < j->Frequency); }
        } comparator;
};

#endif

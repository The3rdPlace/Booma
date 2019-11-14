#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>

#include "booma.h"
#include "config.h"
#include "language.h"

void ConfigOptions::PrintUsage() {
    std::cout << tr("Usage: booma-console [-option [parameter, ...]]") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Information]==") << std::endl;
    std::cout << tr("Show a list of audio devices             -c --cards") << std::endl;
    std::cout << tr("Show this help and exit                  -h --help") << std::endl;
    std::cout << tr("Show version and exit                    -v --version") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Options]==") << std::endl;
    std::cout << tr("Dump output audio as pcm to file         -a PCM") << std::endl;
    std::cout << tr("Dump output audio as wav to file         -a WAV") << std::endl;
    std::cout << tr("Select frequency (default 17.2KHz)       -f frequecy") << std::endl;
    std::cout << tr("Rf gain (default 30)                     -g gain") << std::endl;
    std::cout << tr("Select audio input source                -i AUDIO devicenumber ") << std::endl;
    std::cout << tr("Output volume (default 200)              -l volume") << std::endl;
    std::cout << tr("Select CW receive mode (default)         -m CW") << std::endl;
    std::cout << tr("Select output device                     -o devicenumber") << std::endl;
    std::cout << tr("Dump rf input as pcm to file             -p PCM") << std::endl;
    std::cout << tr("Dump rf input as wav to file (default)   -p WAV") << std::endl;
    std::cout << tr("Samplerate (default 48KHz)               -q rate") << std::endl;
    std::cout << tr("Receiver for remote input                -r address port") << std::endl;
    std::cout << tr("Server for remote input                  -s port") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Debugging]==") << std::endl;;
    std::cout << tr("Use sine generator as input              -i GENERATOR frequency") << std::endl;
    std::cout << tr("Use pcm file as input                    -i FILE filename") << std::endl;
    std::cout << tr("Use silence as input                     -i SILENCE") << std::endl;
    std::cout << tr("Select /dev/null as output device        -o -1") << std::endl;
}

void ConfigOptions::PrintCards() {

    if( HSoundcard::AvailableDevices() == 0 )
    {
        std::cout << "There is no soundcards available on this system" << std::endl; 
        return;
    }
    std::cout << "Default device is: " << HSoundcard::GetDefaultDevice() << std::endl;
    std::cout << std::endl; 
    std::vector<HSoundcard::DeviceInformation> info = HSoundcard::GetDeviceInformation(); 
    for( std::vector<HSoundcard::DeviceInformation>::iterator it = info.begin(); it != info.end(); it++)
    {
        std::cout << "Device:            " << (*it).Device << " = " << (*it).Name << std::endl;
        std::cout << "Inputs:              " << (*it).Inputs << std::endl;
        std::cout << "Outputs:             " << (*it).Outputs << std::endl;
        std::cout << "Is default device:   " << ((*it).IsDefaultDevice ? "yes" : "no") << std::endl;
        std::cout << std::endl;
    }
}

ConfigOptions::ConfigOptions(std::string appName, std::string appVersion, int argc, char** argv) {

    // First pass: help or version
    for( int i = 1; i < argc; i++ ) {

        // Show available audio cards
        if( strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--cards") == 0 ) {
            PrintCards();
            exit(0);
        }

        // Get helpt
        if( strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 ) {
            PrintUsage();
            exit(0);
        }

        // Get version information
        if( strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version" ) == 0 ) {
            std::cout << appName << " " << appVersion << " " << tr("using") << " Booma " << BOOMA_MAJORVERSION << "." << BOOMA_MINORVERSION << "." << BOOMA_BUILDNO << " " << tr("and") << " Hardt " << getversion() << std::endl;
            exit(0);
        }
    }

    // Seed configuration with values from last execution
    if( !ReadStoredConfig() && argc == 1 ) {
        std::cout << "No stored config in ~/.booma/config.ini and no arguments. Kindly presenting options" << std::endl << std::endl;
        PrintUsage();
        exit(1);
    }

    // Second pass: config options
    for( int i = 0; i < argc; i++ ) {

        // Dump output as ... to file
        if( strcmp(argv[i], "-a") == 0 && i < argc + 1) {
            _dumpAudio = true;
            if( strcmp(argv[i + 1], "PCM") == 0 ) {
                _dumpFileFormat = PCM;
            } else if( strcmp(argv[i + 1], "WAV") == 0 ) {
               _dumpFileFormat = WAV;
            }
            i++;
            continue;
        }

        // Frequency
        if( strcmp(argv[i], "-f") == 0 && i < argc + 1) {
            _frequency = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Rf gain
        if( strcmp(argv[i], "-g") == 0 && i < argc + 1) {
            _rfGain = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Select input device
        if( strcmp(argv[i], "-i") == 0 && i < argc + 2) {
            if( strcmp(argv[i + 1], "AUDIO") == 0 ) {
                _inputSourceType = AUDIO_DEVICE;
                _inputAudioDevice = atoi(argv[i + 2]);
                HLog("Input audio device %d", _inputAudioDevice);
            }
            else if( strcmp(argv[i + 1], "GENERATOR") == 0 ) {
                _inputSourceType = SIGNAL_GENERATOR;
                _signalGeneratorFrequency = atoi(argv[i + 2]);
                HLog("Input generator running at %d Hz", _signalGeneratorFrequency);
            }
            else if( strcmp(argv[i + 1], "FILE") == 0 ) {
                _inputSourceType = PCM_FILE;
                _pcmFile = argv[i + 2];
                HLog("Input file %s", _pcmFile.c_str());
            }
            else if( strcmp(argv[i + 1], "SILENCE") == 0 ) {
                _inputSourceType = SILENCE;
                HLog("input silence");
            }
            else
            {
                std::cout << tr("Unknown input source. Please use on of the types AUDIO|GENERATOR|FILE");
                exit(1);
            }

            _remoteServer = std::string();
            _remotePort = 0;
            _isRemoteHead = false;
            _useRemoteHead = false;

            i += 2;
            continue;
        }

        // Volume
        if( strcmp(argv[i], "-l") == 0 && i < argc + 1) {
            _volume = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Select receiver mode ...
        if( strcmp(argv[i], "-m") == 0 && i < argc + 1) {
            if( strcmp(argv[i + 1], "CW") == 0 ) {
                _receiverModeType = CW;
            }
            i++;
            continue;
        }

        // Select output device
        if( strcmp(argv[i], "-o") == 0 && i < argc + 1) {
            _outputAudioDevice = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Dump output audio as ...
        if( strcmp(argv[i], "-p") == 0) {
            _dumpRf = true;
            if( strcmp(argv[i + 1], "PCM") == 0 ) {
                _dumpFileFormat = PCM;
            } else if( strcmp(argv[i + 1], "WAV") == 0 ) {
               _dumpFileFormat = WAV;
            }
            i++;
            continue;
        }

        // Samplerate
        if( strcmp(argv[i], "-q") == 0 && argc < argc + 1) {
            _sampleRate = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Server for remote input
        if( strcmp(argv[i], "-s") == 0 && argc < argc + 1) {
            _remotePort = atoi(argv[i + 1]);
            _useRemoteHead = true;
            i++;
            continue;
        }

        // Receiver for remote input
        if( strcmp(argv[i], "-r") == 0 && argc < argc + 2) {
            _remoteServer = argv[i + 1];
            _remotePort = atoi(argv[i + 2]);
            _isRemoteHead = true;
            _inputAudioDevice = -1;
            i++;
            continue;
        }
    }

    // Check configuration for remote server/head
    if( _isRemoteHead && _remoteServer.empty() ) {
        std::cout << tr("Please select address of remote input with '-r address port'") << std::endl;
        exit(1);
    }
    if( _isRemoteHead && _remotePort == 0 ) {
        std::cout << tr("Please select port of remote input with '-r address port'") << std::endl;
        exit(1);
    }
    if( _useRemoteHead && _remotePort == 0 ) {
        std::cout << tr("Please select port for the remote input server with '-s port'") << std::endl;
        exit(1);
    }

    // Check that the required minimum of settings has been provided
    if( _useRemoteHead == false ) {
        if( _receiverModeType == NO_RECEIVE_MODE ) {
            std::cout << tr("Please select the receive mode with '-m [CW]'") << std::endl;
            exit(1);
        }
    }
    if( _isRemoteHead == false ) {
        if( _inputSourceType == NO_INPUT_TYPE ) {
            std::cout << tr("Please select the input type with '-i [AUDIO] devicenumber'") << std::endl;
            exit(1);
        }
        else if( _inputSourceType == AUDIO_DEVICE && _inputAudioDevice < 0 ) {
            std::cout << tr("Please select the input audio device with '-i [AUDIO] devicenumber'") << std::endl;
            exit(1);
        }
        else if( _inputSourceType == SIGNAL_GENERATOR && _signalGeneratorFrequency < 0 ) {
            std::cout << tr("Please select the input signal generator frequency with '-i GENERATOR frequency'") << std::endl;
            exit(1);
        }
        else if( _inputSourceType == PCM_FILE ) {

            // We need a filename
            if( _pcmFile.empty() ) {
                std::cout << tr("Please select the input filename with '-i FILE filename'") << std::endl;
                exit(1);
            }

            // Check if the input file exists
            struct stat stats;
            if( stat(_pcmFile.c_str(), &stats) != -1 ) {
                if( !S_ISREG(stats.st_mode) ) {
                    std::cout << "Input file does not exist" << std::endl;
                    exit(1);
                }
            }
        }
    }
}

ConfigOptions::~ConfigOptions() {
    SaveStoredConfig();
}

bool ConfigOptions::ReadStoredConfig() {

    // Get the users homedirectory
    const char* home = std::getenv("HOME");
    if( home == NULL ) {
        HError("No HOME env. variable. Unable to read configuration");
        return false;
    }

    // Compose the config path
    std::string configPath(home);
    configPath += "/.booma";

    // Check of the directory exists
    struct stat stats;
    if( stat(configPath.c_str(), &stats) != -1 ) {
        if( !S_ISDIR(stats.st_mode) ) {
            HError("File ~/.booma exists, but should be a directory");
            return false;
        }
        HLog("Config directory %s exists", configPath.c_str());
    }
    else
    {
        HLog("Config directory does not exists, no config to read");
        return false;
    }

    // Compose path to config file
    std::string configFile(configPath);
    configFile += "/config.ini";
    HLog("Config file is %s", configFile.c_str());

    // Open the config file
    std::ifstream configStream;
    configStream.open(configFile, std::ifstream::in);
    if( !configStream.is_open() ) {
        HError("Failed to open config file for reading");
        return false;
    }

    // Read all stored config settings
    std::string opt;
    configStream >> opt;
    while (configStream.good()) {

        // Split into name and value
        size_t splitAt = opt.find_first_of("=");
        if( splitAt != std::string::npos ) {
            std::string name = opt.substr(0, splitAt);
            std::string value = opt.substr(splitAt + 1, std::string::npos);

            HLog("config value (%s=%s)", name.c_str(), value.c_str());
            if( name == "sampleRate" )              _sampleRate = atoi(value.c_str());
            if( name == "outputAudioDevice" )       _outputAudioDevice = atoi(value.c_str());
            if( name == "inputSourceType" )         _inputSourceType = (InputSourceType) atoi(value.c_str());
            if( name == "inputAudioDevice" )        _inputAudioDevice = atoi(value.c_str());
            if( name == "frequency" )               _frequency = atoi(value.c_str());
            if( name == "receiverModeType" )        _receiverModeType = (ReceiverModeType) atoi(value.c_str());
            if( name == "remoteServer" )            _remoteServer = value;
            if( name == "remotePort" )              _remotePort = atoi(value.c_str());
            if( name == "rfGain" )                  _rfGain = atoi(value.c_str());
            if( name == "volume" )                  _volume = atoi(value.c_str());
            if( name == "dumpRf" )                  _dumpRf = atoi(value.c_str());
            if( name == "dumpAudio" )               _dumpAudio = atoi(value.c_str());
            if( name == "dumpFileFormat" )          _dumpFileFormat = (DumpFileFormatType) atoi(value.c_str());
            if( name == "signalGeneratorFrequency") _signalGeneratorFrequency = atol(value.c_str());
            if( name == "pcmFile" )                 _pcmFile = value;
        }
        configStream >> opt;
    }

    // Done reading the config file
    configStream.close();

    // Set flags
    if( _inputAudioDevice > -1 ) {
        _isRemoteHead = false;
        _useRemoteHead = false;
        HLog("Has input audio device from stored config, use local input");
    }
    if( _inputAudioDevice == -1 && _remoteServer.empty() && _remotePort > 0 ) {
        _useRemoteHead = true;
        _isRemoteHead = false;
        HLog("Has remote port but no remote server, use remote head");
    }
    if( _inputAudioDevice == -1 && !_remoteServer.empty() && _remotePort > 0 ) {
        _useRemoteHead = false;
        _isRemoteHead = true;
        HLog("Has remote port and remote server, is remote head");
    }

    // Read a stored config
    return true;
}


void ConfigOptions::SaveStoredConfig() {

    // Get the users homedirectory
    const char* home = std::getenv("HOME");
    if( home == NULL ) {
        HError("No HOME env. variable. Unable to save configuration");
        return;
    }

    // Compose the config path
    std::string configPath(home);
    configPath += "/.booma";

    // Check of the directory exists
    struct stat stats;
    if( stat(configPath.c_str(), &stats) != -1 ) {
        if( !S_ISDIR(stats.st_mode) ) {
            HError("File ~/.booma exists, but should be a directory");
            return;
        }
        HLog("Config directory %s exists", configPath.c_str());
    }
    else
    {
        HLog("Creating config directory ~/.booma");
        if (mkdir(configPath.c_str(), 0755) == -1) {
            HError( (std::string("Error when creating ~/.booma: ") + strerror(errno)).c_str() );
            return;
        }
    }

    // Compose path to config file
    std::string configFile(configPath);
    configFile += "/config.ini";
    HLog("Config file is %s", configFile.c_str());

    // Open the config file
    std::ofstream configStream;
    configStream.open(configFile);
    if( !configStream.is_open() ) {
        HError("Failed to open config file for writing");
        return;
    }

    // Write config settings
    configStream << "sampleRate=" << _sampleRate << std::endl;
    configStream << "outputAudioDevice=" << _outputAudioDevice << std::endl;
    configStream << "inputSourceType=" << _inputSourceType << std::endl;
    configStream << "inputAudioDevice=" << _inputAudioDevice << std::endl;
    configStream << "frequency=" << _frequency << std::endl;
    configStream << "receiverModeType=" << _receiverModeType << std::endl;
    configStream << "remoteServer=" << _remoteServer << std::endl;
    configStream << "remotePort=" << _remotePort << std::endl;
    configStream << "rfGain=" << _rfGain << std::endl;
    configStream << "volume=" << _volume << std::endl;
    configStream << "dumpRf=" << _dumpRf << std::endl;
    configStream << "dumpAudio=" << _dumpAudio << std::endl;
    configStream << "dumpFileFormat" << _dumpFileFormat << std::endl;
    configStream << "signalGeneratorFrequency=" << _signalGeneratorFrequency << std::endl;
    configStream << "pcmFile=" << _pcmFile << std::endl;

    // Done writing the config file
    configStream.close();
}

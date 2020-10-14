#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

//#include <include/auroralreceiver.h>

#include "booma.h"
#include "configoptions.h"
#include "language.h"

void ConfigOptions::PrintUsage() {
    std::cout << tr("Usage: booma-console [-option [parameter, ...]]") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Information]==") << std::endl;
    std::cout << tr("Show a list of audio devices                             -c --cards") << std::endl;
    std::cout << tr("Show a list of RTL-SDR devices                           -r --rtlsdrs") << std::endl;
    std::cout << tr("Show this help and exit                                  -h --help") << std::endl;
    std::cout << tr("Show version and exit                                    -v --version") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Input, data type and samplerate(s)]==") << std::endl;
    std::cout << tr("Use audio input source                                   -i AUDIO devicenumber") << std::endl;
    std::cout << tr("Use RTL-SDR input source (datatype defaults to IQ)       -i RTLSDR devicenumber") << std::endl;
    std::cout << tr("Use network input                                        -i NETWORK address dataport commandport") << std::endl;
    std::cout << tr("Set input datatype (required for NETWORK and PCM input)  -it REAl|IQ|I|Q") << std::endl;
    std::cout << tr("Set input type (required for NETWORK and PCM input)      -is AUDIO|RTLSDR") << std::endl;

    std::cout << tr("Device (input) samplerate (default 48KHz)                -dr rate") << std::endl;
    std::cout << tr("Output samplerate (default 48KHz)                        -or rate") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Receiver, frequency and gain]==") << std::endl;
    std::cout << tr("Select receiver (CW default)                             -m CW|AURORAL|AM|SSB") << std::endl;
    std::cout << tr("Select frequency (default 17.2KHz)                       -f frequecy") << std::endl;
    std::cout << tr("Rf gain (default 10)                                     -g gain") << std::endl;
    std::cout << tr("Set receiver option (can be repeated)                    -ro NAME=VALUE") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Output, recordings]==") << std::endl;
    std::cout << tr("Select output (audio) device                             -o devicenumber") << std::endl;
    std::cout << tr("Output volume (default 5)                                -l volume") << std::endl;
    std::cout << tr("Dump rf input as pcm to file                             -p PCM (enable) | -p OFF (disable)") << std::endl;
    std::cout << tr("Dump rf input as wav to file (default)                   -p WAV (enable) | -p OFF (disable)") << std::endl;
    std::cout << tr("Dump output audio as pcm to file                         -a PCM (enable) | -a OFF (disable)") << std::endl;
    std::cout << tr("Dump output audio as wav to file                         -a WAV (enable) | -a OFF (disable)") << std::endl;
    std::cout << tr("Dump file suffix. If not set, a timestamp is used        -dfs suffix") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Use remote head]==") << std::endl;
    std::cout << tr("Server for remote input                                  -s dataport commandport") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Options]==") << std::endl;
    std::cout << tr("Wait untill scheduled time                               -b 'YYYY-MM-DD HH:MM' 'YYYY-MM-DD HH:MM' (begin .. end)") << std::endl;
    std::cout << tr("Set initial buffersize for file IO (0 to disable)        -n reserved-block") << std::endl;
    std::cout << tr("RTL-SDR frequency correction                             -rtlc correction") << std::endl;
    std::cout << tr("RTL-SDR tuning offset                                    -rtlo offset") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Debugging]==") << std::endl;
    std::cout << tr("Verbose debug output                                     -d --debug") << std::endl;
    std::cout << tr("Use sine generator as input                              -i GENERATOR frequency") << std::endl;
    std::cout << tr("Use pcm file as input                                    -i PCM filename") << std::endl;
    std::cout << tr("Use wav file as input                                    -i WAV filename") << std::endl;
    std::cout << tr("Use silence as input                                     -i SILENCE") << std::endl;
    std::cout << tr("Select /dev/null as output device                        -o -1") << std::endl;
    std::cout << tr("Write output to this file                                -o filename") << std::endl;
    std::cout << tr("Enable probes and halt after 100 blocks                  -x") << std::endl;
    std::cout << tr("Reset cached configuration                               -z") << std::endl;
}

void ConfigOptions::PrintAudioDevices() {

    if( HSoundcard::AvailableDevices() == 0 )
    {
        std::cout << "There is no soundcards available on this system" << std::endl; 
        return;
    }
    std::vector<HSoundcard::DeviceInformation> info = HSoundcard::GetDeviceInformation();
    std::cout << std::endl;
    for( std::vector<HSoundcard::DeviceInformation>::iterator it = info.begin(); it != info.end(); it++)
    {
        std::cout << "Device: " << (*it).Device << std::endl;
        std::cout << "        \"" << (*it).Name << "\"" << std::endl;
        std::cout << "        Inputs:   " << (*it).Inputs << std::endl;
        std::cout << "        Outputs:  " << (*it).Outputs << std::endl;
        std::cout << std::endl;
    }
}

void ConfigOptions::PrintRtlsdrDevices() {
    if( HRtl2832::AvailableDevices() == 0)
    {
        std::cout << "There is no RTL-SDR devices connected to this system" << std::endl;
        return;
    }
    std::vector<HRtl2832::DeviceInformation> info = HRtl2832::GetDeviceInformation();
    std::cout << std::endl;
    for( std::vector<HRtl2832::DeviceInformation>::iterator it = info.begin(); it != info.end(); it++)
    {
        std::cout << "Device: " << (*it).Device << std::endl;
        std::cout << "        \"" << (*it).Vendor << " " << (*it).Product << "\"" << std::endl;
        std::cout << std::endl;
    }
}

bool ConfigOptions::IsVerbose(int argc, char** argv) {

    for( int i = 1; i < argc; i++ ) {
        if( strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0 ) {
            return true;
        }
    }
    return false;
}

ConfigOptions::ConfigOptions(std::string appName, std::string appVersion, int argc, char** argv) {

    // First pass: help or version
    for( int i = 1; i < argc; i++ ) {

        // Show available audio devices
        if( strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--cards") == 0 ) {
            PrintAudioDevices();
            exit(0);
        }

        // Show available RTL-SDR devices
        if( strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--rtlsdrs") == 0 ) {
            PrintRtlsdrDevices();
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
    if( !ReadStoredConfig(CONFIGNAME) && argc == 1 ) {
        std::cout << "No stored config in ~/.booma/" << CONFIGNAME << " and no arguments. Kindly presenting options" << std::endl << std::endl;
        PrintUsage();
        exit(1);
    }

    // Second pass: config options
    for( int i = 1; i < argc; i++ ) {

        // Enable probes
        if( strcmp(argv[i], "-x") == 0 ) {
            _enableProbes = true;
            continue;
        }

        // Dump output as ... to file
        if( strcmp(argv[i], "-a") == 0 && i < argc - 1) {
            if( strcmp(argv[i + 1], "PCM") == 0 ) {
                _dumpAudio = true;
                _dumpAudioFileFormat = PCM;
            } else if( strcmp(argv[i + 1], "WAV") == 0 ) {
                _dumpAudio = true;
               _dumpAudioFileFormat = WAV;
            } else {
                _dumpAudio = false;
            }
            i++;
            continue;
        }

        // Dump file suffix
        if( strcmp(argv[i], "-dfs") == 0 && i < argc - 1) {
            _dumpFileSuffix = argv[i + 1];
            i++;
            continue;
        }

        // Frequency
        if( strcmp(argv[i], "-f") == 0 && i < argc - 1) {
            _frequency = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Rf gain
        if( strcmp(argv[i], "-g") == 0 && i < argc - 1) {
            _rfGain = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Select input device and optional device-specific settings
        if( strcmp(argv[i], "-i") == 0 && i < argc - 2) {
            if( strcmp(argv[i + 1], "AUDIO") == 0 ) {
                _inputSourceType = AUDIO_DEVICE;
                _inputSourceDataType = (_inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ? REAL_INPUT_SOURCE_DATA_TYPE : _inputSourceDataType);
                _inputDevice = atoi(argv[i + 2]);
                _isRemoteHead = false;
                HLog("Input audio device %d", _inputDevice);
            }
            else if( strcmp(argv[i + 1], "GENERATOR") == 0 ) {
                _inputSourceType = SIGNAL_GENERATOR;
                _signalGeneratorFrequency = atoi(argv[i + 2]);
                _inputSourceDataType = (_inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ? REAL_INPUT_SOURCE_DATA_TYPE : _inputSourceDataType);
                _isRemoteHead = false;
                HLog("Input generator running at %d Hz", _signalGeneratorFrequency);
            }
            else if( strcmp(argv[i + 1], "PCM") == 0 ) {
                _inputSourceType = PCM_FILE;
                _pcmFile = argv[i + 2];
                _inputSourceDataType = (_inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ? REAL_INPUT_SOURCE_DATA_TYPE : _inputSourceDataType);
                _isRemoteHead = false;
                HLog("Input file %s", _pcmFile.c_str());
            }
            else if( strcmp(argv[i + 1], "WAV") == 0 ) {
                _inputSourceType = WAV_FILE;
                _wavFile = argv[i + 2];
                _inputSourceDataType = (_inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ? REAL_INPUT_SOURCE_DATA_TYPE : _inputSourceDataType);
                _isRemoteHead = false;
                HLog("Input file %s", _wavFile.c_str());
            }
            else if( strcmp(argv[i + 1], "SILENCE") == 0 ) {
                _inputSourceType = SILENCE;
                _inputSourceDataType = (_inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ? REAL_INPUT_SOURCE_DATA_TYPE : _inputSourceDataType);
                _isRemoteHead = false;
                i -= 1;
                HLog("input silence");
            }
            else if( strcmp(argv[i + 1], "RTLSDR") == 0 ) {
                _inputDevice = atoi(argv[i + 2]);
                _inputSourceType = RTLSDR;
                _inputSourceDataType = (_inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ? IQ_INPUT_SOURCE_DATA_TYPE : _inputSourceDataType);
                _isRemoteHead = false;
                HLog("Input RTL-SDR device %d", _inputDevice);
            }
            else if( strcmp(argv[i + 1], "NETWORK") == 0 ) {
                _inputDevice = -1;
                _inputSourceType = NETWORK;
                _inputSourceDataType = (_inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ? REAL_INPUT_SOURCE_DATA_TYPE : _inputSourceDataType);
                _remoteServer = argv[i + 2];
                _isRemoteHead = true;

                // Optional port assignments
                if( i < argc - 3 && argv[i + 3][0] != '-' ) {
                    _remoteDataPort = atoi(argv[i + 3]);
                    if( i < argc - 4 && argv[i + 4][0] != '-' ) {
                        _remoteCommandPort = atoi(argv[i + 4]);
                        i++;
                    } else {
                        _remoteCommandPort = _remoteDataPort + 1;
                    }
                    i++;
                } else {
                    _remoteDataPort = 1720;
                    _remoteCommandPort = _remoteDataPort + 1;
                }

                HLog("Input NETWORK device %s:%d+%d", _remoteServer.c_str(), _remoteDataPort, _remoteCommandPort);
            }
            else
            {
                std::cout << tr("Unknown input source. Please use on of the types AUDIO|RTLSDR|GENERATOR|PCM|WAV|SILENCE") << std::endl;
                exit(1);
            }

            i += 2;
            continue;
        }

        // Input data type (if different than the default)
        if( strcmp(argv[i], "-it") == 0 && i < argc - 1) {
            if( strcmp(argv[i + 1], "IQ") == 0 ) {
                _inputSourceDataType = IQ_INPUT_SOURCE_DATA_TYPE;
                HLog("Setting input data type to IQ");
            }
            else if( strcmp(argv[i + 1], "I") == 0 ) {
                _inputSourceDataType = I_INPUT_SOURCE_DATA_TYPE;
                HLog("Setting input data type to I");
            }
            else if( strcmp(argv[i + 1], "Q") == 0 ) {
                _inputSourceDataType = Q_INPUT_SOURCE_DATA_TYPE;
                HLog("Setting input data type to Q");
            }
            else if( strcmp(argv[i + 1], "REAL") == 0 ) {
                _inputSourceDataType = REAL_INPUT_SOURCE_DATA_TYPE;
                HLog("Setting input data type to REAL");
            }
            else {
                std::cout << "Unknown or invalid input data type '" << argv[i + 1] << std::endl;
                exit(1);
            }

            i++;
            continue;
        }

        // Input type (if different than the default)
        if( strcmp(argv[i], "-is") == 0 && i < argc - 1) {
            if( strcmp(argv[i + 1], "AUDIO") == 0 ) {
                _originalInputSourceType = AUDIO_DEVICE;
                HLog("Setting input type to AUDIO_DEVICE");
            }
            else if( strcmp(argv[i + 1], "RTLSDR") == 0 ) {
                _originalInputSourceType = RTLSDR;
                HLog("Setting input type to RTLSDR");
            }
            else {
                std::cout << "Invalid or unknown input type '" << argv[i + 1] << std::endl;
                exit(1);
            }

            i++;
            continue;
        }

        // Volume
        if( strcmp(argv[i], "-l") == 0 && i < argc - 1) {
            _volume = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Select receiver mode ...
        if( strcmp(argv[i], "-m") == 0 && i < argc - 1) {
            if( strcmp(argv[i + 1], "CW") == 0 ) {
                _receiverModeType = CW;
            }
            else if( strcmp(argv[i + 1], "AM") == 0 ) {
                _receiverModeType = AM;
            }
            else if( strcmp(argv[i + 1], "AURORAL") == 0 ) {
                _receiverModeType = AURORAL;
            }
            else if( strcmp(argv[i + 1], "SSB") == 0 ) {
                _receiverModeType = SSB;
            }
            else {
                std::cout << "Unknown receiver type " << argv[i + 1] << std::endl;
                exit(1);
            }
            i++;
            continue;
        }

        // Disable file buffers
        if( strcmp(argv[i], "-n") == 0 && i < argc - 1 ) {
            _reservedBuffers = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Select output device or file
        if( strcmp(argv[i], "-o") == 0 && i < argc - 1) {

            for( int pos = 0; pos < strlen(argv[i + 1]); pos++ ) {
                if( argv[i + 1][pos] < '0' || argv[i + 1][pos] > '9' ) {
                    if( argv[i + 1][pos] != '-' || pos > 0 ) {
                        _outputFilename = argv[i + 1];
                        HLog("Output filename set to %s", _outputFilename.c_str());
                        break;
                    }
                }
            }
            if( _outputFilename == "" ) {
                _outputAudioDevice = atoi(argv[i + 1]);
            }
            i++;
            continue;
        }

        // Dump input rf as ...
        if( strcmp(argv[i], "-p") == 0) {
            if( strcmp(argv[i + 1], "PCM") == 0 ) {
                _dumpRf = true;
                _dumpRfFileFormat = PCM;
            } else if( strcmp(argv[i + 1], "WAV") == 0 ) {
                _dumpRf = true;
               _dumpRfFileFormat = WAV;
            } else {
                _dumpRf = false;
            }
            i++;
            continue;
        }

        // Samplerates
        if( strcmp(argv[i], "-dr") == 0 && i < argc - 1) {
            _inputSampleRate = atoi(argv[i + 1]);
            HLog("Input samplerate set to %d", _inputSampleRate);
            i++;
            continue;
        }
        if( strcmp(argv[i], "-or") == 0 && i < argc - 1) {
            _outputSampleRate = atoi(argv[i + 1]);
            HLog("Output samplerate set to %d", _outputSampleRate);
            i++;
            continue;
        }

        if( strcmp(argv[i], "-ro") == 0 && i < argc - 2) {
            std::string s(argv[i + 1]);
            int pos = s.find("=");
            if( pos < 0 ) {
                std::cout << "Option '-ro' must have a parameter on the form 'NAME=VALUE'" << std::endl;
                exit(1);
            }
            if( pos == 0 ) {
                std::cout << "NAME can not be empty in receiver option definition" << std::endl;
                exit(1);
            }
            if( pos >= s.size() - 1 ) {
                std::cout << "VALUE can not be empty in receiver option definition" << std::endl;
                exit(1);
            }
            _receiverOptions[s.substr(0, pos)] = s.substr(pos + 1);
            i++;
            continue;
        }

        // RTL-SDR options
        if( strcmp(argv[i], "-rtlc") == 0 && i < argc - 1) {
            _rtlsdrCorrection = atoi(argv[i + 1]);
            HLog("RTL-SDR frequency correction set to %d", _rtlsdrCorrection);
            i++;
            continue;
        }
        if( strcmp(argv[i], "-rtlo") == 0 && i < argc - 1) {
            _rtlsdrOffset = atoi(argv[i + 1]);
            HLog("RTL-SDR frequency offset set to %d", _rtlsdrOffset);
            i++;
            continue;
        }

        // Server for remote input
        if( strcmp(argv[i], "-s") == 0 && i < argc - 2) {
            _remoteDataPort = atoi(argv[i + 1]);
            _remoteCommandPort = atoi(argv[i + 2]);
            _isRemoteHead = false;
            _useRemoteHead = true;
            i += 2;
            continue;
        }

        // Reset cached configuration
        if( strcmp(argv[i], "-z") == 0 ) {
            RemoveStoredConfig(CONFIGNAME);
            std::cout << tr("Cached configuration has been removed") << std::endl;
            exit(0);
        }

        // Scheduled start and stop
        if( strcmp(argv[i], "-b") == 0 ) {
            _schedule.SetStart(argv[i + 1]);
            i++;
            _schedule.SetStop(argv[i + 1]);
            i++;
            continue;
        }

        // Parameters used outside the config object
        if( strcmp(argv[i], "-d") == 0 ) {
            continue;
        }

        // Unknown parameter
        std::cout << tr("Unknown parameter") << " '" << argv[i] << "' (" << tr("use '-h' to show the help") << ")" << std::endl;
        exit(1);
    }

    // Check configuration for remote server/head
    if( _isRemoteHead && _remoteServer.empty() ) {
        std::cout << tr("Please select address of remote input with '-r address port'") << std::endl;
        exit(1);
    }
    if( _isRemoteHead && _remoteDataPort == 0 ) {
        std::cout << tr("Please select data port of remote input with '-r address dataport commandport'") << std::endl;
        exit(1);
    }
    if( _useRemoteHead && _remoteDataPort == 0 ) {
        std::cout << tr("Please select data port for the input server with '-s dataport commandport'") << std::endl;
        exit(1);
    }
    if( _isRemoteHead && _remoteCommandPort == 0 ) {
        std::cout << tr("Please select command port of remote input with '-r address dataport commandport'") << std::endl;
        exit(1);
    }
    if( _useRemoteHead && _remoteCommandPort == 0 ) {
        std::cout << tr("Please select command  for the input server with '-s dataport commandport'") << std::endl;
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
        if( _inputSourceType == NO_INPUT_SOURCE_TYPE ) {
            std::cout << tr("Please select the input type with '-i ...'") << std::endl;
            exit(1);
        }
        if( _inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ) {
            std::cout << tr("Please select the input data type with '-i ...' or explicit with '-it ...'") << std::endl;
            exit(1);
        }
        else if( _inputSourceType == AUDIO_DEVICE && _inputDevice < 0 ) {
            std::cout << tr("Please select the input audio device with '-i AUDIO devicenumber'") << std::endl;
            std::cout << tr("Hint: Use '-c' to get a list of audio devices on your system (Raspian may require sudo)") << std::endl;
            exit(1);
        }
        else if( _inputSourceType == RTLSDR && _inputDevice < 0 ) {
            std::cout << tr("Please select the input RTLSDR device with '-i RTLSDR devicenumber'") << std::endl;
            std::cout << tr("Hint: Use '-c' to get a list of audio devices on your system (Raspian may require sudo)") << std::endl;
            exit(1);
        }
        else if( _inputSourceType == SIGNAL_GENERATOR && _signalGeneratorFrequency < 0 ) {
            std::cout << tr("Please select the input signal generator frequency with '-i GENERATOR frequency'") << std::endl;
            exit(1);
        }
        else if( _inputSourceType == PCM_FILE ) {

            // We need a filename
            if( _pcmFile.empty() ) {
                std::cout << tr("Please select the input filename with '-i PCM filename'") << std::endl;
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
        else if( _inputSourceType == WAV_FILE ) {

            // We need a filename
            if( _wavFile.empty() ) {
                std::cout << tr("Please select the input filename with '-i WAV filename'") << std::endl;
                exit(1);
            }

            // Check if the input file exists
            struct stat stats;
            if( stat(_wavFile.c_str(), &stats) != -1 ) {
                if( !S_ISREG(stats.st_mode) ) {
                    std::cout << "Input file does not exist" << std::endl;
                    exit(1);
                }
            }
        }
    }

    // Sanitize sample rate
    if( _inputSampleRate == _outputSampleRate && (_inputSourceType == RTLSDR || _originalInputSourceType == RTLSDR) ) {
        std::cout << "Input (device) and output sample rate should not be the same for RTL-SDR devices" << std::endl;
        exit(1);
    }
    if( _inputSourceType != RTLSDR && _originalInputSourceType != RTLSDR ) {
        _inputSampleRate = _outputSampleRate;
    }
}

ConfigOptions::~ConfigOptions() {
    WriteStoredConfig(CONFIGNAME);
}

void ConfigOptions::RemoveStoredConfig(std::string configname) {

    // Get the users homedirectory
    const char* home = std::getenv("HOME");
    if( home == NULL ) {
        HError("No HOME env. variable. Unable to read configuration");
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
        HLog("Config directory does not exists, no config to remove");
        return;
    }

    // Compose path to config file
    std::string configFile(configPath);
    configFile += "/" + configname;
    HLog("Config file is %s", configFile.c_str());

    // Remove the config file
    remove(configFile.c_str());
    HLog("Removed the config file");
}

bool ConfigOptions::ReadStoredConfig(std::string configname) {

    // Get the users homedirectory
    const char* home = std::getenv("HOME");
    if( home == NULL ) {
        HError("No HOME env. variable. Unable to read configuration");
        return true;
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
    configFile += "/" + configname;
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

            // Known config names
            HLog("config value (%s=%s)", name.c_str(), value.c_str());
            if( name == "inputSampleRate" )          _inputSampleRate = atoi(value.c_str());
            if( name == "outputSampleRate" )         _outputSampleRate = atoi(value.c_str());
            if( name == "outputAudioDevice" )       _outputAudioDevice = atoi(value.c_str());
            if( name == "inputSourceType" )         _inputSourceType = (InputSourceType) atoi(value.c_str());
            if( name == "inputSourceDataType" )     _inputSourceDataType = (InputSourceDataType) atoi(value.c_str());
            if( name == "inputDevice" )             _inputDevice = atoi(value.c_str());
            if( name == "frequency" )               _frequency = atoi(value.c_str());
            if( name == "receiverModeType" )        _receiverModeType = (ReceiverModeType) atoi(value.c_str());
            if( name == "remoteServer" )            _remoteServer = value;
            if( name == "remoteDataPort" )          _remoteDataPort = atoi(value.c_str());
            if( name == "remoteCommandPort" )       _remoteCommandPort = atoi(value.c_str());
            if( name == "rfGain" )                  _rfGain = atoi(value.c_str());
            if( name == "volume" )                  _volume = atoi(value.c_str());
            if( name == "dumpRfFileFormat" )        _dumpRfFileFormat = (DumpFileFormatType) atoi(value.c_str());
            if( name == "dumpAudioFileFormat" )     _dumpAudioFileFormat = (DumpFileFormatType) atoi(value.c_str());
            if( name == "signalGeneratorFrequency") _signalGeneratorFrequency = atol(value.c_str());
            if( name == "pcmFile" )                 _pcmFile = value;
            if( name == "wavFile" )                 _wavFile = value;
            if( name == "reservedBuffers" )         _reservedBuffers = atoi(value.c_str());
            if( name == "receiverOptionsFor" )      _receiverOptionsFor = ReadStoredReceiverOptionsFor(value);

            // Historic config names
            if( name == "inputAudioDevice" )        _inputDevice = atoi(value.c_str());
        }
        configStream >> opt;
    }

    // Done reading the config file
    configStream.close();

    // Set flags
    if( _inputDevice > -1 ) {
        _isRemoteHead = false;
        _useRemoteHead = false;
        HLog("Has input device from stored config, use local input");
    }
    else if( _inputDevice == -1 && _remoteServer.empty() && _remoteDataPort > 0 ) {
        _useRemoteHead = true;
        _isRemoteHead = false;
        HLog("Has remote port but no remote server, use remote head");
    }
    else if( _inputDevice == -1 && !_remoteServer.empty() && _remoteDataPort > 0 ) {
        _useRemoteHead = false;
        _isRemoteHead = true;
        _inputSourceType = NETWORK;
        HLog("Has remote port and remote server, is remote head");
    }

    // Read a stored config
    return true;
}


void ConfigOptions::WriteStoredConfig(std::string configname) {

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
    configFile += "/" + configname;
    HLog("Config file is %s", configFile.c_str());

    // Open the config file
    std::ofstream configStream;
    configStream.open(configFile);
    if( !configStream.is_open() ) {
        HError("Failed to open config file for writing");
        return;
    }

    // Write config settings
    configStream << "inputSampleRate=" << _inputSampleRate << std::endl;
    configStream << "outputSampleRate=" << _outputSampleRate << std::endl;
    configStream << "outputAudioDevice=" << _outputAudioDevice << std::endl;
    configStream << "inputSourceType=" << _inputSourceType << std::endl;
    configStream << "inputSourceDataType=" << _inputSourceDataType << std::endl;
    configStream << "inputDevice=" << _inputDevice << std::endl;
    configStream << "frequency=" << _frequency << std::endl;
    configStream << "receiverModeType=" << _receiverModeType << std::endl;
    configStream << "remoteServer=" << _remoteServer << std::endl;
    configStream << "remoteDataPort=" << _remoteDataPort << std::endl;
    configStream << "remoteCommandPort=" << _remoteCommandPort << std::endl;
    configStream << "rfGain=" << _rfGain << std::endl;
    configStream << "volume=" << _volume << std::endl;
    configStream << "dumpRfFileFormat=" << _dumpRfFileFormat << std::endl;
    configStream << "dumpAudioFileFormat=" << _dumpAudioFileFormat << std::endl;
    configStream << "signalGeneratorFrequency=" << _signalGeneratorFrequency << std::endl;
    configStream << "pcmFile=" << _pcmFile << std::endl;
    configStream << "wavFile=" << _wavFile << std::endl;
    configStream << "reservedBuffers=" << _reservedBuffers << std::endl;
    configStream << "receiverOptionsFor=" << WriteStoredReceiverOptionsFor(_receiverOptionsFor) << std::endl;

    // Done writing the config file
    configStream.close();
}

std::map<std::string, std::string> ConfigOptions::ReadStoredReceiverOptions(std::string optionsString) {
    std::map<std::string, std::string> options;
    if( optionsString.size() == 0 ) {
        return options;
    }

    int pos = optionsString.find("=");
    int stop;
    while( pos >= 0 ) {
        stop = optionsString.find(",");
        if( stop < pos ) {
            stop = optionsString.size();
        }

        std::string name = optionsString.substr(0, pos);
        std::string value = optionsString.substr(pos + 1, stop - pos - 1);
        options[name] = value;

        if( stop < optionsString.size() ) {
            optionsString = optionsString.substr(stop + 1);
        } else {
            optionsString = "";
        }

        pos = optionsString.find("=");
    }

    return options;
}

std::string ConfigOptions::WriteStoredReceiverOptions(std::map<std::string, std::string> options) {
    std::string optionsString = "";
    for( std::map<std::string, std::string>::iterator it = options.begin(); it != options.end(); it++ ) {
        optionsString += (*it).first + "=" + (*it).second + ",";
    }
    if( optionsString[optionsString.size() - 1] == ',') {
        optionsString = optionsString.substr(0, optionsString.size() - 1);
    }
    return optionsString;
}

void ConfigOptions::SetReceiverOptionsFor(std::string receiver, std::map<std::string, std::string> options) {
    std::map<std::string, std::map<std::string, std::string>>::iterator it = _receiverOptionsFor.find(receiver);
    if( it != _receiverOptionsFor.end() ) {
        HLog("Has existing options map for receiver %s", receiver.c_str());
        it->second = options;
        return;
    }
    HLog("Adding options map for receiver %s", receiver.c_str());
    _receiverOptionsFor[receiver] = options;
}

std::map<std::string, std::string> ConfigOptions::GetReceiverOptionsFor(std::string receiver) {
    std::map<std::string, std::map<std::string, std::string>>::iterator it = _receiverOptionsFor.find(receiver);
    if( it != _receiverOptionsFor.end() ) {
        HLog("Found existing options map for receiver %s", receiver.c_str());
        return it->second;
    }
    HLog("No existing options map for receiver %s. Returning empty list", receiver.c_str());
    std::map<std::string, std::string> empty;
    return empty;
}

std::string ConfigOptions::WriteStoredReceiverOptionsFor(std::map<std::string, std::map<std::string, std::string>> options) {
    std::string optionsString = "";
    for( std::map<std::string, std::map<std::string, std::string>>::iterator rit = options.begin(); rit != options.end(); rit++ ) {
        optionsString += rit->first + ":{";
        optionsString += WriteStoredReceiverOptions(rit->second);
        optionsString += "},";
    }
    if( optionsString[optionsString.size() - 1] == ',') {
        optionsString = optionsString.substr(0, optionsString.size() - 1);
    }
    return optionsString;
}

std::map<std::string, std::map<std::string, std::string>> ConfigOptions::ReadStoredReceiverOptionsFor(std::string optionsForString) {
    std::map<std::string, std::map<std::string, std::string>> optionsFor;
    if( optionsForString.size() == 0 ) {
        return optionsFor;
    }

    int pos = optionsForString.find("{");
    int stop;
    while( pos >= 0 ) {
        stop = optionsForString.find("}");
        if( stop < pos ) {
            stop = optionsForString.size() - 1;
        }

        std::string receiver = optionsForString.substr(0, pos - 1);
        std::string value = optionsForString.substr(pos + 1, stop - pos - 1);

        optionsFor[receiver] = ReadStoredReceiverOptions (value);

        if( stop + 1 < optionsForString.size() ) {
            optionsForString = optionsForString.substr(stop + 2);
        } else if( stop < optionsForString.size() ) {
            optionsForString = optionsForString.substr(stop + 1);
        }

        pos = optionsForString.find("{");
    }

    return optionsFor;
}

void ConfigOptions::WriteBookmark(std::string name) {
    WriteStoredConfig(name + ".bookmark");
}

bool ConfigOptions::ReadBookmark(std::string name) {
    return ReadStoredConfig(name + ".bookmark");
}

void ConfigOptions::DeleteBookmark(std::string name) {
    RemoveStoredConfig(name + ".bookmark");
}

std::vector<std::string> ConfigOptions::ListBookmarks() {
    std::vector<std::string> bms;    

    // Get the users homedirectory
    const char* home = std::getenv("HOME");
    if( home == NULL ) {
        HError("No HOME env. variable. Unable to list bookmarks");
        return bms;
    }

    // Compose the config path
    std::string configPath(home);
    configPath += "/.booma";

    // Check of the directory exists
    struct stat stats;
    if( stat(configPath.c_str(), &stats) != -1 ) {
        if( !S_ISDIR(stats.st_mode) ) {
            HError("File ~/.booma exists, but should be a directory");
            return bms;
        }
        HLog("Config directory %s exists", configPath.c_str());
    }
    else
    {
        HLog("Config directory does not exists, no bookmarks to list");
        return bms;
    }

    // List content of the config folder
    DIR *directory;
    struct dirent *entry;
    if( (directory = opendir( configPath.c_str()) ) != NULL ) {
        while( (entry = readdir( directory )) != NULL ) {
            std::string bm( entry->d_name );
            
            if( bm != "." && bm != ".." && bm != CONFIGNAME && bm.find(".bookmark") != std::string::npos )  {
                bms.push_back( bm.substr(0, bm.size() - strlen(".bookmark")) );
            }
        }
        closedir (directory);
    } else {
        HError("Unable to list content of the ~/.booma folder");
    }

    // Return the list of bookmarks
    return bms;
}

#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>

#include "booma.h"
#include "configoptions.h"
#include "language.h"

void ConfigOptions::PrintUsage(bool showSecretSettings) {
    std::cout << tr("Usage: booma-console [-option [parameter, ...]]") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Information]==") << std::endl;
    std::cout << tr("Show a list of hardware audio devices                    -c --cards") << std::endl;
    std::cout << tr("Show a list of all audio devices                         -ac --allcards") << std::endl;
    std::cout << tr("Show a list of virtual audio devices                     -vc --allcards") << std::endl;
    std::cout << tr("Show a list of RTL-SDR devices                           -r --rtlsdrs") << std::endl;
    std::cout << tr("Show this help and exit                                  -h --help") << std::endl;
    std::cout << tr("Show version and exit                                    -v --version") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Input, data type and samplerate(s)]==") << std::endl;
    std::cout << tr("Use audio input source                                   -i AUDIO devicenumber") << std::endl;
    std::cout << tr("Use RTL-SDR input source (datatype defaults to IQ)       -i RTLSDR devicenumber") << std::endl;
    std::cout << tr("Use pcm file as input                                    -i PCM filename") << std::endl;
    std::cout << tr("Use wav file as input                                    -i WAV filename") << std::endl;
    std::cout << tr("Use network input                                        -i NETWORK address dataport commandport") << std::endl;
    std::cout << tr("Set input datatype (required for NETWORK and PCM input)  -it REAl|IQ|I|Q") << std::endl;
    std::cout << tr("Set input type (required for NETWORK and PCM input)      -is AUDIO|RTLSDR") << std::endl;
    std::cout << tr("Device (input) samplerate (default 48KHz)                -dr rate") << std::endl;
    std::cout << tr("Output samplerate (default 48KHz)                        -or rate") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Receiver, frequency and gain]==") << std::endl;
    std::cout << tr("Select receiver (CW default)                             -m CW|AURORAL|AM|SSB") << std::endl;
    std::cout << tr("Select frequency (default 17.2KHz)                       -f frequecy") << std::endl;
    std::cout << tr("Rf gain (default 0 = auto)                               -g gain") << std::endl;
    std::cout << tr("Set receiver option (can be repeated)                    -ro NAME=VALUE") << std::endl;
    std::cout << tr("Enable or disable RF gain (AGC) (default enabled)        -rfg 1 (enable) or -rfg 0 (disable)") << std::endl;
    std::cout << tr("Set preamp level (default off)                           -pa -1 (-12dB) or -pa 0 (off) or -pa 1 (+12dB)") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Output, recordings]==") << std::endl;
    std::cout << tr("Select output (audio) device                             -o devicenumber") << std::endl;
    std::cout << tr("Write output to this file                                -o filename") << std::endl;
    std::cout << tr("Output volume (default 5)                                -l volume") << std::endl;
    std::cout << tr("Dump rf input as pcm to file                             -p PCM (enable) | -p OFF (disable)") << std::endl;
    std::cout << tr("Dump rf input as wav to file (default)                   -p WAV (enable) | -p OFF (disable)") << std::endl;
    std::cout << tr("Dump output audio as pcm to file                         -a PCM (enable) | -a OFF (disable)") << std::endl;
    std::cout << tr("Dump output audio as wav to file                         -a WAV (enable) | -a OFF (disable)") << std::endl;
    std::cout << tr("Dump file suffix. If not set, a timestamp is used        -dfs suffix") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Use with converters]==") << std::endl;
    std::cout << tr("Up-/Downconverter in use                                 -shift basefrequency") << std::endl;
    std::cout << tr("RTL-SDR tuning error alignment (default 0)               -rtla adjustment") << std::endl;
    std::cout << tr("Enable RTL-SDR frequency align mode                      -fa") << std::endl;
    std::cout << tr("Frequency align mode output volume (default 500)         -fav volume") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Configuration sections]==") << std::endl;
    std::cout << tr("Use existing or create new configuration section         -config section") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Remote head operation]==") << std::endl;
    std::cout << tr("Server for remote input                                  -s dataport commandport") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Options]==") << std::endl;
    std::cout << tr("Wait untill scheduled time                               -b 'YYYY-MM-DD HH:MM' 'YYYY-MM-DD HH:MM' (begin .. end)") << std::endl;
    std::cout << tr("Set initial buffersize for file IO (0 to disable)        -n reserved-block") << std::endl;
    std::cout << tr("Reset cached configuration                               -z") << std::endl;
    std::cout << std::endl;

    std::cout << tr("==[Performance and quality (not persisted)]==") << std::endl;
    std::cout << tr("FIR filter size for decimation (default 51)              -ffs points") << std::endl;
    std::cout << tr("1.st IF filter width (default 10000)                     -ifw width") << std::endl;
    std::cout << std::endl;

    if( showSecretSettings ) {
        std::cout << tr("==[Internal settings, try to leave untouched!!]==") << std::endl;
        std::cout << tr("=========(These settings are NOT stored)=========") << std::endl;
        std::cout << tr("RTL-SDR frequency correction (default 0)                 -rtlc correction") << std::endl;
        std::cout << tr("RTL-SDR tuning offset (default 6000)                     -rtlo offset") << std::endl;
        std::cout << tr("RTL-SDR frequency correction factor (default 0)          -rtlf factor") << std::endl;
        std::cout << tr("RTL-SDR gain (default 0 = auto)                          -rtlg gain") << std::endl;
        std::cout << tr("Decimation gain for high rate inputs (default 0 = auto)  -dg gain") << std::endl;
        std::cout << tr("Agc level for automatic decimator gain (default 1000)    -dal level") << std::endl;
        std::cout << tr("Automatic RF gain level (default 500)                    -ral level") << std::endl;
        std::cout << tr("AF FFT agc level (default 255)                           -afl level") << std::endl;
        std::cout << std::endl;

        std::cout << tr("==[Debugging]==") << std::endl;
        std::cout << tr("Verbose debug output                                     -d --debug") << std::endl;
        std::cout << tr("Use sine generator as input                              -i GENERATOR frequency") << std::endl;
        std::cout << tr("Use silence as input                                     -i SILENCE") << std::endl;
        std::cout << tr("Select /dev/null as output device.                       -o -1") << std::endl;
        std::cout << tr("Enable probes and halt after 100 blocks                  -x") << std::endl;
        std::cout << std::endl;
    } else {
        std::cout << tr("==[Debugging and internal settings best left untouched]==") << std::endl;
        std::cout << tr("Show options for debugging and internal settings         -hh") << std::endl;
        std::cout << std::endl;
    }
}

void ConfigOptions::PrintAudioDevices(bool hardwareDevices, bool virtualDevices) {

    if( HSoundcard::AvailableDevices() == 0 )
    {
        std::cout << "There is no soundcards available on this system" << std::endl; 
        return;
    }

    std::vector<HSoundcard::DeviceInformation> info = HSoundcard::GetDeviceInformation();
    std::cout << std::endl;
    for( std::vector<HSoundcard::DeviceInformation>::iterator it = info.begin(); it != info.end(); it++)
    {
        if( ((*it).Name.find("(hw:") != std::string::npos && hardwareDevices) || ((*it).Name.find("(hw:") == std::string::npos && virtualDevices)) {
            std::cout << "Device: " << (*it).Device << std::endl;
            std::cout << "        \"" << (*it).Name << "\"" << std::endl;
            std::cout << "        Inputs:   " << (*it).Inputs << std::endl;
            std::cout << "        Outputs:  " << (*it).Outputs << std::endl;
            std::cout << std::endl;
        }
    }
}

std::string ConfigOptions::GetAudioDevice(int device) {

    if( HSoundcard::AvailableDevices() == 0 )
    {
        return "(no devices)";
    }

    std::vector<HSoundcard::DeviceInformation> info = HSoundcard::GetDeviceInformation();
    for( std::vector<HSoundcard::DeviceInformation>::iterator it = info.begin(); it != info.end(); it++)
    {
        if( (*it).Device == device ) {
            return "\"" + (*it).Name + "\"";
        }
    }
    return "(not found)";
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
        std::cout << "    Vendor: " << (*it).Vendor << std::endl;
        std::cout << "    Product: " << (*it).Product << std::endl;
        std::cout << "    Available gain values: ";
        int cols = 0;
        for( std::vector<int>::iterator git = (*it).Gain.begin(); git != (*it).Gain.end(); git++ ) {
            std::cout << std::to_string(*git) << " ";
            if( cols++ >= 10 ) {
                std::cout << std::endl << "                           ";
                cols = 0;
            }
        }
        if( cols != 0 ) {
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

std::string ConfigOptions::GetRtlsdrDevice(int device) {

    if( HRtl2832::AvailableDevices() == 0)
    {
        return "(no devices)";
    }

    std::vector<HRtl2832::DeviceInformation> info = HRtl2832::GetDeviceInformation();
    for( std::vector<HRtl2832::DeviceInformation>::iterator it = info.begin(); it != info.end(); it++)
    {
        if( (*it).Device == device ) {
            return "\"" + (*it).Vendor + " " + (*it).Product + "\"";
        }
    }
    return "(not found)";
}

std::map<int, std::string> ConfigOptions::GetAudioDevices(bool hardwareDevices, bool virtualDevices, bool inputs, bool outputs) {
    std::map<int, std::string> cards;

    if( HSoundcard::AvailableDevices() == 0 )
    {
        HLog("There is no soundcards available on this system");
        return cards;
    }

    std::vector<HSoundcard::DeviceInformation> info = HSoundcard::GetDeviceInformation();
    for( std::vector<HSoundcard::DeviceInformation>::iterator it = info.begin(); it != info.end(); it++)
    {
        if( ((*it).Name.find("(hw:") != std::string::npos && hardwareDevices) || ((*it).Name.find("(hw:") == std::string::npos && virtualDevices)) {
            if( (inputs && (*it).Inputs > 0) || (outputs && (*it).Outputs > 0) ) {
                cards.insert(std::pair<int, std::string>((*it).Device, (*it).Name));
            }
        }
    }
    return cards;
}

std::map<int, std::string> ConfigOptions::GetRtlsdrDevices() {
    std::map<int, std::string> devices;

    if( HRtl2832::AvailableDevices() == 0)
    {
        HLog("There is no RTL-SDR devices connected to this system");
        return devices;
    }

    std::vector<HRtl2832::DeviceInformation> info = HRtl2832::GetDeviceInformation();
    for( std::vector<HRtl2832::DeviceInformation>::iterator it = info.begin(); it != info.end(); it++)
    {
        devices.insert(std::pair<int, std::string>((*it).Device, (*it).Vendor + ": " + (*it).Product));
    }
    return devices;
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

    // Initial check - make sure that we have a recent version of Hardt
    if( !isVersionOrNewer(2, 0, 1) ) {
        std::cout << "The version of Hardt that is installed (" << getVersion() << ") is too old" << std::endl;
        std::cout << "Please install version 2.0.1 or newer" << std::endl;
        exit(1);
    }
    if( !isVersionMajorOrOlder(2) ) {
        std::cout << "The version of Hardt that is installed (" << getVersion() << ") is too new" << std::endl;
        std::cout << "Please install version 2.0.1 or older" << std::endl;
        exit(1);
    }

    // First pass: help or version
    for( int i = 1; i < argc; i++ ) {

        // Show available audio devices
        if( strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--cards") == 0 ) {
            PrintAudioDevices(true, false);
            exit(0);
        }
        if( strcmp(argv[i], "-ac") == 0 || strcmp(argv[i], "--allcards") == 0 ) {
            PrintAudioDevices(true, true);
            exit(0);
        }
        if( strcmp(argv[i], "-vc") == 0 || strcmp(argv[i], "--virtualcards") == 0 ) {
            PrintAudioDevices(false, true);
            exit(0);
        }

        // Show available RTL-SDR devices
        if( strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--rtlsdrs") == 0 ) {
            PrintRtlsdrDevices();
            exit(0);
        }

        // Get help
        if( strcmp(argv[i], "-hh") == 0 ) {
            PrintUsage(true);
            exit(0);
        }
        if( strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 ) {
            PrintUsage();
            exit(0);
        }

        // Get version information
        if( strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version" ) == 0 ) {
            std::cout << appName << " " << appVersion << " " << tr("using") << " Booma " << BOOMA_MAJORVERSION << "." << BOOMA_MINORVERSION << "." << BOOMA_BUILDNO << " " << tr("and") << " Hardt " << getVersion() << std::endl;
            exit(0);
        }

        // Reset cached configuration
        if( strcmp(argv[i], "-z") == 0 ) {
            RemoveStoredConfig(CONFIGNAME);
            std::cout << tr("Cached configuration has been removed") << std::endl;
            exit(0);
        }
    }

    // Seed configuration with values from last execution
    if( !ReadStoredConfig(CONFIGNAME, false) && argc == 1 ) {
        std::cout << "No stored config in ~/.booma/" << CONFIGNAME << " and no arguments. Kindly presenting options" << std::endl << std::endl;
        PrintUsage();
        exit(1);
    }

    // Second pass: config section
    for( int i = 1; i < argc; i++ ) {

        // Config sections
        if (strcmp(argv[i], "-config") == 0 && i < argc - 1) {
            if (!SetConfigSection(argv[i + 1])) {
                if (!CreateConfigSection(argv[i + 1], false, false)) {
                    std::cout << "Could not create config section '" << argv[i + 1] << "'" << std::endl;
                    exit(1);
                }
            }
            i++;
            continue;
        }
    }

    // Check if we have some channels, if not then add a default set
    if( _values.at(_section)->_channels.empty() ) {
        std::cout << "NO CHANNELS\n";
        _values.at(_section)->_channels = ReadPersistentChannels(CONFIGNAME, _section);
    }

    // Third pass: config options
    for( int i = 1; i < argc; i++ ) {

        // Enable probes
        if( strcmp(argv[i], "-x") == 0 ) {
            HLog("Enabled probe run");
            _values.at(_section)->_enableProbes = true;
            continue;
        }

        // Dump output as ... to file
        if( strcmp(argv[i], "-a") == 0 && i < argc - 1) {
            if( strcmp(argv[i + 1], "PCM") == 0 ) {
                _values.at(_section)->_dumpAudio = true;
                _values.at(_section)->_dumpAudioFileFormat = PCM;
            } else if( strcmp(argv[i + 1], "WAV") == 0 ) {
                _values.at(_section)->_dumpAudio = true;
                _values.at(_section)->_dumpAudioFileFormat = WAV;
            } else {
                _values.at(_section)->_dumpAudio = false;
            }
            i++;
            continue;
        }

        // Dump file suffix
        if( strcmp(argv[i], "-dfs") == 0 && i < argc - 1) {
            _values.at(_section)->_dumpFileSuffix = argv[i + 1];
            i++;
            continue;
        }

        // Frequency
        if( strcmp(argv[i], "-f") == 0 && i < argc - 1) {
            _values.at(_section)->_frequency = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Rf gain
        if( strcmp(argv[i], "-g") == 0 && i < argc - 1) {
            _values.at(_section)->_rfGain = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Rf gain enable/disable
        if( strcmp(argv[i], "-rfg") == 0 && i < argc - 1) {
            if( strcmp(argv[i + 1], "1") == 0 ) {
                _values.at(_section)->_rfGainEnabled = true;
            } else if( strcmp(argv[i + 1], "0") == 0 ) {
                _values.at(_section)->_rfGainEnabled = false;
            } else {
                std::cout << "Unknown parameter value '" << argv[i + 1] << "' for RF gain enable/disable\n";
            }
            i++;
            continue;
        }

        // Preamp level
        if( strcmp(argv[i], "-pa") == 0 && i < argc - 1) {
            if( strcmp(argv[i + 1], "-1") == 0 ) {
                _values.at(_section)->_preamp = -1;
            } else if( strcmp(argv[i + 1], "0") == 0 ) {
                _values.at(_section)->_preamp = 0;
            } else if( strcmp(argv[i + 1], "+1") == 0 ) {
                _values.at(_section)->_preamp = 1;
            } else {
                std::cout << "Unknown parameter value '" << argv[i + 1] << "' for preamp level\n";
            }
            i++;
            continue;
        }

        // Select input device and optional device-specific settings
        if( strcmp(argv[i], "-i") == 0 && i < argc - 1) {
            if( strcmp(argv[i + 1], "AUDIO") == 0 && i < argc - 2 ) {
                _values.at(_section)->_inputSourceType = AUDIO_DEVICE;
                _values.at(_section)->_inputSourceDataType = (_values.at(_section)->_inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ? REAL_INPUT_SOURCE_DATA_TYPE : _values.at(_section)->_inputSourceDataType);
                _values.at(_section)->_originalInputSourceType = _values.at(_section)->_inputSourceType;
                _values.at(_section)->_inputDevice = atoi(argv[i + 2]);
                _values.at(_section)->_isRemoteHead = false;
                HLog("Input audio device %d", _values.at(_section)->_inputDevice);
            }
            else if( strcmp(argv[i + 1], "GENERATOR") == 0 && i < argc - 2 ) {
                _values.at(_section)->_inputSourceType = SIGNAL_GENERATOR;
                _values.at(_section)->_signalGeneratorFrequency = atoi(argv[i + 2]);
                _values.at(_section)->_originalInputSourceType = _values.at(_section)->_inputSourceType;
                _values.at(_section)->_inputSourceDataType = (_values.at(_section)->_inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ? REAL_INPUT_SOURCE_DATA_TYPE : _values.at(_section)->_inputSourceDataType);
                _values.at(_section)->_isRemoteHead = false;
                HLog("Input generator running at %d Hz", _values.at(_section)->_signalGeneratorFrequency);
            }
            else if( strcmp(argv[i + 1], "PCM") == 0 && i < argc - 2 ) {
                _values.at(_section)->_inputSourceType = PCM_FILE;
                _values.at(_section)->_pcmFile = argv[i + 2];
                _values.at(_section)->_inputSourceDataType = (_values.at(_section)->_inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ? REAL_INPUT_SOURCE_DATA_TYPE : _values.at(_section)->_inputSourceDataType);
                _values.at(_section)->_isRemoteHead = false;
                HLog("Input file %s", _values.at(_section)->_pcmFile.c_str());
            }
            else if( strcmp(argv[i + 1], "WAV") == 0 && i < argc - 2 ) {
                _values.at(_section)->_inputSourceType = WAV_FILE;
                _values.at(_section)->_wavFile = argv[i + 2];
                _values.at(_section)->_inputSourceDataType = (_values.at(_section)->_inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ? REAL_INPUT_SOURCE_DATA_TYPE : _values.at(_section)->_inputSourceDataType);
                _values.at(_section)->_isRemoteHead = false;
                HLog("Input file %s", _values.at(_section)->_wavFile.c_str());
            }
            else if( strcmp(argv[i + 1], "SILENCE") == 0 ) {
                _values.at(_section)->_inputSourceType = SILENCE;
                _values.at(_section)->_inputSourceDataType = (_values.at(_section)->_inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ? REAL_INPUT_SOURCE_DATA_TYPE : _values.at(_section)->_inputSourceDataType);
                _values.at(_section)->_originalInputSourceType = _values.at(_section)->_inputSourceType;
                _values.at(_section)->_isRemoteHead = false;
                i -= 1;
                HLog("input silence");
            }
            else if( strcmp(argv[i + 1], "RTLSDR") == 0 && i < argc - 2 ) {
                _values.at(_section)->_inputDevice = atoi(argv[i + 2]);
                _values.at(_section)->_inputSourceType = RTLSDR;
                _values.at(_section)->_inputSourceDataType = (_values.at(_section)->_inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ? IQ_INPUT_SOURCE_DATA_TYPE : _values.at(_section)->_inputSourceDataType);
                _values.at(_section)->_originalInputSourceType = _values.at(_section)->_inputSourceType;
                _values.at(_section)->_isRemoteHead = false;
                HLog("Input RTL-SDR device %d", _values.at(_section)->_inputDevice);
            }
            else if( strcmp(argv[i + 1], "NETWORK") == 0 && i < argc - 2 ) {
                _values.at(_section)->_inputDevice = -1;
                _values.at(_section)->_inputSourceType = NETWORK;
                _values.at(_section)->_inputSourceDataType = (_values.at(_section)->_inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ? REAL_INPUT_SOURCE_DATA_TYPE : _values.at(_section)->_inputSourceDataType);
                _values.at(_section)->_remoteServer = argv[i + 2];
                _values.at(_section)->_isRemoteHead = true;

                // Optional port assignments
                if( i < argc - 3 && argv[i + 3][0] != '-' ) {
                    _values.at(_section)->_remoteDataPort = atoi(argv[i + 3]);
                    if( i < argc - 4 && argv[i + 4][0] != '-' ) {
                        _values.at(_section)->_remoteCommandPort = atoi(argv[i + 4]);
                        i++;
                    } else {
                        _values.at(_section)->_remoteCommandPort = _values.at(_section)->_remoteDataPort + 1;
                    }
                    i++;
                } else {
                    _values.at(_section)->_remoteDataPort = 1720;
                    _values.at(_section)->_remoteCommandPort = _values.at(_section)->_remoteDataPort + 1;
                }

                HLog("Input NETWORK device %s:%d+%d", _values.at(_section)->_remoteServer.c_str(), _values.at(_section)->_remoteDataPort, _values.at(_section)->_remoteCommandPort);
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
                _values.at(_section)->_inputSourceDataType = IQ_INPUT_SOURCE_DATA_TYPE;
                HLog("Setting input data type to IQ");
            }
            else if( strcmp(argv[i + 1], "I") == 0 ) {
                _values.at(_section)->_inputSourceDataType = I_INPUT_SOURCE_DATA_TYPE;
                HLog("Setting input data type to I");
            }
            else if( strcmp(argv[i + 1], "Q") == 0 ) {
                _values.at(_section)->_inputSourceDataType = Q_INPUT_SOURCE_DATA_TYPE;
                HLog("Setting input data type to Q");
            }
            else if( strcmp(argv[i + 1], "REAL") == 0 ) {
                _values.at(_section)->_inputSourceDataType = REAL_INPUT_SOURCE_DATA_TYPE;
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
                _values.at(_section)->_originalInputSourceType = AUDIO_DEVICE;
                HLog("Setting input type to AUDIO_DEVICE");
            }
            else if( strcmp(argv[i + 1], "RTLSDR") == 0 ) {
                _values.at(_section)->_originalInputSourceType = RTLSDR;
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
            _values.at(_section)->_volume = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Select receiver mode ...
        if( strcmp(argv[i], "-m") == 0 && i < argc - 1) {
            if( strcmp(argv[i + 1], "CW") == 0 ) {
                _values.at(_section)->_receiverModeType = CW;
            }
            else if( strcmp(argv[i + 1], "AM") == 0 ) {
                _values.at(_section)->_receiverModeType = AM;
            }
            else if( strcmp(argv[i + 1], "AURORAL") == 0 ) {
                _values.at(_section)->_receiverModeType = AURORAL;
            }
            else if( strcmp(argv[i + 1], "SSB") == 0 ) {
                _values.at(_section)->_receiverModeType = SSB;
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
            _values.at(_section)->_reservedBuffers = atoi(argv[i + 1]);
            i++;
            continue;
        }

        // Select output device or file
        if( strcmp(argv[i], "-o") == 0 && i < argc - 1) {

            for( int pos = 0; pos < strlen(argv[i + 1]); pos++ ) {
                if( argv[i + 1][pos] < '0' || argv[i + 1][pos] > '9' ) {
                    if( argv[i + 1][pos] != '-' || pos > 0 ) {
                        _values.at(_section)->_outputFilename = argv[i + 1];
                        HLog("Output filename set to %s", _values.at(_section)->_outputFilename.c_str());
                        break;
                    }
                }
            }
            if( _values.at(_section)->_outputFilename == "" ) {
                _values.at(_section)->_outputAudioDevice = atoi(argv[i + 1]);
            }
            i++;
            continue;
        }

        // Dump input rf as ...
        if( strcmp(argv[i], "-p") == 0) {
            if( strcmp(argv[i + 1], "PCM") == 0 ) {
                _values.at(_section)->_dumpRf = true;
                _values.at(_section)->_dumpRfFileFormat = PCM;
            } else if( strcmp(argv[i + 1], "WAV") == 0 ) {
                _values.at(_section)->_dumpRf = true;
                _values.at(_section)->_dumpRfFileFormat = WAV;
            } else {
                _values.at(_section)->_dumpRf = false;
            }
            i++;
            continue;
        }

        // Samplerates
        if( strcmp(argv[i], "-dr") == 0 && i < argc - 1) {
            _values.at(_section)->_inputSampleRate = atoi(argv[i + 1]);
            HLog("Input samplerate set to %d", _values.at(_section)->_inputSampleRate);
            i++;
            continue;
        }
        if( strcmp(argv[i], "-or") == 0 && i < argc - 1) {
            _values.at(_section)->_outputSampleRate = atoi(argv[i + 1]);
            HLog("Output samplerate set to %d", _values.at(_section)->_outputSampleRate);
            i++;
            continue;
        }

        if( strcmp(argv[i], "-ro") == 0 && i < argc - 1) {
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
            _values.at(_section)->_receiverOptions[s.substr(0, pos)] = s.substr(pos + 1);
            i++;
            continue;
        }

        // RTL-SDR options
        if( strcmp(argv[i], "-rtlc") == 0 && i < argc - 1) {
            _values.at(_section)->_rtlsdrCorrection = atoi(argv[i + 1]);
            HLog("RTL-SDR frequency correction set to %d", _values.at(_section)->_rtlsdrCorrection);
            i++;
            continue;
        }
        if( strcmp(argv[i], "-rtlo") == 0 && i < argc - 1) {
            _values.at(_section)->_rtlsdrOffset = atoi(argv[i + 1]);
            HLog("RTL-SDR frequency offset set to %d", _values.at(_section)->_rtlsdrOffset);
            i++;
            continue;
        }
        if( strcmp(argv[i], "-rtla") == 0 && i < argc - 1) {
            _values.at(_section)->_rtlsdrAdjust = atoi(argv[i + 1]);
            HLog("RTL-SDR frequency adjust set to %d", _values.at(_section)->_rtlsdrAdjust);
            i++;
            continue;
        }

        // Frequency shift when used with converters
        if( strcmp(argv[i], "-shift") == 0 && i < argc - 1) {
            _values.at(_section)->_shift = atoi(argv[i + 1]);
            HLog("Frequency shift set to %d", _values.at(_section)->_shift);
            i++;
            continue;
        }

        // Decimation gain
        if( strcmp(argv[i], "-dg") == 0 && i < argc - 1) {
            _values.at(_section)->_decimatorGain = atoi(argv[i + 1]);
            HLog("Decimation gain set to %d", _values.at(_section)->_decimatorGain);
            i++;
            continue;
        }

        // Decimation AGC level
        if( strcmp(argv[i], "-dal") == 0 && i < argc - 1) {
            _values.at(_section)->_decimatorAgcLevel = atoi(argv[i + 1]);
            HLog("Decimation AGC level set to %d", _values.at(_section)->_decimatorAgcLevel);
            i++;
            continue;
        }

        // Frequency correction factor for rtl-sdr dongles
        if( strcmp(argv[i], "-rtlf") == 0 && i < argc - 1) {
            _values.at(_section)->_rtlsdrCorrectionFactor = atoi(argv[i + 1]);
            HLog("RTL-SDR frequency correction factor set to %d", _values.at(_section)->_rtlsdrCorrectionFactor);
            i++;
            continue;
        }

        // RTL-SDR gain
        if( strcmp(argv[i], "-rtlg") == 0 && i < argc - 1) {
            _values.at(_section)->_rtlsdrGain = atoi(argv[i + 1]);
            HLog("RTL-SDR gain set to %d", _values.at(_section)->_rtlsdrGain);
            i++;
            continue;
        }

        // Fir filter size
        if( strcmp(argv[i], "-ffs") == 0 && i < argc - 1) {
            _values.at(_section)->_firFilterSize = atoi(argv[i + 1]);
            if( _values.at(_section)->_firFilterSize % 2 == 0 ) {
                _values.at(_section)->_firFilterSize++;
            }
            HLog("Fir filter size set to %d", _values.at(_section)->_firFilterSize);
            i++;
            continue;
        }

        // Input filter width
        if( strcmp(argv[i], "-ifw") == 0 && i < argc - 1) {
            _values.at(_section)->_inputFilterWidth = atoi(argv[i + 1]);
            HLog("Input filter width set to %d", _values.at(_section)->_inputFilterWidth);
            i++;
            continue;
        }

        // Automatic RF gain level
        if( strcmp(argv[i], "-ral") == 0 && i < argc - 1) {
            _values.at(_section)->_rfAgcLevel = atoi(argv[i + 1]);
            HLog("Automatic RF gain level set to %d", _values.at(_section)->_rfAgcLevel);
            i++;
            continue;
        }

        // AF fft gain level
        if( strcmp(argv[i], "-afl") == 0 && i < argc - 1) {
            _values.at(_section)->_afFftAgcLevel = atoi(argv[i + 1]);
            HLog("AF FFT agc level set to  %d", _values.at(_section)->_afFftAgcLevel);
            i++;
            continue;
        }

        // Server for remote input
        if( strcmp(argv[i], "-s") == 0 && i < argc - 2) {
            _values.at(_section)->_remoteDataPort = atoi(argv[i + 1]);
            _values.at(_section)->_remoteCommandPort = atoi(argv[i + 2]);
            _values.at(_section)->_isRemoteHead = false;
            _values.at(_section)->_useRemoteHead = true;
            i += 2;
            continue;
        }

        // Scheduled start and stop
        if( strcmp(argv[i], "-b") == 0 ) {
            _values.at(_section)->_schedule.SetStart(argv[i + 1]);
            i++;
            _values.at(_section)->_schedule.SetStop(argv[i + 1]);
            i++;
            continue;
        }

        // Verbose output
        if( strcmp(argv[i], "-d") == 0 ) {
            _values.at(_section)->_verbose = true;
            continue;
        }

        // Enable frequency align mode
        if( strcmp(argv[i], "-fa") == 0 ) {
            _values.at(_section)->_frequencyAlign = true;
            _values.at(_section)->_rtlsdrAdjust = 0;
            std::cout << tr("Frequency align mode is enabled.") << std::endl;
            std::cout << std::endl;
            std::cout << tr("RTL-SDR tuning error alignment (-rtla) set to 0") << std::endl;
            std::cout << std::endl;
            std::cout << tr("An 800Hz tone is superimposed on the output audio") << std::endl;
            std::cout << tr("to help you find the offset error in your RTL-SDR") << std::endl;
            std::cout << tr("dongle. Use this value with the -rtla parameter") << std::endl;
            std::cout << tr("to enable precise tuning.") << std::endl;
            HLog("Frequency alignment mode enabled");
            continue;
        }

        // Frequency alignment volume
        if( strcmp(argv[i], "-fav") == 0 && i < argc - 1) {
            _values.at(_section)->_frequencyAlignVolume = atoi(argv[i + 1]);
            HLog("Frequency alignment volume set to %d", _values.at(_section)->_frequencyAlignVolume);
            i++;
            continue;
        }

        // Options handled in previously passes
        if( strcmp(argv[i], "-config") == 0 && i < argc - 1) {
            i++;
            continue;
        }

        // Unknown parameter
        std::cout << tr("Unknown parameter") << " '" << argv[i] << "' (" << tr("use '-h' to show the help") << ")" << std::endl;
        exit(1);
    }

    // Check configuration for remote server/head
    if( _values.at(_section)->_isRemoteHead && _values.at(_section)->_remoteServer.empty() ) {
        std::cout << tr("Please select address of remote input with '-r address port'") << std::endl;
        exit(1);
    }
    if( _values.at(_section)->_isRemoteHead && _values.at(_section)->_remoteDataPort == 0 ) {
        std::cout << tr("Please select data port of remote input with '-r address dataport commandport'") << std::endl;
        exit(1);
    }
    if( _values.at(_section)->_useRemoteHead && _values.at(_section)->_remoteDataPort == 0 ) {
        std::cout << tr("Please select data port for the input server with '-s dataport commandport'") << std::endl;
        exit(1);
    }
    if( _values.at(_section)->_isRemoteHead && _values.at(_section)->_remoteCommandPort == 0 ) {
        std::cout << tr("Please select command port of remote input with '-r address dataport commandport'") << std::endl;
        exit(1);
    }
    if( _values.at(_section)->_useRemoteHead && _values.at(_section)->_remoteCommandPort == 0 ) {
        std::cout << tr("Please select command  for the input server with '-s dataport commandport'") << std::endl;
        exit(1);
    }

    // Check that the required minimum of settings has been provided
    if( _values.at(_section)->_useRemoteHead == false ) {
        if( _values.at(_section)->_receiverModeType == NO_RECEIVE_MODE ) {
            std::cout << tr("Please select the receive mode with '-m [CW]'") << std::endl;
            exit(1);
        }
    }
    if( _values.at(_section)->_isRemoteHead == false ) {
        if( _values.at(_section)->_inputSourceType == NO_INPUT_SOURCE_TYPE ) {
            std::cout << tr("Please select the input type with '-i ...'") << std::endl;
            exit(1);
        }
        if( _values.at(_section)->_inputSourceDataType == NO_INPUT_SOURCE_DATA_TYPE ) {
            std::cout << tr("Please select the input data type with '-i ...' or explicit with '-it ...'") << std::endl;
            exit(1);
        }
        else if( _values.at(_section)->_inputSourceType == AUDIO_DEVICE && _values.at(_section)->_inputDevice < 0 ) {
            std::cout << tr("Please select the input audio device with '-i AUDIO devicenumber'") << std::endl;
            std::cout << tr("Hint: Use '-c' to get a list of audio devices on your system (Raspian may require sudo)") << std::endl;
            exit(1);
        }
        else if( _values.at(_section)->_inputSourceType == RTLSDR && _values.at(_section)->_inputDevice < 0 ) {
            std::cout << tr("Please select the input RTLSDR device with '-i RTLSDR devicenumber'") << std::endl;
            std::cout << tr("Hint: Use '-c' to get a list of audio devices on your system (Raspian may require sudo)") << std::endl;
            exit(1);
        }
        else if( _values.at(_section)->_inputSourceType == SIGNAL_GENERATOR && _values.at(_section)->_signalGeneratorFrequency < 0 ) {
            std::cout << tr("Please select the input signal generator frequency with '-i GENERATOR frequency'") << std::endl;
            exit(1);
        }
        else if( _values.at(_section)->_inputSourceType == PCM_FILE ) {

            // We need a filename
            if( _values.at(_section)->_pcmFile.empty() ) {
                std::cout << tr("Please select the input filename with '-i PCM filename'") << std::endl;
                exit(1);
            }

            // Check if the input file exists
            struct stat stats;
            if( stat(_values.at(_section)->_pcmFile.c_str(), &stats) != -1 ) {
                if( !S_ISREG(stats.st_mode) ) {
                    std::cout << "Input file does not exist" << std::endl;
                    exit(1);
                }
            }
        }
        else if( _values.at(_section)->_inputSourceType == WAV_FILE ) {

            // We need a filename
            if( _values.at(_section)->_wavFile.empty() ) {
                std::cout << tr("Please select the input filename with '-i WAV filename'") << std::endl;
                exit(1);
            }

            // Check if the input file exists
            struct stat stats;
            if( stat(_values.at(_section)->_wavFile.c_str(), &stats) != -1 ) {
                if( !S_ISREG(stats.st_mode) ) {
                    std::cout << "Input file does not exist" << std::endl;
                    exit(1);
                }
            }
        }
    }

    // Sanitize sample rate
    if( _values.at(_section)->_inputSampleRate == _values.at(_section)->_outputSampleRate && (_values.at(_section)->_inputSourceType == RTLSDR) ) {
        std::cout << "Input (device) and output sample rate should not be the same for RTL-SDR devices" << std::endl;
        exit(1);
    }
    if( _values.at(_section)->_inputSourceType != RTLSDR ) {
        _values.at(_section)->_inputSampleRate = _values.at(_section)->_outputSampleRate;
    }


    // Dump info during startup
    DumpConfigInfo();
}

void ConfigOptions::DumpConfigInfo() {
    if( !_values.at(_section)->_verbose ) {
        return;
    }

    std::cout << "Using libbooma version " << BOOMA_MAJORVERSION << "." << BOOMA_MINORVERSION << "." << BOOMA_BUILDNO << std::endl;

    // Remote/local
    if( _values.at(_section)->_useRemoteHead ) {
        std::cout << "Receiver for remote head with dataport " << _values.at(_section)->_remoteDataPort << " and commandport " << _values.at(_section)->_remoteCommandPort << std::endl;
    } else if( _values.at(_section)->_isRemoteHead) {
        std::cout << "Head for remote receiver on " << _values.at(_section)->_remoteServer << " with dataport " << _values.at(_section)->_remoteDataPort << " and commandport " << _values.at(_section)->_remoteCommandPort << std::endl;
    }

    // Input
    switch( _values.at(_section)->_inputSourceType ) {
        case NO_INPUT_SOURCE_TYPE:
            std::cout << "No input type given!" << std::endl;
            exit(1);
        case AUDIO_DEVICE:
            std::cout << "Input is audio device " << GetAudioDevice(_values.at(_section)->_inputDevice) << " running with sampling rate " << _values.at(_section)->_inputSampleRate << std::endl;
            break;
        case SIGNAL_GENERATOR:
            std::cout << "Input is a generator running at frequency " << _values.at(_section)->_signalGeneratorFrequency <<  " with sampling rate " << _values.at(_section)->_inputSampleRate << std::endl;
            break;
        case PCM_FILE:
            std::cout << "Input PCM is read from " << _values.at(_section)->_pcmFile << " assuming 16 bit signed at sampling rate " << _values.at(_section)->_inputSampleRate << std::endl;
            break;
        case WAV_FILE:
            std::cout << "Input WAV is read from " << _values.at(_section)->_wavFile << " assuming 16 bit signed at sampling rate " << _values.at(_section)->_inputSampleRate << std::endl;
            break;
        case SILENCE:
            std::cout << "Input is total silence" << std::endl;
            break;
        case NETWORK:
            switch( _values.at(_section)->_originalInputSourceType ) {
                case AUDIO_DEVICE:
                case SIGNAL_GENERATOR:
                case PCM_FILE:
                case WAV_FILE:
                case SILENCE:
                    std::cout << "Remote input is either an audio device, signalgenerator, pcm- or wavfile, or just silence." << std::endl;
                    std::cout << "Remote input is running in REAL input mode at samplerate " << _values.at(_section)->_inputSampleRate << std::endl;
                    break;
                case RTLSDR:
                    std::cout << "Remote input is an RTL-SDR dongle." << std::endl;
                    switch (_values.at(_section)->_inputSourceDataType) {
                        case IQ_INPUT_SOURCE_DATA_TYPE:
                            std::cout << "Remote input is running in IQ mode decimated to samplerate " << _values.at(_section)->_outputSampleRate << std::endl;
                            break;
                        case I_INPUT_SOURCE_DATA_TYPE:
                            std::cout << "Remote input is running in I mode decimated to samplerate " << _values.at(_section)->_outputSampleRate << std::endl;
                            break;
                        case Q_INPUT_SOURCE_DATA_TYPE:
                            std::cout << "Remote input is running in Q mode decimated to samplerate " << _values.at(_section)->_outputSampleRate << std::endl;
                            break;
                        case REAL_INPUT_SOURCE_DATA_TYPE:
                            std::cout << "Remote input is running in REAL mode decimated to samplerate " << _values.at(_section)->_outputSampleRate << std::endl;
                            break;
                        case NO_INPUT_SOURCE_DATA_TYPE:
                            std::cout << "Remote input has  no input mode specified, assuming REAL, decimated to " << _values.at(_section)->_outputSampleRate << std::endl;
                            break;
                    }
                    std::cout << "Center frequency set to " << _values.at(_section)->_frequency << " using offset " << _values.at(_section)->_rtlsdrOffset << " and correction " << _values.at(_section)->_rtlsdrCorrection << " * " << _values.at(_section)->_rtlsdrCorrectionFactor << " = " << (_values.at(_section)->_rtlsdrCorrection * _values.at(_section)->_rtlsdrCorrectionFactor) << std::endl;
                    break;
                case NETWORK:
                    std::cout << "Remote input is a remote receiver" << std::endl;
                    break;
                case NO_INPUT_SOURCE_TYPE:
                    std::cout << "No remote input type given. Receiver output will behave strangely!" << std::endl;
                    break;
            }
            break;
        case RTLSDR:
            std::cout << "Input is an RTL-SDR dongle " << GetRtlsdrDevice(_values.at(_section)->_inputDevice) << std::endl;
            switch( _values.at(_section)->_inputSourceDataType ) {
                case IQ_INPUT_SOURCE_DATA_TYPE:
                    std::cout << "Input is running in IQ mode decimating samplerate " << _values.at(_section)->_inputSampleRate << " to " << _values.at(_section)->_outputSampleRate << std::endl;
                    break;
                case I_INPUT_SOURCE_DATA_TYPE:
                    std::cout << "Input is running in I mode decimating samplerate " << _values.at(_section)->_inputSampleRate << " to " << _values.at(_section)->_outputSampleRate << std::endl;
                    break;
                case Q_INPUT_SOURCE_DATA_TYPE:
                    std::cout << "Input is running in Q mode decimating samplerate " << _values.at(_section)->_inputSampleRate << " to " << _values.at(_section)->_outputSampleRate << std::endl;
                    break;
                case REAL_INPUT_SOURCE_DATA_TYPE:
                    std::cout << "Input is running in REAL mode decimating samplerate " << _values.at(_section)->_inputSampleRate << " to " << _values.at(_section)->_outputSampleRate << std::endl;
                    break;
                case NO_INPUT_SOURCE_DATA_TYPE:
                    std::cout << "Input is running with no input mode specified, assuming REAL, decimating samplerate " << _values.at(_section)->_inputSampleRate << " to " << _values.at(_section)->_outputSampleRate << std::endl;
                    break;
            }
            std::cout << "Center frequency set to " << _values.at(_section)->_frequency << " using offset " << _values.at(_section)->_rtlsdrOffset << " and correction " << _values.at(_section)->_rtlsdrCorrection << " * " << _values.at(_section)->_rtlsdrCorrectionFactor << " = " << (_values.at(_section)->_rtlsdrCorrection * _values.at(_section)->_rtlsdrCorrectionFactor) << std::endl;
            if( _values.at(_section)->_decimatorGain == 0 ) {
                std::cout << "Decimator gain set to auto using agc with minimum output level " << _values.at(_section)->_decimatorAgcLevel << std::endl;
            } else {
                std::cout << "Decimator gain set to " << _values.at(_section)->_decimatorGain << std::endl;
            }
            std::cout << "FIR Decimator running with " << _values.at(_section)->_firFilterSize << " points and cutoff frequency " << GetDecimatorCutoff() << std::endl;
            break;
    }

    // Output
    if( !_values.at(_section)->_useRemoteHead ) {
        if (_values.at(_section)->_outputFilename == "") {
            if (_values.at(_section)->_outputAudioDevice == -1) {
                std::cout << "No output (silently discarding receiver output)" << std::endl;
            } else {
                std::cout << "Output to audio device " << GetAudioDevice(_values.at(_section)->_outputAudioDevice) << " at samplerate " << _values.at(_section)->_outputSampleRate << std::endl;
            }
        } else {
            std::cout << "Output to file " << _values.at(_section)->_outputFilename << std::endl;
        }
    }
}

ConfigOptions::~ConfigOptions() {
    WriteStoredConfig(CONFIGNAME, false);
    for( std::vector<Channel*>::iterator it = _values.at(_section)->_channels.begin(); it != _values.at(_section)->_channels.end(); it++ ) {
        delete (*it);
    }
    for( std::map<std::string, ConfigOptionValues*>::iterator it = _values.begin(); it != _values.end(); it++ ) {
        delete ((*it).second);
    }
}

void ConfigOptions::SyncStoredConfig() {
    WriteStoredConfig(CONFIGNAME, false);
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

bool ConfigOptions::ReadStoredConfig(std::string configname, bool isBookmark) {

    // Get the users homedirectory
    const char* home = std::getenv("HOME");
    if( home == NULL ) {
        HError("No HOME env. variable. Unable to read configuration");
        _values.insert(std::pair<std::string, ConfigOptionValues *>(_section, new ConfigOptionValues));
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
            _values.insert(std::pair<std::string, ConfigOptionValues *>(_section, new ConfigOptionValues));
            return false;
        }
        HLog("Config directory %s exists", configPath.c_str());
    }
    else
    {
        HLog("Config directory does not exists, no config to read");
        _values.insert(std::pair<std::string, ConfigOptionValues *>(_section, new ConfigOptionValues));
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
        HError("Failed to open config file '%s' for reading", configFile.c_str());
        _values.insert(std::pair<std::string, ConfigOptionValues *>(_section, new ConfigOptionValues));
        return false;
    }

    // Read all stored config settings
    std::string opt;
    configStream >> opt;
    while (configStream.good()) {

        // Check for section name
        if( opt[0] == '[' && opt[opt.length() - 1] == ']' ) {
            _section = opt.substr(1, opt.length() - 2);
            std::replace(_section.begin(), _section.end(), '_', ' ');
            std::replace(_section.begin(), _section.end(), '_', ' ');
            HLog("Next config section is '%s'", _section.c_str());

            if (_values.find(_section) == _values.end()) {
                _values.insert(std::pair<std::string, ConfigOptionValues *>(_section, new ConfigOptionValues));
            }

            configStream >> opt;
            continue;
        }

        // Split into name and value
        size_t splitAt = opt.find_first_of("=");
        if( splitAt != std::string::npos ) {
            std::string name = opt.substr(0, splitAt);
            std::string value = opt.substr(splitAt + 1, std::string::npos);

            // Make sure we have the default config section, if no other section name has been set
            if( name.length() > 0 && value.length() > 0 ) {
                if( _values.find(_section) == _values.end() ) {
                    _values.insert(std::pair<std::string, ConfigOptionValues *>(_section, new ConfigOptionValues));
                }
            }

            // Known config names
            HLog("config value (%s=%s)", name.c_str(), value.c_str());
            if( !isBookmark ) {
                if (name == "inputSampleRate") _values.at(_section)->_inputSampleRate = atoi(value.c_str());
                if (name == "outputSampleRate") _values.at(_section)->_outputSampleRate = atoi(value.c_str());
                if (name == "outputAudioDevice") _values.at(_section)->_outputAudioDevice = atoi(value.c_str());
                if (name == "inputSourceType") _values.at(_section)->_inputSourceType = (InputSourceType) atoi(value.c_str());
                if (name == "inputSourceDataType") _values.at(_section)->_inputSourceDataType = (InputSourceDataType) atoi(value.c_str());
                if (name == "originalInputSourceType") _values.at(_section)->_originalInputSourceType = (InputSourceType) atoi(value.c_str());
                if (name == "inputDevice") _values.at(_section)->_inputDevice = atoi(value.c_str());
                if (name == "remoteServer") _values.at(_section)->_remoteServer = value;
                if (name == "remoteDataPort") _values.at(_section)->_remoteDataPort = atoi(value.c_str());
                if (name == "remoteCommandPort") _values.at(_section)->_remoteCommandPort = atoi(value.c_str());
                if (name == "dumpRfFileFormat") _values.at(_section)->_dumpRfFileFormat = (DumpFileFormatType) atoi(value.c_str());
                if (name == "dumpAudioFileFormat") _values.at(_section)->_dumpAudioFileFormat = (DumpFileFormatType) atoi(value.c_str());
                if (name == "signalGeneratorFrequency") _values.at(_section)->_signalGeneratorFrequency = atol(value.c_str());
                if (name == "pcmFile") _values.at(_section)->_pcmFile = value;
                if (name == "wavFile") _values.at(_section)->_wavFile = value;
                if (name == "reservedBuffers") _values.at(_section)->_reservedBuffers = atoi(value.c_str());
                if (name == "rtlsdrAdjust") _values.at(_section)->_rtlsdrAdjust = atoi(value.c_str());
                if (name == "shift") _values.at(_section)->_shift = atoi(value.c_str());
                if (name == "channels") _values.at(_section)->_channels = ReadChannels(configname, value);
                if (name == "isRemoteHead") _values.at(_section)->_isRemoteHead = (value == "true" ? true : false);
            }
            if (name == "frequency") _values.at(_section)->_frequency = atoi(value.c_str());
            if (name == "receiverModeType") _values.at(_section)->_receiverModeType = (ReceiverModeType) atoi(value.c_str());
            if (name == "rfGain") _values.at(_section)->_rfGain = atoi(value.c_str());
            if (name == "rfGainEnabled") _values.at(_section)->_rfGainEnabled = (value == "true" ? true : false);
            if (name == "volume") _values.at(_section)->_volume = atoi(value.c_str());
            if (name == "receiverOptionsFor") _values.at(_section)->_receiverOptionsFor = ReadStoredReceiverOptionsFor(value);
            if (name == "preamp") _values.at(_section)->_preamp = atoi(value.c_str());
            if (name == "activeSection" && value == "true") _activeSection = _section;
        }
        configStream >> opt;
    }

    // Done reading the config file
    _section = _activeSection;
    configStream.close();

    // Read a stored config
    return true;
}

void ConfigOptions::WriteStoredConfig(std::string configname, bool isBookmark) {

    // If in frequency alignment mode, prevent writing the settings
    if( _values.at(_section)->_frequencyAlign ) {
        std::cout << "Running in frequency alignment mode. Config changes is not saved" << std::endl;
        HLog("Discarding config changes due to frequency alignment mode");
        return;
    }

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

    // Run through all config sections
    for( std::map<std::string, ConfigOptionValues*>::iterator it = _values.begin(); it != _values.end(); it++ ) {

        // Section name
        std::string name = (*it).first;
        std::replace(name.begin(), name.end(), ' ', '_');
        configStream << "[" << name << "]" << std::endl;

        // Write config settings
        if (!isBookmark) {
            configStream << "inputSampleRate=" << _values.at((*it).first)->_inputSampleRate << std::endl;
            configStream << "outputSampleRate=" << _values.at((*it).first)->_outputSampleRate << std::endl;
            configStream << "outputAudioDevice=" << _values.at((*it).first)->_outputAudioDevice << std::endl;
            configStream << "inputSourceType=" << _values.at((*it).first)->_inputSourceType << std::endl;
            configStream << "inputSourceDataType=" << _values.at((*it).first)->_inputSourceDataType << std::endl;
            configStream << "originalInputSourceType=" << _values.at((*it).first)->_originalInputSourceType << std::endl;
            configStream << "inputDevice=" << _values.at((*it).first)->_inputDevice << std::endl;
            configStream << "remoteServer=" << _values.at((*it).first)->_remoteServer << std::endl;
            configStream << "remoteDataPort=" << _values.at((*it).first)->_remoteDataPort << std::endl;
            configStream << "remoteCommandPort=" << _values.at((*it).first)->_remoteCommandPort << std::endl;
            configStream << "dumpRfFileFormat=" << _values.at((*it).first)->_dumpRfFileFormat << std::endl;
            configStream << "dumpAudioFileFormat=" << _values.at((*it).first)->_dumpAudioFileFormat << std::endl;
            configStream << "signalGeneratorFrequency=" << _values.at((*it).first)->_signalGeneratorFrequency << std::endl;
            configStream << "pcmFile=" << _values.at((*it).first)->_pcmFile << std::endl;
            configStream << "wavFile=" << _values.at((*it).first)->_wavFile << std::endl;
            configStream << "reservedBuffers=" << _values.at((*it).first)->_reservedBuffers << std::endl;
            configStream << "rtlsdrAdjust=" << _values.at((*it).first)->_rtlsdrAdjust << std::endl;
            configStream << "shift=" << _values.at((*it).first)->_shift << std::endl;
            configStream << "channels=" << WriteChannels(configname, (*it).first, _values.at(_section)->_channels) << std::endl;
            configStream << "isRemoteHead=" << (_values.at((*it).first)->_isRemoteHead ? "true" : "false") << std::endl;
        }
        configStream << "frequency=" << _values.at((*it).first)->_frequency << std::endl;
        configStream << "receiverModeType=" << _values.at((*it).first)->_receiverModeType << std::endl;
        configStream << "rfGain=" << _values.at((*it).first)->_rfGain << std::endl;
        configStream << "rfGainEnabled=" << (_values.at((*it).first)->_rfGainEnabled ? "true" : "false")  << std::endl;
        configStream << "volume=" << _values.at((*it).first)->_volume << std::endl;
        configStream << "receiverOptionsFor=" << WriteStoredReceiverOptionsFor(_values.at((*it).first)->_receiverOptionsFor) << std::endl;
        configStream << "preamp=" << _values.at((*it).first)->_preamp << std::endl;
        configStream << "activeSection=" << (_section == (*it).first ? "true" : "false") << std::endl;
        configStream << std::endl;
    }

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
    std::map<std::string, std::map<std::string, std::string>>::iterator it = _values.at(_section)->_receiverOptionsFor.find(receiver);
    if( it != _values.at(_section)->_receiverOptionsFor.end() ) {
        HLog("Has existing options map for receiver %s", receiver.c_str());
        it->second = options;
        return;
    }
    HLog("Adding options map for receiver %s", receiver.c_str());
    _values.at(_section)->_receiverOptionsFor[receiver] = options;
}

std::map<std::string, std::string> ConfigOptions::GetReceiverOptionsFor(std::string receiver) {
    std::map<std::string, std::map<std::string, std::string>>::iterator it = _values.at(_section)->_receiverOptionsFor.find(receiver);
    if( it != _values.at(_section)->_receiverOptionsFor.end() ) {
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

std::string ConfigOptions::WriteChannels(std::string configname, std::string section, std::vector<Channel*> channels) {
    std::string list = "";

    // Get the users homedirectory
    const char* home = std::getenv("HOME");
    if( home == NULL ) {
        HError("No HOME env. variable. Unable to save configuration");
        return "";
    }

    // Compose the config path
    std::string configPath(home);
    configPath += "/.booma";

    // Compose path to config file's channel list (persistent)
    std::string configFile(configPath);
    configFile += "/" + configname + "." + section + ".channels";
    HLog("Config file's persistent channel file is %s", configFile.c_str());

    // Open the config file
    std::ofstream configStream;
    configStream.open(configFile);
    if( !configStream.is_open() ) {
        HError("Failed to open config file for writing");
        return "";
    }

    for( std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); it++ ) {
        list += (*it)->GetDefinition() + ",";
        configStream << (*it)->GetDefinition() << std::endl;
    }

    // Close the channels file stream
    configStream.close();

    return list;
}

std::vector<Channel*> ConfigOptions::ReadChannels(std::string configname, std::string channels) {

    std::vector<Channel*> list;

    std::istringstream stream(channels);
    std::string definition;
    while( std::getline(stream, definition, ',') ) {
        if( definition.length() > 0 ) {
            list.push_back(new Channel(definition));
        }
    }
    std::sort(list.begin(), list.end(), Channel::ChannelComparator());

    return list;
}

std::vector<Channel*> ConfigOptions::ReadPersistentChannels(std::string configname, std::string section) {

    std::vector<Channel *> list;

    // Get the users homedirectory
    const char *home = std::getenv("HOME");
    if (home == NULL) {
        HError("No HOME env. variable. Unable to save configuration");
        return list;
    }

    // Compose the config path
    std::string configPath(home);
    configPath += "/.booma";

    // Compose path to config file's channel list (persistent)
    std::string configFile(configPath);
    configFile += "/" + configname + "." + section + ".channels";
    HLog("Config file's persistent channel file is %s", configFile.c_str());

    std::ifstream configStream;
    configStream.open(configFile, std::ifstream::in);
    if (!configStream.is_open()) {
        std::cout << "No persistent channels file found, adding default channels" << std::endl;
        AddDefaultChannels(&list);
        return list;
    }

    // Read channels
    std::string opt;
    configStream >> opt;
    while (configStream.good()) {
        if (opt.length() > 0) {
            list.push_back(new Channel(opt));
        }
        configStream >> opt;
    }

    // Close the channels file stream
    configStream.close();
    if (!list.empty()) {
        std::cout << "Restored channels from persistent channels file" << std::endl;
    } else {
        std::cout << "Persistent channels file was empty. Adding default channels\n";
        AddDefaultChannels(&list);
    }

    return list;
}

void ConfigOptions::AddDefaultChannels(std::vector<Channel *>* list) {

    // VLF transmitters
    list->push_back(new Channel("JXN Novik. Norway", 16400));
    list->push_back(new Channel("SAQ Grimeton. Sweeden", 17200));
    list->push_back(new Channel("RDL ???. Russia", 18100));
    list->push_back(new Channel("GBZ Anthorn. UK", 19580));
    list->push_back(new Channel("NWC Harold E. Holt. North West Cape. Exmouth. Australia", 19800));
    list->push_back(new Channel("ICV Isola di Tavolara. Italy", 20270));
    list->push_back(new Channel("FTA Sainte-Assise. France", 20900));
    list->push_back(new Channel("HWU Rosnay. France", 21750));
    list->push_back(new Channel("GQD Skelton. UK", 22100));
    list->push_back(new Channel("DHO38 Rhauderfehn. Germany", 23400));
    list->push_back(new Channel("NAA Cutler. ME. USA", 24000));
    list->push_back(new Channel("unid25 Mokpo. South Korea", 25000));
    list->push_back(new Channel("NML La Moure. ND. USA", 25200));
    list->push_back(new Channel("TBB Bafa. Turkey", 26700));
    list->push_back(new Channel("(unknown 1)", 30700));
    list->push_back(new Channel("NRK/TFK Grindavik. Iceland", 37500));
    list->push_back(new Channel("SRC Varberg. Sweden", 40400));
    list->push_back(new Channel("(unknown 2)", 44270));
    list->push_back(new Channel("NSY Niscemi. Italy", 45900));
    list->push_back(new Channel("SXA Marathon. Greece", 49000));
    list->push_back(new Channel("GYW1 Crimond. UK", 51950));
    list->push_back(new Channel("MSF Anthorn. UK", 60000));
    list->push_back(new Channel("FUG La Régine. France", 62600));
    list->push_back(new Channel("FUE Kerlouan. France", 65800));
    list->push_back(new Channel("RBU Moscow/Taldom. Russia", 66666));
    list->push_back(new Channel("CFH Halifax. Canada", 73600));
    list->push_back(new Channel("DCF77 Mainflingen. Germany", 77500));
    list->push_back(new Channel("GYN2 Inskip. UK", 81000));
    list->push_back(new Channel("LORAN", 100000));
    list->push_back(new Channel("EFR Teleswitch ainflingen. Germany", 129100));
    list->push_back(new Channel("EFR Teleswitch Lakihegy. Hungary", 135600));
    list->push_back(new Channel("EFR Teleswitch Burg. Germany", 139000));
    list->push_back(new Channel("DDH47 DWD Pinneberg. Germany", 147300));

    // Longwave Am transmitters
    list->push_back(new Channel("ROU SRR Radio Antena Satelor. Hungary (AM)", 153000));
    list->push_back(new Channel("TDF Timesignal  Allouis. France (AM)", 162000));
    list->push_back(new Channel("BBC Radio 4. UK (AM)", 198000));
    list->push_back(new Channel("POL Polskie Radio Jedynka. Poland (AM)", 225000));
    list->push_back(new Channel("LUX RTL (AM/DRM). Luxemburg ", 234000));
    list->push_back(new Channel("DNK DR Langboelge. Denmark (AM)", 243000));
    list->push_back(new Channel("IRL RTE Radio 1. Ireland (AM)", 252000));
    list->push_back(new Channel("CZE CRo Radiozurnal. czech republic (AM)", 270000));
    list->push_back(new Channel("Radio 208 Ishoej. Denmark (AM)", 1440000));
    list->push_back(new Channel("Radio OZ-Viola Hilleroed. Denmark (AM)", 5980000));

    // RTTY or other small bandwidth signals
    list->push_back(new Channel("??? ???. ???", 290000));
    list->push_back(new Channel("??? ???. ???", 298500));

    // NDB
    list->push_back(new Channel("RK NDB Roskilde. Denmark (beacon)", 368000));
    list->push_back(new Channel("RK NDB Roskilde. Denmark (id)", 368400));
    list->push_back(new Channel("LB NDB Angelholm. Sweeden (beacon)", 370500));
    list->push_back(new Channel("LB NDB Angelholm. Sweeden (id)", 370900));

    // Aviation services
    list->push_back(new Channel("VOLMET Shannon. Ireland (AM)", 3413000));
    list->push_back(new Channel("VOLMET Kastrup. Denmark (AM)", 127000000));

    // Ham radio beacons
    list->push_back(new Channel("OZ7IGY 6m Jystrup. Denmark (USB-CW)", 50470250));
    list->push_back(new Channel("OZ7IGY 2m Jystrup. Denmark (USB-CW)", 144470250));
}

void ConfigOptions::WriteBookmark(std::string name) {
    WriteStoredConfig(name + ".bookmark", true);
}

bool ConfigOptions::ReadBookmark(std::string name) {
    return ReadStoredConfig(name + ".bookmark", true);
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

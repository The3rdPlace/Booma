#ifndef __RECEIVER_H
#define __RECEIVER_H

#include <hardtapi.h>
#include "configoptions.h"
#include "boomainput.h"
#include "boomadecoder.h"
#include "option.h"

#include "boomareceiverexception.h"

class BoomaReceiver {

    private:

        HWriterConsumer<int16_t>* _preProcess;
        HWriterConsumer<int16_t>* _receive;
        HWriterConsumer<int16_t>* _postProcess;
        HSplitter<int16_t>* _spectrum;
        HSplitter<int16_t>* _decoder;

        std::vector<Option> _options;

        int _frequency;
        int _inputSamplerate;
        int _outputSamplerate;

        bool _hasBuilded;

        bool SetOption(ConfigOptions* opts, std::string name, int value){
            for( std::vector<Option>::iterator it = _options.begin(); it != _options.end(); it++ ) {
                if( (*it).Name == name ) {
                    for( std::vector<OptionValue>::iterator valIt = (*it).Values.begin(); valIt != (*it).Values.end(); valIt++ ) {
                        if( (*valIt).Value == value ) {

                            // Discard repeated setting of the same value
                            if( (*it).CurrentValue == value ) {
                                HLog("Value is the same as the current value, discarding option set");
                                return true;
                            }

                            // Set current value
                            (*it).CurrentValue = value;

                            // Update the stored copy of receiver options
                            std::map<std::string, std::string> optionsMap;
                            for( std::vector<Option>::iterator optIt = _options.begin(); optIt != _options.end(); optIt++ ) {
                                for( std::vector<OptionValue>::iterator optValIt = (*optIt).Values.begin(); optValIt != (*optIt).Values.end(); optValIt++ ) {
                                    if( (*optValIt).Value == (*optIt).CurrentValue ) {
                                        optionsMap[(*optIt).Name] = (*optValIt).Name;
                                        continue;
                                    }
                                }
                            }
                            opts->SetReceiverOptionsFor(GetName(), optionsMap);

                            // Report the change to the receiver implementation
                            if( _hasBuilded ) {
                                OptionChanged(name, value);
                            }

                            // Receiver option set
                            return true;
                        }
                    }
                    HError("Attempt to internal set receiver option '%s' to non-existing value %d", name.c_str(), value);
                    return false;
                }
            }
            HError("Attempt to internal set non-existing receiver option '%s' to value '%d'", name.c_str(), value);
            return false;
        };

    protected:

        virtual std::string GetName() = 0;

        virtual HWriterConsumer<int16_t>* PreProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) = 0;
        virtual HWriterConsumer<int16_t>* Receive(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) = 0;
        virtual HWriterConsumer<int16_t>* PostProcess(ConfigOptions* opts, HWriterConsumer<int16_t>* previous) = 0;

        virtual bool IsDataTypeSupported(InputSourceDataType datatype) = 0;

        void RegisterOption(Option option) {
            HLog("Registering option %s", option.Name.c_str());
            _options.push_back(option);
        }

        virtual void OptionChanged(std::string name, int value) = 0;

        virtual bool SetFrequency(int frequency) = 0;

    public:

        BoomaReceiver(ConfigOptions* opts, int initialFrequency):
            _hasBuilded(false),
            _frequency(initialFrequency) {

            HLog("Creating BoomaReceiver with initial frequency %d", _frequency);
            _inputSamplerate = opts->GetInputSampleRate();
            _outputSamplerate = opts->GetOutputSampleRate();
        }

        virtual ~BoomaReceiver() {
            delete _spectrum;
        }

        std::vector<Option>* GetOptions() {
            return &_options;
        }

        int GetOption(std::string name) {
            for( std::vector<Option>::iterator it = _options.begin(); it != _options.end(); it++ ) {
                if( (*it).Name == name ) {
                    return (*it).CurrentValue;
                }
            }
            return -1;
        };

        bool SetOption(ConfigOptions* opts, std::string name, std::string value){
            HLog("Setting option '%s' to value '%s'", name.c_str(), value.c_str());
            for( std::vector<Option>::iterator it = _options.begin(); it != _options.end(); it++ ) {
                if( (*it).Name == name ) {
                    for( std::vector<OptionValue>::iterator valIt = (*it).Values.begin(); valIt != (*it).Values.end(); valIt++ ) {
                        if( (*valIt).Name == value ) {
                            return SetOption(opts, name, (*valIt).Value);
                        }
                    }
                    HError("Attempt to set receiver option '%s' to non-existing value '%s'", name.c_str(), value.c_str());
                    return false;
                }
            }
            HError("Attempt to set non-existing receiver option '%s' to value '%s'", name.c_str(), value.c_str());
            return false;
        };

        virtual std::string GetOptionInfoString() = 0;

        void Build(ConfigOptions* opts, BoomaInput* input, BoomaDecoder* decoder = NULL) {

            // Can we build a receiver for the given input data type ?
            if( !IsDataTypeSupported(opts->GetInputSourceDataType()) ) {
                HError("Attempt to build receiver for unsupported input data type");
                _hasBuilded = false;
                throw new BoomaReceiverException("Attempt to build receiver for unsupported input data type");
            }

            // Set options from saved configuration
            std::map<std::string, std::string> storedOptions = opts->GetReceiverOptionsFor(GetName());
            for( std::map<std::string, std::string>::iterator it = storedOptions.begin(); it != storedOptions.end(); it++ ) {
                SetOption(opts, (*it).first, (*it).second);
            }

            // Set options from the command line
            for( std::map<std::string, std::string>::iterator it = opts->GetReceiverOptions()->begin(); it != opts->GetReceiverOptions()->end(); it++ ) {
                SetOption(opts, (*it).first, (*it).second);
            }

            // Add preprocessing part of the receiver
            _preProcess = PreProcess(opts, input->GetLastWriterConsumer());

            // Add a splitter so that we can take the full spectrum out before running through the receiver filters
            _spectrum = new HSplitter<int16_t>(_preProcess);

            // Add the receiver chain
            _receive = Receive(opts, _spectrum->Consumer());

            // Add postprocessing part of the receiver
            _postProcess = PostProcess(opts, _receive);

            // Add a splitter so that we can push fully processed samples through an optional decoder
            _decoder = new HSplitter<int16_t>(_postProcess);
            if( decoder != NULL ) {
                _decoder->SetWriter(decoder->Writer());
            }

            // Receiver has been build and all components is initialized (or so they should be!)
            _hasBuilded = true;
        };

        bool SetFrequency(ConfigOptions* opts, int frequency) {
            _frequency = frequency;
            return SetFrequency(_frequency);
        }

        int GetFrequency() {
            return _frequency;
        }

        int GetInputSamplerate() {
            return _inputSamplerate;
        }

        int GetOutputSamplerate() {
            return _outputSamplerate;
        }

        HWriterConsumer<int16_t>* GetLastWriterConsumer() {
            return _decoder->Consumer();
        }

        HWriterConsumer<int16_t>* GetSpectrumConsumer() {
            return _spectrum->Consumer();
        }
};

#endif

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

        // Major receiver hooks
        HWriterConsumer<int16_t>* _preProcess;
        HWriterConsumer<int16_t>* _receive;
        HWriterConsumer<int16_t>* _postProcess;
        HSplitter<int16_t>* _spectrum;
        HSplitter<int16_t>* _decoder;

        // RF spectrum reporting
        HFftOutput<int16_t>* _rfFft;
        HCustomWriter<HFftResults>* _rfFftWriter;
        int RfFftCallback(HFftResults* result, size_t length);
        HRectangularWindow<int16_t>* _rfFftWindow;
        double* _rfSpectrum;
        int _rfFftSize;

        // Audio spectrum reporting
        HFftOutput<int16_t>* _audioFft;
        HCustomWriter<HFftResults>* _audioFftWriter;
        int AudioFftCallback(HFftResults* result, size_t length);
        HRectangularWindow<int16_t>* _audioFftWindow;
        double* _audioSpectrum;
        int _audioFftSize;

        // Decimation
        bool _decimate;
        HIqFirDecimator<int16_t>* _iqFirDecimator;
        HIqFirFilter<int16_t>* _iqFirFilter;
        HIqDecimator<int16_t>* _iqDecimator;
        HFirDecimator<int16_t>* _firDecimator;
        HFirFilter<int16_t>* _firFilter;
        HDecimator<int16_t>* _decimator;

        // Probes
        HProbe<int16_t>* _iqFirDecimatorProbe;
        HProbe<int16_t>* _iqFirFilterProbe;
        HProbe<int16_t>* _iqDecimatorProbe;
        HProbe<int16_t>* _firDecimatorProbe;
        HProbe<int16_t>* _firFilterProbe;
        HProbe<int16_t>* _decimatorProbe;

        std::vector<Option> _options;

        int _cutOff;
        int _frequency;

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
                                OptionChanged(opts, name, value);
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

        HWriterConsumer<int16_t>* Decimate(ConfigOptions* opts, HWriterConsumer<int16_t>* previous);

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

        virtual void OptionChanged(ConfigOptions* opts, std::string name, int value) = 0;

        virtual bool SetInternalFrequency(ConfigOptions* opts, int frequency) = 0;

        /**
         * Base class for all Booma receivers
         *
         * @param opts Receiver options
         * @param initialFrequency The initial frequency being received on
         * @param cutOff If decimating and if cutOff is not 0 (zero), then this is used
         *               as the upper cutoff frequency in the lowpass filter being applied
         *               by the FIR decimator, otherwise the highest frequency supported by
         *               the output samplerate will be used as cutoff frequency.
         * @param decimate If true then the incomming samples will be decimated
         *                 from the 'device rate to 'output rate. The samples will be filtered
         *                 to the range [-cutOff; cutOff]
         *
         */
        BoomaReceiver(ConfigOptions* opts, int initialFrequency, int cutOff, bool decimate = false):
            _hasBuilded(false),
            _frequency(initialFrequency),
            _cutOff(cutOff),
            _spectrum(nullptr),
            _iqFirDecimator(nullptr),
            _iqFirFilter(nullptr),
            _iqDecimator(nullptr),
            _firDecimator(nullptr),
            _firFilter(nullptr),
            _decimator(nullptr),
            _iqFirDecimatorProbe(nullptr),
            _iqFirFilterProbe(nullptr),
            _iqDecimatorProbe(nullptr),
            _firDecimatorProbe(nullptr),
            _firFilterProbe(nullptr),
            _decimatorProbe(nullptr),
            _decimate(decimate),
            _rfFft(nullptr),
            _rfFftWindow(nullptr),
            _rfFftWriter(nullptr),
            _rfSpectrum(nullptr),
            _rfFftSize(256),
            _audioFft(nullptr),
            _audioFftWindow(nullptr),
            _audioFftWriter(nullptr),
            _audioSpectrum(nullptr),
            _audioFftSize(256) {

            HLog("Creating BoomaReceiver with initial frequency %d", _frequency);
            //_inputSamplerate = opts->GetInputSampleRate();
            //_outputSamplerate = opts->GetOutputSampleRate();
        }

        bool GetDecimationRate(int inputRate, int outputRate, int* first, int* second);

    public:

        virtual ~BoomaReceiver() {
            SAFE_DELETE(_spectrum);

            SAFE_DELETE(_iqFirDecimator);
            SAFE_DELETE(_iqFirFilter);
            SAFE_DELETE(_iqDecimator);
            SAFE_DELETE(_firDecimator);
            SAFE_DELETE(_firFilter);
            SAFE_DELETE(_decimator);

            SAFE_DELETE(_iqFirDecimatorProbe);
            SAFE_DELETE(_iqFirFilterProbe);
            SAFE_DELETE(_iqDecimatorProbe);
            SAFE_DELETE(_firDecimatorProbe);
            SAFE_DELETE(_firFilterProbe);
            SAFE_DELETE(_decimatorProbe);

            SAFE_DELETE(_rfFft);
            SAFE_DELETE(_rfFftWriter);
            SAFE_DELETE(_rfFftWindow);
            SAFE_DELETE(_rfSpectrum);

            SAFE_DELETE(_audioFft);
            SAFE_DELETE(_audioFftWriter);
            SAFE_DELETE(_audioFftWindow);
            SAFE_DELETE(_audioSpectrum);
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

            // Add a splitter so that we can take the full spectrum out before running through the receiver filters
            _spectrum = new HSplitter<int16_t>(input->GetLastWriterConsumer());

            // Add RF spectrum calculation
            _rfFftWindow = new HRectangularWindow<int16_t>();
            _rfFft = new HFftOutput<int16_t>(_rfFftSize, RFFFT_AVERAGING_COUNT, RFFFT_SKIP, _spectrum->Consumer(), _rfFftWindow);
            _rfFftWriter = HCustomWriter<HFftResults>::Create<BoomaReceiver>(this, &BoomaReceiver::RfFftCallback, _rfFft->Consumer());
            _rfSpectrum = new double[_rfFftSize / 2];
            memset((void*) _rfSpectrum, 0, sizeof(double) * _rfFftSize / 2);

            // Apply decimation - if requested
            HWriterConsumer<int16_t>* decimatedConsumer = Decimate(opts, _spectrum);

            // Add preprocessing part of the receiver
            _preProcess = PreProcess(opts, decimatedConsumer);

            // Add the receiver chain
            _receive = Receive(opts, _preProcess);

            // Add postprocessing part of the receiver
            _postProcess = PostProcess(opts, _receive);

            // Add a splitter so that we can push fully processed samples through an optional decoder
            _decoder = new HSplitter<int16_t>(_postProcess);
            if( decoder != NULL ) {
                _decoder->SetWriter(decoder->Writer());
            }

            // Add audio spectrum calculation
            _audioFftWindow = new HRectangularWindow<int16_t>();
            _audioFft = new HFftOutput<int16_t>(_audioFftSize, AUDIOFFT_AVERAGING_COUNT, AUDIOFFT_SKIP, _decoder->Consumer(), _audioFftWindow);
            _audioFftWriter = HCustomWriter<HFftResults>::Create<BoomaReceiver>(this, &BoomaReceiver::AudioFftCallback, _audioFft->Consumer());
            _audioSpectrum = new double[_audioFftSize / 2];
            memset((void*) _audioSpectrum, 0, sizeof(double) * _audioFftSize / 2);

            // Receiver has been build and all components is initialized (or so they should be!)
            _hasBuilded = true;
        };

        bool SetFrequency(ConfigOptions* opts, int frequency) {
            _frequency = frequency;
            return SetInternalFrequency(opts, _frequency);
        }

        int GetFrequency() {
            return _frequency;
        }

        HWriterConsumer<int16_t>* GetLastWriterConsumer() {
            return _decoder->Consumer();
        }

        HWriterConsumer<int16_t>* GetSpectrumConsumer() {
            return _spectrum->Consumer();
        }

        int GetRfSpectrum(double* spectrum);
        int GetRfFftSize();
        int GetAudioFftSize();
        int GetAudioSpectrum(double* spectrum);
};

#endif

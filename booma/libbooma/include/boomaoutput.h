#ifndef __OUTPUT_H
#define __OUTPUT_H

#include <hardtapi.h>
#include "configoptions.h"
#include "boomareceiver.h"

class BoomaOutput {

    private:

        //HGain<int16_t>* _outputWriter;
        HSoundcardWriter<int16_t>* _soundcardWriter;
        HNullWriter<int16_t>* _nullWriter;
        HAgc<int16_t>* _outputWriter;

        // Splitting audio and RF
        HWriter<int16_t>* _audioWriter;
        HSplitter<int16_t>* _audioSplitter;
        HBreaker<int16_t>* _audioBreaker;
        HBufferedWriter<int16_t>* _audioBuffer;

        // Signal level reporting
        HSignalLevelOutput<int16_t>* _signalLevel;
        HCustomWriter<HSignalLevelResult>* _signalLevelWriter;
        int SignalLevelCallback(HSignalLevelResult* result, size_t length);
        int _signalStrength;
        double _signalSum;

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

    public:

        BoomaOutput(ConfigOptions* opts, BoomaReceiver* receiver);
        ~BoomaOutput();

        bool SetDumpAudio(bool enabled);
        int SetVolume(int volume);

        int GetSignalLevel();
        int GetSignalSum();
        int GetRfFftSize();
        int GetRfSpectrum(double* spectrum);
        int GetAudioFftSize();
        int GetAudioSpectrum(double* spectrum);
};

#endif
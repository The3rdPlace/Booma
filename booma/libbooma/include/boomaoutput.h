#ifndef __OUTPUT_H
#define __OUTPUT_H

#include <hardtapi.h>
#include "configoptions.h"
#include "boomareceiver.h"

class BoomaOutput {

    private:

        HSoundcardWriter<int16_t>* _soundcardWriter;
        HNullWriter<int16_t>* _nullWriter;
        HAgc<int16_t>* _outputAgc;
        HGain<int16_t>* _outputVolume;
        HBiQuadFilter<HLowpassBiQuad<int16_t>, int16_t>* _loFilter1;
        HBiQuadFilter<HNotchBiQuad<int16_t>, int16_t>* _loFilter2;

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

        // Probes
        HProbe<int16_t>* _outputAgcProbe;
        HProbe<int16_t>* _outputVolumeProbe;
        HProbe<int16_t>* _loFilterProbe1;
        HProbe<int16_t>* _loFilterProbe2;
        HProbe<int16_t>* _loFilterProbe;

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

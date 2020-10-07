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
        HFirFilter<int16_t>* _outputFilter;

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

        // Probes
        HProbe<int16_t>* _outputAgcProbe;
        HProbe<int16_t>* _outputVolumeProbe;
        HProbe<int16_t>* _outputFilterProbe;

    public:

        BoomaOutput(ConfigOptions* opts, BoomaReceiver* receiver);
        ~BoomaOutput();

        bool SetDumpAudio(bool enabled);
        int SetVolume(int volume);

        int GetSignalLevel();
        int GetSignalSum();
};

#endif

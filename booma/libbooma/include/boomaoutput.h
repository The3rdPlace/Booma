#ifndef __OUTPUT_H
#define __OUTPUT_H

#include <hardtapi.h>
#include "configoptions.h"
#include "boomareceiver.h"

class BoomaOutput {

    private:

        // Output
        HMux<int16_t>* _soundcardMultiplexer;
        HSoundcardWriter<int16_t>* _soundcardWriter;
        HNullWriter<int16_t>* _nullWriter;
        HFileWriter<int16_t>* _pcmWriter;
        HWavWriter<int16_t>* _wavWriter;
        HGain<int16_t>* _outputVolume;
        HFirFilter<int16_t>* _outputFilter;

        // Splitting audio and RF
        HWriter<int16_t>* _audioWriter;
        HSplitter<int16_t>* _audioSplitter;
        HBreaker<int16_t>* _audioBreaker;
        HBufferedWriter<int16_t>* _audioBuffer;

        // Signal level reporting
        HSplitter<int16_t>* _ifSplitter;
        HSignalLevelOutput<int16_t>* _signalLevel;
        HCustomWriter<HSignalLevelResult>* _signalLevelWriter;
        int SignalLevelCallback(HSignalLevelResult* result, size_t length);
        int _signalStrength;
        int _signalMax;
        double _signalSum;

        // Probes
        HProbe<int16_t>* _outputVolumeProbe;
        HProbe<int16_t>* _outputFilterProbe;

        // Frequency alignment
        HSineGenerator<int16_t>* _frequencyAlignmentGenerator;
        HLinearMixer<int16_t>* _frequencyAlignmentMixer;
        HWriterConsumer<int16_t>* GetOutputVolumeConsumer() {
            return _frequencyAlignmentMixer == nullptr
                    ? _outputVolume->Consumer()
                    : _frequencyAlignmentMixer->Consumer();
        }

        // Output filter
        int _outputFilterWidth;

        bool IsWav(std::string filename);

    public:

        BoomaOutput(ConfigOptions* opts, BoomaReceiver* receiver);
        ~BoomaOutput();

        bool SetDumpAudio(bool enabled);
        int SetVolume(int volume);

        int GetSignalLevel();
        int GetSignalSum();
        int GetSignalMax();

        int GetOutputFilterWidth() {
            return _outputFilterWidth;
        }
};

#endif

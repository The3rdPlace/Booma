#include <hardtapi.h>

#include <booma.h>
#include "internals.h"


HGain<int16_t>* gain;

bool CreateCwReceiverChain(ConfigOptions* configOptions, HWriterConsumer<int16_t>* previous, HWriter<int16_t>* next) {

    HLog("Creating CW receiver chain");

    // Set mixer (multiplier local oscillator input frequency). This could also be 18.240 which
    // would give more or less the same result - allthough the highpass filter should be adjusted
    // then - otherwise the 17.200 signal we are looking for will be reduced.
    const int LOCAL_OSCILATOR = configOptions->Frequency - 1024;

    // -------------------------------------------------------------------------------------
    // Setup dsp chain for readers - first to last

    // Create a wave-file reader, reading a local file with a prerecorded saq transmission.
    // We know that the recording is 16 bit signed, 48KSps, mono
    //HWavReader<int16_t> input(argv[1]);

    // Add hum filter to remove 50Hz harmonics and the very lowest part of the spectrum (incl. 50Hz)
    // These components, which have very high levels, will completely botch the rest of the chain
    // if allowed through (50Hz input is here a.o., with an insanely high level)
    HHumFilter<int16_t> humFilter(previous, H_SAMPLE_RATE_48K, 50, 500, BLOCKSIZE);

    // Increase signal strength after mixing to avoid losses before filtering and mixing
    HGain<int16_t> gain(humFilter.Consumer(), configOptions->FirstStageGain, BLOCKSIZE);

    // Highpass filter before mixing to remove some of the lowest frequencies that may
    // get mirrored back into the final frequency range and cause (more) distortion.
    // (In this receiver, the results are good when the cutoff frequency is located at the local oscillator frequency)
    HBiQuadFilter<HHighpassBiQuad<int16_t>, int16_t> highpass(gain.Consumer(), LOCAL_OSCILATOR, H_SAMPLE_RATE_48K, 0.7071f, 1, BLOCKSIZE);

    // Mix down to the output frequency.
    // 17200Hz - 16160Hz = 1040Hz  (place it somewhere inside the bandpass filter pass region)
    HMultiplier<int16_t> multiplier(highpass.Consumer(), H_SAMPLE_RATE_48K, LOCAL_OSCILATOR, BLOCKSIZE);

    // Narrow butterworth bandpass filter, bandwidth 100Hz around 1000-1100. 4th. order, 4 biquads cascaded
    // Removes (almost) anything but the mixed down signal from SAQ (Grimeton)
    //
    // Designed using http://www.micromodeler.com/dsp/
    //
    // Do not forget that the a1 and a2 coefficients should be multiplied by -1 in the input coefficients
    // (thus saving some cycles in the filter)
    float coeffs[] =
    {
        0.06053979315740952, -0.12107958631481903, 0.06053979315740952, 1.9701579350811518, -0.9881958184253727,// b0, b1, b2, -a1, -a2
        0.125, -0.25, 0.125, 1.9780280925054692, -0.9952212910209018,// b0, b1, b2, -a1, -a2
        0.00048828125, 0.0009765625, 0.00048828125, 1.9683639531082289, -0.9877622267827567,// b0, b1, b2, -a1, -a2
        0.00048828125, 0.0009765625, 0.00048828125, 1.9742906058109615, -0.9947853486870636// b0, b1, b2, -a1, -a2
    };
    HCascadedBiQuadFilter<int16_t> bandpass(multiplier.Consumer(), coeffs, 20, BLOCKSIZE);

    // General lowpass filtering after mixing down to IF
    HBiQuadFilter<HLowpassBiQuad<int16_t>, int16_t> lowpass(bandpass.Consumer(), 2000, H_SAMPLE_RATE_48K, 0.7071f, 1, BLOCKSIZE);

    // Final boost of signal (output volume)
    HGain<int16_t> volume(lowpass.Consumer(), configOptions->Volume, BLOCKSIZE);

    // -------------------------------------------------------------------------------------
    // Setup dsp chain for writers - first to last

    // Processor that reads from the last reader and writes to the first writer
    //bool terminated = false;
    //HStreamProcessor<int16_t> rxProcessor(volume.Reader(), BLOCKSIZE, &terminated);



    // Create a fader that turns up the output volume when we begin to process samples.
    // This hides a naste "Click" in the beginning of the file, and other spurious noise
    // coming from filters that needs to stabilize
    // This writer registers as the second writer in the splitter
    HFade<int16_t> fade(volume.Consumer(), 0, 500, true, BLOCKSIZE);

    // Register the outputWriter with the fade component
    fade.Consumer()->SetWriter(next);

    processor->Run();
    return true;
}
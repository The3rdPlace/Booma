#ifndef __INTERNALS_H
#define __INTERNALS_H

#include <stdlib.h>
#include <iostream>
#include <thread>

#include "language.h"
#include "config.h"
#include "receiver.h"

#include "cwreceiver.h"

#include "booma.h"


extern bool terminated;

#define BLOCKSIZE 4096
#define SAMPLERATE H_SAMPLE_RATE_48K

extern HProcessor<int16_t>* processor;
extern HReader<int16_t>* inputReader;
extern HWriter<int16_t>* outputWriter;

extern HWriter<int16_t>* pcmWriter;
extern HWriter<int16_t>* audioWriter;
extern HSplitter<int16_t>* pcmSplitter;
extern HSplitter<int16_t>* audioSplitter;
extern HMute<int16_t>* pcmMute;
extern HMute<int16_t>* audioMute;

extern BoomaReceiver* receiver;

extern std::thread* current;

extern bool BoomaSetInput(ConfigOptions* configOptions);
extern bool BoomaSetOutput(ConfigOptions* configOptions);
extern bool BoomaSetDumps(ConfigOptions* configOptions);

bool CreateCwReceiverChain(ConfigOptions* configOptions, HWriterConsumer<int16_t>* previous, HWriter<int16_t>* next);

#endif

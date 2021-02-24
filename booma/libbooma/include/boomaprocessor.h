#ifndef __PROCESSOR_H
#define __PROCESSOR_H

#include <hardtapi.h>
#include "booma.h"
#include "configoptions.h"

class BoomaProcessor {

    private:

        HStreamProcessor<uint8_t>* _streamProcessor_uint8;
        HStreamProcessor<int16_t>* _streamProcessor_int16;
        HNetworkProcessor<uint8_t>* _networkProcessor_uint8;
        HNetworkProcessor<int16_t>* _networkProcessor_int16;

        HTypeConverter<uint8_t, int16_t>* _converter;

    public:

        BoomaProcessor(HStreamProcessor<uint8_t>* processor):
            _streamProcessor_uint8(processor),
            _streamProcessor_int16(nullptr),
            _networkProcessor_uint8(nullptr),
            _networkProcessor_int16(nullptr) {
            _converter = new HTypeConverter<uint8_t, int16_t>(processor->Consumer(), BLOCKSIZE);
        }

        BoomaProcessor(HStreamProcessor<int16_t>* processor):
                _streamProcessor_uint8(nullptr),
                _streamProcessor_int16(processor),
                _networkProcessor_uint8(nullptr),
                _networkProcessor_int16(nullptr),
                _converter(nullptr) {}

        BoomaProcessor(HNetworkProcessor<uint8_t>* processor, bool withConverter):
                _streamProcessor_uint8(nullptr),
                _streamProcessor_int16(nullptr),
                _networkProcessor_uint8(processor),
                _networkProcessor_int16(nullptr) {
            if( withConverter ) {
                HLog("Adding converter");
                _converter = new HTypeConverter<uint8_t, int16_t>(processor->Consumer(), BLOCKSIZE);
            } else {
                HLog("Not adding converter");
            }

        }

        BoomaProcessor(HNetworkProcessor<int16_t>* processor):
                _streamProcessor_uint8(nullptr),
                _streamProcessor_int16(nullptr),
                _networkProcessor_uint8(nullptr),
                _networkProcessor_int16(processor),
                _converter(nullptr) {}

        ~BoomaProcessor() {
            SAFE_DELETE(_streamProcessor_uint8);
            SAFE_DELETE(_streamProcessor_int16);
            SAFE_DELETE(_networkProcessor_uint8);
            SAFE_DELETE(_networkProcessor_int16);
        }

        HWriterConsumer<int16_t>* Consumer() {
            if( _streamProcessor_uint8 != nullptr ) {
                return _converter->Consumer();
            }
            if( _streamProcessor_int16 != nullptr ) {
                return _streamProcessor_int16->Consumer();
            }
            if( _networkProcessor_uint8 != nullptr ) {
                return _converter->Consumer();
            }
            if( _networkProcessor_int16 != nullptr ) {
                return _networkProcessor_int16->Consumer();
            }
            return nullptr;
        }

        void Run() {
            if( _streamProcessor_uint8 != nullptr ) {
                _streamProcessor_uint8->Run();
            }
            if( _streamProcessor_int16 != nullptr ) {
                _streamProcessor_int16->Run();
            }
            if( _networkProcessor_uint8 != nullptr ) {
                _networkProcessor_uint8->Run();
            }
            if( _networkProcessor_int16 != nullptr ) {
                _networkProcessor_int16->Run();
            }
        }

        void Run(int blocks) {
            if( _streamProcessor_uint8 != nullptr ) {
                _streamProcessor_uint8->Run(blocks);
            }
            if( _streamProcessor_int16 != nullptr ) {
                _streamProcessor_int16->Run(blocks);
            }
            if( _networkProcessor_uint8 != nullptr ) {
                _networkProcessor_uint8->Run(blocks);
            }
            if( _networkProcessor_int16 != nullptr ) {
                _networkProcessor_int16->Run(blocks);
            }
        }

        void Halt() {
            if( _streamProcessor_uint8 != nullptr ) {
                _streamProcessor_uint8->Halt();
            }
            if( _streamProcessor_int16 != nullptr ) {
                _streamProcessor_int16->Halt();
            }
            if( _networkProcessor_uint8 != nullptr ) {
                _networkProcessor_uint8->Halt();
            }
            if( _networkProcessor_int16 != nullptr ) {
                _networkProcessor_int16->Halt();
            }
        }

        bool Command(H_COMMAND_CLASS commandClass, H_COMMAND_OPCODE commandOpcode, int32_t value) {
            HLog("SEND COMMAND %d %d %d", commandClass, commandOpcode, value);
            if( _streamProcessor_uint8 != nullptr ) {
                return ((HProcessor<int16_t>*) _streamProcessor_uint8)->Command(commandClass, commandOpcode, value);
            }
            if( _streamProcessor_int16 != nullptr ) {
                return ((HProcessor<int16_t>*) _streamProcessor_int16)->Command(commandClass, commandOpcode, value);
            }
            if( _networkProcessor_uint8 != nullptr ) {
                return ((HProcessor<int16_t>*) _networkProcessor_uint8)->Command(commandClass, commandOpcode, value);
            }
            if( _networkProcessor_int16 != nullptr ) {
                return ((HProcessor<int16_t>*) _networkProcessor_int16)->Command(commandClass, commandOpcode, value);
            }
            return false;
        }
};

#endif

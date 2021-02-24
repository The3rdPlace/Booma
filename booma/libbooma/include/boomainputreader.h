#ifndef __INPUTREADER_H
#define __INPUTREADER_H

#include <hardtapi.h>
#include "booma.h"
#include "configoptions.h"

class BoomaInputReader {

    private:

        HReader<uint8_t>* _reader_uint8;
        HReader<int16_t>* _reader_int16;

    public:

        BoomaInputReader(HReader<uint8_t>* reader):
            _reader_uint8(reader),
            _reader_int16(nullptr) {
        }

        BoomaInputReader(HReader<int16_t>* reader):
                _reader_uint8(nullptr),
                _reader_int16(reader) {
        }

        ~BoomaInputReader() {
            SAFE_DELETE(_reader_uint8);
            SAFE_DELETE(_reader_int16);
        }

        HReader<uint8_t>* Reader_uint8() {
            return _reader_uint8;
        }

        HReader<int16_t>* Reader_int16() {
            return _reader_int16;
        }
};


#endif

#ifndef __INPUTREADER_H
#define __INPUTREADER_H

#include <hardtapi.h>
#include "booma.h"
#include "configoptions.h"
#include "boomainputreaderexception.h"

class BoomaInputReader {

    private:

        HReader<uint8_t>* _reader_uint8;
        HReader<int16_t>* _reader_int16;

        int _cutOff;

        // unsigned int8 decimation
        HProbe<uint8_t>* _ifMultiplierProbe_uint8;
        HProbe<uint8_t>* _iqFirDecimatorProbe_uint8;
        HProbe<uint8_t>* _iqFirFilterProbe_uint8;
        HProbe<uint8_t>* _iqDecimatorProbe_uint8;
        HProbe<uint8_t>* _firDecimatorProbe_uint8;
        HProbe<uint8_t>* _firFilterProbe_uint8;
        HProbe<uint8_t>* _decimatorProbe_uint8;

        HIqMultiplier<uint8_t>* _ifMultiplier_uint8;
        HIqFirDecimator<uint8_t>* _iqFirDecimator_uint8;
        HIqFirFilter<uint8_t>* _iqFirFilter_uint8;
        HIqDecimator<uint8_t>* _iqDecimator_uint8;
        HFirDecimator<uint8_t>* _firDecimator_uint8;
        HFirFilter<uint8_t>* _firFilter_uint8;
        HDecimator<uint8_t>* _decimator_uint8;

        // signed int16 decimation
        HProbe<int16_t>* _ifMultiplierProbe_int16;
        HProbe<int16_t>* _iqFirDecimatorProbe_int16;
        HProbe<int16_t>* _iqFirFilterProbe_int16;
        HProbe<int16_t>* _iqDecimatorProbe_int16;
        HProbe<int16_t>* _firDecimatorProbe_int16;
        HProbe<int16_t>* _firFilterProbe_int16;
        HProbe<int16_t>* _decimatorProbe_int16;

        HIqMultiplier<int16_t>* _ifMultiplier_int16;
        HIqFirDecimator<int16_t>* _iqFirDecimator_int16;
        HIqFirFilter<int16_t>* _iqFirFilter_int16;
        HIqDecimator<int16_t>* _iqDecimator_int16;
        HFirDecimator<int16_t>* _firDecimator_int16;
        HFirFilter<int16_t>* _firFilter_int16;
        HDecimator<int16_t>* _decimator_int16;

        bool GetDecimationRate(int inputRate, int outputRate, int* first, int* second);
        void SetDecimation(ConfigOptions* opts);

        void Setup8bitChain(ConfigOptions* opts, int firstFactor, int secondFactor);
        void Setup16bitChain(ConfigOptions* opts, int firstFactor, int secondFactor);

    public:

        BoomaInputReader(ConfigOptions* opts, HReader<uint8_t>* reader):
            _reader_uint8(reader),
            _reader_int16(nullptr),
            _ifMultiplier_uint8(nullptr),
            _ifMultiplierProbe_uint8(nullptr),
            _iqFirDecimator_uint8(nullptr),
            _iqFirFilter_uint8(nullptr),
            _iqDecimator_uint8(nullptr),
            _firDecimator_uint8(nullptr),
            _firFilter_uint8(nullptr),
            _decimator_uint8(nullptr),
            _iqFirDecimatorProbe_uint8(nullptr),
            _iqFirFilterProbe_uint8(nullptr),
            _iqDecimatorProbe_uint8(nullptr),
            _firDecimatorProbe_uint8(nullptr),
            _firFilterProbe_uint8(nullptr),
            _decimatorProbe_uint8(nullptr),
            _ifMultiplier_int16(nullptr),
            _ifMultiplierProbe_int16(nullptr),
            _iqFirDecimator_int16(nullptr),
            _iqFirFilter_int16(nullptr),
            _iqDecimator_int16(nullptr),
            _firDecimator_int16(nullptr),
            _firFilter_int16(nullptr),
            _decimator_int16(nullptr),
            _iqFirDecimatorProbe_int16(nullptr),
            _iqFirFilterProbe_int16(nullptr),
            _iqDecimatorProbe_int16(nullptr),
            _firDecimatorProbe_int16(nullptr),
            _firFilterProbe_int16(nullptr),
            _decimatorProbe_int16(nullptr),
            _cutOff(3000) /* Todo: allow configuring this value */ {
            SetDecimation(opts);
        }

        BoomaInputReader(ConfigOptions* opts, HReader<int16_t>* reader):
            _reader_uint8(nullptr),
            _reader_int16(reader),
            _ifMultiplier_uint8(nullptr),
            _ifMultiplierProbe_uint8(nullptr),
            _iqFirDecimator_uint8(nullptr),
            _iqFirFilter_uint8(nullptr),
            _iqDecimator_uint8(nullptr),
            _firDecimator_uint8(nullptr),
            _firFilter_uint8(nullptr),
            _decimator_uint8(nullptr),
            _iqFirDecimatorProbe_uint8(nullptr),
            _iqFirFilterProbe_uint8(nullptr),
            _iqDecimatorProbe_uint8(nullptr),
            _firDecimatorProbe_uint8(nullptr),
            _firFilterProbe_uint8(nullptr),
            _decimatorProbe_uint8(nullptr),
            _ifMultiplier_int16(nullptr),
            _ifMultiplierProbe_int16(nullptr),
            _iqFirDecimator_int16(nullptr),
            _iqFirFilter_int16(nullptr),
            _iqDecimator_int16(nullptr),
            _firDecimator_int16(nullptr),
            _firFilter_int16(nullptr),
            _decimator_int16(nullptr),
            _iqFirDecimatorProbe_int16(nullptr),
            _iqFirFilterProbe_int16(nullptr),
            _iqDecimatorProbe_int16(nullptr),
            _firDecimatorProbe_int16(nullptr),
            _firFilterProbe_int16(nullptr),
            _decimatorProbe_int16(nullptr),
            _cutOff(3000) /* Todo: allow configuring this value */ {
            SetDecimation(opts);
        }

        ~BoomaInputReader() {
            SAFE_DELETE(_reader_uint8);
            SAFE_DELETE(_reader_int16);

            SAFE_DELETE(_ifMultiplier_uint8);
            SAFE_DELETE(_iqFirDecimator_uint8);
            SAFE_DELETE(_iqFirFilter_uint8);
            SAFE_DELETE(_iqDecimator_uint8);
            SAFE_DELETE(_firDecimator_uint8);
            SAFE_DELETE(_firFilter_uint8);
            SAFE_DELETE(_decimator_uint8);

            SAFE_DELETE(_ifMultiplier_int16);
            SAFE_DELETE(_iqFirDecimator_int16);
            SAFE_DELETE(_iqFirFilter_int16);
            SAFE_DELETE(_iqDecimator_int16);
            SAFE_DELETE(_firDecimator_int16);
            SAFE_DELETE(_firFilter_int16);
            SAFE_DELETE(_decimator_int16);

            SAFE_DELETE(_ifMultiplierProbe_uint8);
            SAFE_DELETE(_iqFirDecimatorProbe_uint8);
            SAFE_DELETE(_iqFirFilterProbe_uint8);
            SAFE_DELETE(_iqDecimatorProbe_uint8);
            SAFE_DELETE(_firDecimatorProbe_uint8);
            SAFE_DELETE(_firFilterProbe_uint8);
            SAFE_DELETE(_decimatorProbe_uint8);

            SAFE_DELETE(_ifMultiplierProbe_int16);
            SAFE_DELETE(_iqFirDecimatorProbe_int16);
            SAFE_DELETE(_iqFirFilterProbe_int16);
            SAFE_DELETE(_iqDecimatorProbe_int16);
            SAFE_DELETE(_firDecimatorProbe_int16);
            SAFE_DELETE(_firFilterProbe_int16);
            SAFE_DELETE(_decimatorProbe_int16);
        }

        HReader<uint8_t>* Reader_uint8() {
            if( _iqDecimator_uint8 != nullptr ) {
                return _iqDecimator_uint8->Reader();
            } else if( _iqFirFilter_uint8 != nullptr ) {
                return _iqFirFilter_uint8->Reader();
            } else {
                return _reader_uint8;
            }
        }

        HReader<int16_t>* Reader_int16() {
            // Currently we never use 16 bit decimation
            return _reader_int16;
        }
};

#endif

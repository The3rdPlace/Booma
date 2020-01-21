#ifndef __DECODER_H
#define __DECODER_H

class BoomaDecoder: public HWriter<int16_t> {

    protected:

        BoomaDecoder() {}

        virtual void Decode(int16_t* src, int blocksize) = 0;

    public:

        int Write(int16_t* src, size_t blocksize) {
            Decode(src, blocksize);
            return blocksize;
        }

};

#endif

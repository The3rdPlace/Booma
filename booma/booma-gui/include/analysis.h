#ifndef BOOMA_ANALYSIS_H
#define BOOMA_ANALYSIS_H

#include "boomaapplication.h"

#include <FL/fl_draw.H>

enum AnalysisType { AVERAGE_SPECTRUM };

class Analysis : public Fl_Widget {

    private:

        BoomaApplication* _app;
        AnalysisType _type = AVERAGE_SPECTRUM;
        int _n;
        int _zoom;
        double* _fft;
        float _hzPerBin;
        float _xFactor;
        float _yFactor;

        double* _averageSpectrum;

        void DrawAverageSpectrum();

        inline int ColorMap(double value) {
            long k = value / (long) 20 ;
            int j = k > 255 ? 255 : k;
            return j;
        }

    public:

        Analysis(int X, int Y, int W, int H, const char *L, int n, int zoom, BoomaApplication* app);
        ~Analysis();

        void ReConfigure(int n, int zoom);

        void draw();
        int handle(int event);
        void Refresh();

        double* GetFftBuffer() {
            return _fft;
        }

        void SetType( AnalysisType type ) {
            _type = type;
        }
};

#endif
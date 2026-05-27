#ifndef FILTERPOINTDATA_H
#define FILTERPOINTDATA_H

struct SpanSliderMapping {
    double realMin;
    double realMax;
    int intMin = 0;
    int intMax = 10000;

    int toInt(double value) const {
        return int((value - realMin) / (realMax - realMin) * (intMax - intMin) + intMin + 0.5);
    }

    double toReal(int value) const {
        return realMin + (double)(value - intMin) / (intMax - intMin) * (realMax - realMin);
    }

    void updateRange(double minVal, double maxVal) {
        realMin = minVal;
        realMax = maxVal;
    }
};


struct PointCloudDataminMax {
    double rangeMin, rangeMax;
    double dopplerSpeedMin, dopplerSpeedMax;
    double powerDBMin, powerDBMax;
    double SNRdBMin, SNRdBMax;
    double Q_aziMin, Q_aziMax;
    double Q_eleMin, Q_eleMax;
    double azimuthAngMin, azimuthAngMax;
    double eleAngMin, eleAngMax;
    double radVelAbsMin, radVelAbsMax;
    double rcsdBMin, rcsdBMax;
};


struct FilterCriteria {
    bool enabled = false;

    double rangeMin, rangeMax;
    double dopplerMin, dopplerMax;
    double powerdBMin, powerdBMax;
    double snrMin, snrMax;
    double qAziMin, qAziMax;
    double qEleMin, qEleMax;
    double aziAngMin, aziAngMax;
    double eleAngMin, eleAngMax;
    double radVelAbsMin, radVelAbsMax;
    double rcsdBMin, rcsdBMax;

    enum DetValidMode { All, Only0, Only1 } detValidMode = All;

    FilterCriteria()
        : enabled(false),
        rangeMin(0.0), rangeMax(300.0),
        dopplerMin(-50.0), dopplerMax(50.0),
        powerdBMin(-50.0), powerdBMax(150.0),
        snrMin(0.0), snrMax(60.0),
        qAziMin(0.0), qAziMax(1.0),
        qEleMin(0.0), qEleMax(1.0),
        aziAngMin(-75.0), aziAngMax(75.0),
        eleAngMin(-20.0), eleAngMax(20.0),
        radVelAbsMin(-50.0), radVelAbsMax(50.0),
        rcsdBMin(-50.0), rcsdBMax(150.0),
        detValidMode(All)
    {}
};

#endif // FILTERPOINTDATA_H

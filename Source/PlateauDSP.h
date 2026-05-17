#pragma once
#include <cmath>
#include <vector>
#include <algorithm>

// Math Mocks
inline double clamp(double v, double min_v, double max_v) { return std::max(min_v, std::min(v, max_v)); }
inline double rescale(double x, double xMin, double xMax, double yMin, double yMax) { return yMin + (x - xMin) * (yMax - yMin) / (xMax - xMin); }
inline double scale(double x, double xMin, double xMax, double yMin, double yMax) { return rescale(x, xMin, xMax, yMin, yMax); }
inline double tanhDriveSignal(double x, double drive) { return std::tanh(x * drive) / drive; } // simplified saturation

// VCV Rack DSP Mocks
template <typename T>
struct InterpDelay {
    std::vector<T> buffer;
    size_t writeIndex = 0;
    T input = 0.0;
    T output = 0.0;
    double delayTime = 0.0;

    InterpDelay(size_t maxFrames = 192000, size_t unused = 0) { buffer.resize(maxFrames, 0.0); }
    
    void setDelayTime(double frames) { delayTime = frames; }
    void clear() { std::fill(buffer.begin(), buffer.end(), 0.0); }
    
    T process() {
        buffer[writeIndex] = input;
        output = tap(delayTime);
        writeIndex = (writeIndex + 1) % buffer.size();
        return output;
    }
    
    T tap(double frames) const {
        double readPos = (double)writeIndex - frames;
        while (readPos < 0.0) readPos += buffer.size();
        size_t index1 = (size_t)readPos;
        size_t index2 = (index1 + 1) % buffer.size();
        double frac = readPos - (double)index1;
        return buffer[index1] * (1.0 - frac) + buffer[index2] * frac;
    }
};

template <typename T>
struct AllpassFilter {
    InterpDelay<T> delay;
    T input = 0.0, output = 0.0, gain = 0.5;

    AllpassFilter(size_t maxFrames = 192000, double dTime = 0.0, double g = 0.5) : delay(maxFrames) {
        delay.setDelayTime(dTime);
        setGain(g);
    }
    
    void setGain(double g) { gain = g; }
    void clear() { delay.clear(); }
    
    T process() {
        T delayed = delay.tap(delay.delayTime);
        T v = input - gain * delayed;
        delay.input = v;
        delay.process();
        output = delayed + gain * v;
        return output;
    }
};

struct OnePoleLPFilter {
    double input = 0.0, output = 0.0, fc = 20000.0, sr = 44100.0;
    OnePoleLPFilter(double freq = 22000.0) : fc(freq) {}
    void setCutoffFreq(double freq) { fc = freq; }
    void setSampleRate(double rate) { sr = rate; }
    void clear() { output = 0.0; }
    double process() {
        double w = 2.0 * M_PI * fc / sr;
        if (w > 1.0) w = 1.0;
        output += (input - output) * w;
        return output;
    }
};

struct OnePoleHPFilter {
    OnePoleLPFilter lp;
    double input = 0.0, output = 0.0;
    OnePoleHPFilter(double freq = 0.0) { lp.setCutoffFreq(freq); }
    void setCutoffFreq(double freq) { lp.setCutoffFreq(freq); }
    void setSampleRate(double rate) { lp.setSampleRate(rate); }
    void clear() { lp.clear(); output = 0.0; }
    double process() {
        lp.input = input;
        output = input - lp.process();
        return output;
    }
};

struct TriSawLFO {
    double phase = 0.0, freq = 1.0, sr = 44100.0, rev = 0.5;
    void setFrequency(double f) { freq = f; }
    void setRevPoint(double r) { rev = clamp(r, 0.001, 0.999); }
    void setSampleRate(double s) { sr = s; }
    double process() {
        phase += freq / sr;
        if (phase >= 1.0) phase -= 1.0;
        if (phase < rev) return phase / rev * 2.0 - 1.0;
        else return (1.0 - phase) / (1.0 - rev) * 2.0 - 1.0;
    }
};

struct LinearEnvelope {
    double _value = 0.0, sr = 44100.0, step = 0.001;
    double start = 0.0, end = 1.0;
    bool _justFinished = false, active = false;
    
    void setSampleRate(double s) { sr = s; }
    void setTime(double t) { step = 1.0 / (t * sr); }
    void setStartEndPoints(double s, double e) { start = s; end = e; }
    void trigger() { _value = start; active = true; _justFinished = false; }
    
    void process() {
        _justFinished = false;
        if (!active) return;
        if (start < end) {
            _value += step;
            if (_value >= end) { _value = end; active = false; _justFinished = true; }
        } else {
            _value -= step;
            if (_value <= end) { _value = end; active = false; _justFinished = true; }
        }
    }
};

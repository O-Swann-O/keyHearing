#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <thread>
#include <chrono>
#include "portaudio.h"


struct Note {
    const char* name;
    float frequency;
};

static const Note pianoNotes[] = {
    {"C3", 130.813f}, {"Db3", 138.591f}, {"D3", 146.832f}, {"Eb3", 155.563f},
    {"E3", 164.814f}, {"F3", 174.614f}, {"Gb3", 185.000f}, {"G3", 196.000f},
    {"Ab3", 207.652f}, {"A3", 220.000f}, {"Bb3", 233.082f}, {"B3", 246.942f},
    {"C4", 261.626f}, {"Db4", 277.183f}, {"D4", 293.665f}, {"Eb4", 311.127f},
    {"E4", 329.628f}, {"F4", 349.228f}, {"Gb4", 369.994f}, {"G4", 391.995f},
    {"Ab4", 415.305f}, {"A4", 440.000f}, {"Bb4", 466.164f}, {"B4", 493.883f},
    {"C5", 523.251f}, {"Db5", 554.365f}, {"D5", 587.330f}, {"Eb5", 622.254f},
    {"E5", 659.255f}, {"F5", 698.456f}, {"Gb5", 739.989f}, {"G5", 783.991f},
    {"Ab5", 830.609f}, {"A5", 880.000f}, {"Bb5", 932.328f}, {"B5", 987.767f},
    {"C6", 1046.50f}
};

const int noteCount = sizeof(pianoNotes) / sizeof(Note);

const Note* getNote(float frequency) {
    float minDiff = 3.4028235e+38f;
    const Note* nearestNote = nullptr;

    for (int i = 0; i < noteCount; ++i) {
        float diff = frequency - pianoNotes[i].frequency;
        if (diff < 0.0f) diff = -diff; 
        if (diff < minDiff) {
            minDiff = diff;
            nearestNote = &pianoNotes[i];
        }
    }
    return nearestNote;
}


void fastfourier(std::vector<std::complex<float>>& signal) {
    constexpr float PI = 3.1415927f;
    size_t N = signal.size();
    if (N <= 1) return;

    std::vector<std::complex<float>> even(N / 2);
    std::vector<std::complex<float>> odd(N / 2);

    for (size_t i = 0; i < N / 2; ++i) {
        even[i] = signal[2 * i];
        odd[i] = signal[2 * i + 1];
    }

    fastfourier(even);
    fastfourier(odd);

    for (size_t k = 0; k < N / 2; ++k) {
        std::complex<float> t = std::polar(1.0f, -2 * PI * k / N) * odd[k];
        signal[k] = even[k] + t;
        signal[k + N / 2] = even[k] - t;
    }
}

int main() {
    Pa_Initialize();
    PaStream* stream;
    const int SAMPLE_RATE = 44100;
    const int FRAMES_PER_BUFFER = 4096;
    float buffer[FRAMES_PER_BUFFER];

    Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, SAMPLE_RATE, FRAMES_PER_BUFFER, nullptr, nullptr);
    Pa_StartStream(stream);

    constexpr float PI = 3.1415927f;
    const int spectrumSize = FRAMES_PER_BUFFER / 2;
    const int maxHarmonic = 3;

    std::vector<float> hannWindow(FRAMES_PER_BUFFER);
    for (size_t i = 0; i < FRAMES_PER_BUFFER; ++i) {
        hannWindow[i] = 0.5f * (1.0f - std::cos(2.0f * PI * i / (FRAMES_PER_BUFFER - 1)));
    }

    while (true) {
    Pa_ReadStream(stream, buffer, FRAMES_PER_BUFFER);

    // use Hann window.
    std::vector<std::complex<float>> signal(FRAMES_PER_BUFFER);
    for (size_t i = 0; i < FRAMES_PER_BUFFER; ++i) {
        signal[i] = std::complex<float>(buffer[i] * hannWindow[i], 0.0f);
    }

    fastfourier(signal);

    // compute power of signal.
    std::vector<float> spectrum(spectrumSize);
    float totalPower = 0.0f;
    for (int k = 1; k < spectrumSize; ++k) {
        float power = signal[k].real() * signal[k].real() + signal[k].imag() * signal[k].imag();
        spectrum[k] = power;
        totalPower += power;
    }


    // HPS. calculates how powerful an indexes overtones are. if they are strong we can assume this is index corresponds to the fundamental freq. 
    std::vector<float> hps = spectrum;
    for (int h = 2; h <= maxHarmonic; ++h) {
        int limit = spectrumSize / h;
        for (int k = 0; k < limit; ++k) {
            hps[k] *= spectrum[k * h];
        }
        for (int k = limit; k < spectrumSize; ++k) {
            hps[k] = 0.0f;
        }
    }

    
    float maxValue = hps[1];
    int maxIndex = 1;
    for (int k = 2; k < spectrumSize; ++k) {
        if (hps[k] > maxValue) {
            maxValue = hps[k];
            maxIndex = k;
        }
    }

    float binToFreq = SAMPLE_RATE / static_cast<float>(FRAMES_PER_BUFFER);
    float maxFreq = maxIndex * binToFreq;

    const Note* note = getNote(maxFreq);
    float centsOff = 0.0f;
    if (note && note->frequency > 0.0f) {
        centsOff = 1200.0f * std::log2(maxFreq / note->frequency);
    }
    
    if (totalPower < 500.0f || maxFreq < 129) {
    std::cout << "NO_SIGNAL" << std::endl;


    } else {
        std::cout << note->name << " " << maxFreq << " " << centsOff << std::endl;
    }

    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    return 0;
}

#include <iostream>
#include "portaudio.h"

int main() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    PaStream *stream;
    const int SAMPLE_RATE = 44100;
    const int FRAMES_PER_BUFFER = 256;
    float buffer[FRAMES_PER_BUFFER];

    err = Pa_OpenDefaultStream(
        &stream,
        1,          // num input channels
        0,          // num output channels
        paFloat32,  // sample format
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        nullptr,    // no callback, use blocking API
        nullptr     // no user data
    );
    if (err != paNoError) {
        std::cerr << "Failed to open stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return 1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "Failed to start stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    std::cout << "Recording from microphone..." << std::endl;
    err = Pa_ReadStream(stream, buffer, FRAMES_PER_BUFFER);
    if (err != paNoError) {
        std::cerr << "Failed to read stream: " << Pa_GetErrorText(err) << std::endl;
    } else {
        std::cout << "Read " << FRAMES_PER_BUFFER << " samples from mic." << std::endl;
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    return 0;
}
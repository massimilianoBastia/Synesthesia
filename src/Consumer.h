#pragma once

#include <vector>
#include <string>
#include "Queue.h"
#include "Log.h"
#include <fstream>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <map>

namespace processing {

    class Consumer {
    public:

        Consumer(Queue<std::pair<std::vector<double>, int>>& queue, const std::string& outputFilename, bool verbose, utils::Log& logger)
            : queue(queue), outputFilename(outputFilename), verbose(verbose), logger(logger) {}
        
        inline void writePCM24BitBE() {

            std::ofstream outFile(outputFilename, std::ios::binary);

            if (!outFile.is_open()) {
                throw std::runtime_error("[Consumer Thread] Failed to open output file");
            }

            std::map<int, std::vector<double>> frequencyData;
            size_t maxSamples = 0;

            while (true) {

                std::pair<std::vector<double>, int> data;
                bool hasData = queue.try_pop(data);

                if (!hasData) {
                    if (queue.has_done()) {
                        break;
                    }
                    continue;
                }

                auto& freqBuffer = frequencyData[data.second];
                freqBuffer.insert(freqBuffer.end(), data.first.begin(), data.first.end());
                maxSamples = std::max(maxSamples, freqBuffer.size());

                logger.logMessage("[Consumer Thread] Received " + std::to_string(data.first.size()) +
                    " samples for frequency " + std::to_string(data.second));
            }

            for (auto& kv : frequencyData) {
                if (kv.second.size() < maxSamples) {
                    kv.second.resize(maxSamples, 0.0);
                }
            }


            std::vector<double> mixedBuffer(maxSamples, 0.0);

            for (const auto& kv : frequencyData) {
                const auto& buffer = kv.second;
                for (size_t i = 0; i < buffer.size(); ++i) {
                    mixedBuffer[i] += buffer[i];
                }
            }

            // Normalize the mixed buffer to ensure the audio signal remains within the valid range from [-1.0, 1.0] 

            double maxAmplitude = *std::max_element(mixedBuffer.begin(), mixedBuffer.end());
            if (maxAmplitude > 1.0) {
                for (auto& sample : mixedBuffer) {
                    sample /= maxAmplitude;
                }
            }

            std::vector<uint8_t> outputBuffer(mixedBuffer.size() * 3);

            for (size_t i = 0; i < mixedBuffer.size(); ++i) {

                /*
                    Normalize the sample from[-1.0, 1.0] to[0, 1] for unsigned PCM conversion
                */
                double normalizedSample = (mixedBuffer[i] + 1.0) * 0.5;


                /*
                    Convert the normalized sample(which is in the range[0, 1])
                    into a 24-bit unsigned PCM value to make sure the PCM
                    representation is accurate.
                */
                uint32_t pcmValue = static_cast<uint32_t>(std::round(normalizedSample * MAX_AMPLITUDE));

                /*
                    Convert the 24-bit value to big-endian format:
                */
                size_t offset = i * 3;
                outputBuffer[offset] = (pcmValue >> 16) & 0xFF;
                outputBuffer[offset + 1] = (pcmValue >> 8) & 0xFF;
                outputBuffer[offset + 2] = pcmValue & 0xFF;

                if (verbose) {
                    logger.logMessage(std::string("[Consumer Thread] ") +
                        "  Sample Index: " + std::to_string(i) +
                        ", Mixed Value: " + std::to_string(mixedBuffer[i]) +
                        ", Normalized: " + std::to_string(normalizedSample) +
                        ", PCM Value: " + std::to_string(pcmValue) +
                        ", PCM Hex Value: " + logger.formatHex(pcmValue));
                }
            }

            outFile.write(reinterpret_cast<const char*>(outputBuffer.data()), outputBuffer.size());

            if (!outFile) {
                throw std::runtime_error("Error occurred while writing to the output file.");
            }

            outFile.close();
            if (!outFile) {
                throw std::runtime_error("Error occurred while writing to the output file.");
            }

            logger.logMessage("[Consumer Thread] Done writing task. A total of " + std::to_string(outputBuffer.size() / 3) + " samples were successfully written to the output file.");

        }

    private:

        Queue<std::pair<std::vector<double>, int>>& queue;
        std::string outputFilename;
        bool verbose;
        utils::Log& logger;
        const double MIN_SAMPLE = -1.0;
        const double MAX_SAMPLE = 1.0;
        const double MAX_AMPLITUDE = std::pow(2.0, 24) - 1;
    };

}
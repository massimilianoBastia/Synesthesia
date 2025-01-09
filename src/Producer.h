#pragma once

#include <vector>
#include <cmath>
#include "Queue.h"
#include "Log.h"
#include <stdexcept>
#include <algorithm>
#include <sstream>

namespace processing {

    class Producer {

    public:

        Producer(int frequency, int samplesCount, Queue<std::pair<std::vector<double>, int>>& queue, int thread_index, bool verbose, utils::Log& logger)
            : frequency(frequency), samplesCount(samplesCount), queue(queue), thread_index(thread_index), verbose(verbose), logger(logger) {}

        void generateSinusoidalData() {

            double angularFrequency = 2.0 * PI / frequency;
            int remaining_samples = samplesCount;
            int sample_index = 0;

            logger.logMessage("[Producer Thread No. " + std::to_string(thread_index) + "] Generating " + std::to_string(samplesCount) + " samples for frequency " + std::to_string(frequency));

            /*
                 Create the sine wave data in smaller chunks, repeating until all the needed samples are generated
            */

            while (remaining_samples > 0) {

                int samplesToGenerate = std::min(remaining_samples, MAX_BUFFER_SIZE);
                std::vector<double> buffer(samplesToGenerate);

                /*
                    Fill the buffer with sine wave values.
                    Each value is calculated using the current sample index and the angular frequency to determine its position in the wave.
                */

                for (int i = 0; i < samplesToGenerate; ++i) {
                    buffer[i] = std::sin(angularFrequency * (sample_index + i));
                    if (verbose) {
                        logger.logMessage("[Producer Thread No. " + std::to_string(thread_index) + "] Frequency:  " + std::to_string(frequency) + ", Sample Index: " + std::to_string(sample_index + i) + ", Value " + std::to_string(buffer[i]));
                    }
                }

                queue.push({ buffer, frequency });
                logger.logMessage("[Producer Thread No. " + std::to_string(thread_index) + "] Pushed " + std::to_string(buffer.size()) + " samples");

                remaining_samples -= samplesToGenerate;
                sample_index += samplesToGenerate;
            }

        }

    private:

        int frequency;
        int samplesCount;
        Queue<std::pair<std::vector<double>, int>>& queue;
        int thread_index;
        bool verbose;
        utils::Log& logger;
        static constexpr double PI = 3.14159265358979323846;
        static constexpr int MAX_BUFFER_SIZE = 1000;
    };

}


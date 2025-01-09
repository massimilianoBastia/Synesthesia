#include <stdexcept>
#include "CliParser.h"
#include "Queue.h"
#include "Log.h"
#include "Producer.h"
#include "Consumer.h"

int main(int argc, char* argv[]) {

    try {
        
        configuration::SynesthesiaConfiguration config = configuration::CliParser::parse(argc, argv);
        
        std::vector<std::thread> producer_threads;
        
        Queue<std::pair<std::vector<double>, int>> queue;
        
        utils::Log logger(1 * 1024 * 1024);

        if (config.log_to_file) {
            logger.setupFile(config.log_file_name);
        }

        int thread_index = 0;

        for (auto& frequency : config.freq_list) {
            logger.logMessage("[Main] Assigned frequency: " + std::to_string(frequency) + " to thread number " + std::to_string(thread_index));
            producer_threads.emplace_back(
                std::thread(
                    &processing::Producer::generateSinusoidalData,
                    processing::Producer(frequency, config.samples_number, queue, thread_index, config.verbose, logger)
                )
            );
            ++thread_index;
        }

        std::thread consumer_thread(
            &processing::Consumer::writePCM24BitBE,
            processing::Consumer(queue, config.output_file_name, config.verbose, logger)
        );

        for (auto& producer_thread : producer_threads) {
            producer_thread.join();
        }
        queue.set_done();
        consumer_thread.join();

    }
    catch (const std::exception& ex) {
        std::cerr << "[Main] Error: " << ex.what() << std::endl;
    }

    return 0;
}
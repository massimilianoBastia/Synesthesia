#pragma once

#include <sstream>
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <limits>

namespace configuration {

    struct SynesthesiaConfiguration {
        std::vector<int> freq_list;
        std::string output_file_name;
        int samples_number = 0;
        std::string log_file_name;
        bool log_to_file = false;
        bool verbose = false;
    };

    class CliParser {

    public:

        inline static SynesthesiaConfiguration parse(int argc, char* argv[]) {
            if (argc < 4 || argc > 7) {
                throw std::invalid_argument("Usage Synesthesia freqlist outfile samplescount --logfile=<path> --verbose");
            }

            SynesthesiaConfiguration config;

            try {

                config.freq_list = parse_freqlist(argv[1]);

                config.output_file_name = argv[2];
                if (config.output_file_name.empty()) {
                    throw std::invalid_argument("Output file name cannot be empty");
                }

                config.samples_number = parse_to_int(argv[3], 1);

                for (int i = 4; i < argc; ++i) {

                    std::string arg = argv[i];

                    if (arg.rfind("--logfile=", 0) == 0) {
                        config.log_file_name = arg.substr(10);
                        if (config.log_file_name.empty()) {
                            throw std::invalid_argument("Log file path cannot be empty");
                        }
                        config.log_to_file = true;
                        std::cout << "Log file path: " << config.log_file_name << std::endl;
                    }
                    else if (arg == "--verbose") {
                        config.verbose = true;
                        std::cout << "Verbose logging enabled." << std::endl;
                    }
                    else {
                        throw std::invalid_argument("Invalid argument: " + arg);
                    }

                }

            }
            catch (const std::exception& e) {
                throw std::invalid_argument(std::string("Error parsing command-line arguments: ") + e.what());
            }

            return config;
        }

    private:

        inline static std::vector<int> parse_freqlist(const std::string& freqlist) {
            std::vector<int> frequencies;
            std::istringstream stream(freqlist);
            std::string token;

            while (std::getline(stream, token, ',')) {
                frequencies.push_back(parse_to_int(token, 1, 10000));
            }

            return frequencies;
        }

        inline static int parse_to_int(const std::string& input, int min_value, int max_value = std::numeric_limits<int>::max()) {
            try {
                long value = std::stol(input);

                if (value < min_value || value > max_value) {
                    throw std::out_of_range("Value out of range");
                }

                return static_cast<int>(value);
            }
            catch (const std::exception& e) {
                throw std::invalid_argument("Invalid integer value: " + input + ". " + e.what());
            }
        }

    };
}
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <stdexcept>
#include <atomic>
#include <vector>
#include <iomanip>

namespace utils {

    class Log {

    public:

        explicit Log(size_t maxFileSizeInBytes)
            : logStream(&std::cout), currentLogFileSize(0), logFileIndex(0), maxFileSize(maxFileSizeInBytes), stopLogging(false) {
            loggingThread = std::thread(&Log::processLogQueue, this);
        }

        ~Log() {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                stopLogging = true;
            }
            queueCondition.notify_all();
            if (loggingThread.joinable()) {
                loggingThread.join();
            }
        }

        void setupFile(const std::string& logFilePath) {
            std::lock_guard<std::mutex> lock(logMutex);
            if (!logFilePath.empty()) {
                logFileIndex = 0;
                std::string initialLogFileName = "log_0.txt";

                logFile.open(initialLogFileName, std::ios::out | std::ios::trunc);
                if (!logFile.is_open()) {
                    throw std::runtime_error("Error: Unable to open log file at " + initialLogFileName);
                }

                logStream = &logFile;
                currentLogFileSize = 0;
            }
        }

        void logMessage(const std::string& message) {
            enqueueLog(message, "\033[32m");
        }

        void logError(const std::string& message) {
            enqueueLog(message, "\033[31m");
        }

        std::string formatHex(uint32_t value) {
            std::ostringstream oss;
            oss << std::hex << std::uppercase << value;
            return oss.str();
        }

        void logHex(uint32_t value, const std::string& message) {
            std::ostringstream oss;
            oss << std::hex << std::setfill('0') << std::setw(8) << value;
            std::string hexString = oss.str();
            enqueueLog(message + " " + hexString, "\033[36m");
        }

    private:

        void enqueueLog(const std::string& message, const std::string& colorCode) {
            std::lock_guard<std::mutex> lock(queueMutex);
            logQueue.push({ message, colorCode });
            queueCondition.notify_one();
        }

        void processLogQueue() {
            while (true) {
                std::unique_lock<std::mutex> lock(queueMutex);
                queueCondition.wait(lock, [this] { return !logQueue.empty() || stopLogging; });

                if (stopLogging && logQueue.empty()) {
                    break;
                }

                auto [message, colorCode] = logQueue.front();
                logQueue.pop();
                lock.unlock();

                log(message, colorCode);
            }
        }

        void log(const std::string& message, const std::string& colorCode) {
            std::lock_guard<std::mutex> lock(logMutex);

            if (logFile.is_open()) {
                logFile << message << std::endl;
                currentLogFileSize += message.size() + 1;

                if (currentLogFileSize >= maxFileSize) {
                    rotateLogFile();
                }
            }
            else {
                *logStream << colorCode << message << "\033[0m" << std::endl;
            }
        }

        void rotateLogFile() {
            logFile.close();
            ++logFileIndex;
            currentLogFileSize = 0;

            std::string newLogFileName = "log_" + std::to_string(logFileIndex) + ".txt";
            logFile.open(newLogFileName, std::ios::out | std::ios::trunc);
            if (!logFile.is_open()) {
                throw std::runtime_error("Error: Unable to open new log file at " + newLogFileName);
            }
        }

        std::ostream* logStream;
        std::ofstream logFile;
        std::mutex logMutex;
        std::queue<std::pair<std::string, std::string>> logQueue;
        std::mutex queueMutex;
        std::condition_variable queueCondition;
        std::thread loggingThread;
        std::atomic<bool> stopLogging;
        size_t currentLogFileSize;
        int logFileIndex;
        size_t maxFileSize;
    };

}
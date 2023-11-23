#include <iostream>
#include <Windows.h>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <regex>

#include "log_duration.h"
#include <list>

std::mutex outputMutex;
std::condition_variable resultReady;
bool allThreadsFinished = false;

using namespace std;

void searchSubstring(const std::string& filename, const std::string& mask, int threadId, int numThreads)
{
    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;
    std::vector<std::string> results;
    int lineNumber = 0;

    while (std::getline(file, line))
    {
        lineNumber++;

        if (lineNumber % numThreads == threadId)
        {
            size_t pos = 0;
            while ((pos = line.find(mask, pos)) != std::string::npos)
            {
                results.push_back(std::to_string(lineNumber) + " " + std::to_string(pos + 1) + " " + line.substr(pos, mask.size()));
                pos += mask.size();
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(outputMutex);
        for (const auto& result : results)
        {
            std::cout << result << std::endl;
        }
    }

    if (results.empty())
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        //std::cout << "Thread " << threadId << " did not find any matches." << std::endl;
    }

    file.close();
    resultReady.notify_one();
}

void searchSubstring_(const std::string& filename, const std::string& mask, int threadId, int numThreads)
{
    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;
    std::list<std::string> results;
    //results.reserve(1000000);
    int lineNumber = 0;

    size_t maskSize = mask.size();

    while (std::getline(file, line))
    {
        lineNumber++;

        if (line.size() < mask.size())
            continue;

        if (lineNumber % numThreads == threadId)
        {
            size_t lineSize = line.size();
            for (size_t i = 0; i <= lineSize - maskSize; ++i)
            {
                bool match = true;
                for (size_t j = 0; j < maskSize; ++j)
                {
                    if (mask[j] != '?' && mask[j] != line[i + j])
                    {
                        match = false;
                        break;
                    }
                }

                if (match)
                {
                    results.push_back(std::to_string(lineNumber) + " " + std::to_string(i + 1) + " " + line.substr(i, maskSize));
                    i += maskSize - 1; // Skip to the end of the matched substring
                }
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(outputMutex);
        //std::cout << results.size() << std::endl;
        for (const auto& result : results)
        {
            //std::cout << result << std::endl;
        }
    }

    if (results.empty())
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        //std::cout << "Thread " << threadId << " did not find any matches." << std::endl;
    }

    file.close();
    resultReady.notify_one();
}

void searchSubstringRegex(const std::string& filename, const std::string& mask, int threadId, int numThreads)
{
    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;
    std::vector<std::string> results;
    int lineNumber = 0;

    std::regex regex(mask);

    while (std::getline(file, line))
    {
        lineNumber++;

        if (lineNumber % numThreads == threadId)
        {
            std::smatch match;
            std::string::const_iterator searchStart(line.cbegin());

            while (std::regex_search(searchStart, line.cend(), match, regex))
            {
                size_t position = match.position();
                results.push_back(std::to_string(lineNumber) + " " + std::to_string(position + 1) + " " + match.str());
                searchStart += match.position() + 1; // Move search position to the next character
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(outputMutex);
        for (const auto& result : results)
        {
            std::cout << result << std::endl;
        }
    }

    if (results.empty())
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        //std::cout << "Thread " << threadId << " did not find any matches." << std::endl;
    }

    file.close();
    resultReady.notify_one();
}

void callWithThreads(const std::string& file, const std::string& mask)
{
    LOG_DURATION("callWithThreads"s);
    if (mask.size() > 1000)
    {
        std::cerr << "Mask size exceeds the maximum limit of 1000 characters." << std::endl;
        return;
    }

    std::vector<std::thread> threads;
    int numThreads = std::thread::hardware_concurrency();

    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(searchSubstring_, file, mask, i, numThreads);
    }

    for (std::thread& thread : threads)
    {
        thread.join();
    }
}

void callSeq(const std::string& file, const std::string& mask)
{
    LOG_DURATION("callSeq"s);
    if (mask.size() > 1000)
    {
        std::cerr << "Mask size exceeds the maximum limit of 1000 characters." << std::endl;
        return;
    }

    searchSubstring_(file, mask, 0, 1);
}

int main(int argc, char* argv[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <filename> <mask>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::string mask = argv[2];

    callSeq(filename, mask);
    callWithThreads(filename, mask);

    return 0;
}

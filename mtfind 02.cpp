#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

class MtFinder {
public:
    MtFinder(const std::string& filename, const std::string& mask)
        : filename_(filename), mask_(mask), occurrences_(0) {}

    void FindOccurrences() {
        std::ifstream file(filename_);
        if (!file) {
            std::cerr << "Error opening file: " << filename_ << std::endl;
            return;
        }

        std::string line;
        std::mutex mutex;
        std::vector<std::thread> threads;

        size_t lineIndex = 0;
        while (std::getline(file, line)) {
            lineIndex++;
            threads.emplace_back(std::thread([this, line, lineIndex, &mutex]() {
                size_t pos = 0;
                while ((pos = line.find(mask_, pos)) != std::string::npos) {
                    mutex.lock();
                    occurrences_++;
                    std::cout << occurrences_ << " " << lineIndex << " " << pos + 1 << " " << line.substr(pos, mask_.size()) << std::endl;
                    mutex.unlock();
                    pos += mask_.size();  // Move past the found occurrence
                }
                }));
        }

        for (auto& thread : threads) {
            thread.join();
        }

        file.close();
    }

private:
    std::string filename_;
    std::string mask_;
    size_t occurrences_;
};

int main(int argc, char** argv) {

    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    if (argc < 3) {
        std::cerr << "Usage: mtfind <filename> <mask>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::string mask = argv[2];

    MtFinder finder(filename, mask);
    finder.FindOccurrences();

    return 0;
}

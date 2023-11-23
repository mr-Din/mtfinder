#pragma once
#include "FindedLine.h"
#include <string>
#include <mutex>
#include <map>
#include <vector>

class MtFinderReadToVector
{
public:
    MtFinderReadToVector(const std::string& filename, const std::string& mask);

public:
    void RunSearch();
    void Print();
    void OpenFile();

private:
    void ProcessLine(int thread_id, int num_threads);

private:
    std::string m_filename;
    std::string m_mask;
    std::mutex m_output_mutex;
    std::map<size_t, FindedLine> m_map_results;
    std::vector<std::string> m_lines;
    std::mutex m_lines_mutex;
};


#pragma once
#include "FindedLine.h"
#include <string>
#include <mutex>
#include <map>
#include <vector>

class MyMtFinder
{
public:
	MyMtFinder(const std::string& filename, const std::string& mask);

public:
    // Реализация построчного поиска. Каждый поток открывает файл на чтение
	void RunSearch();
    void Print();

private:
	void FindOccurrences(int thread_id, int num_threads);

private:
    std::string m_filename;
    std::string m_mask;
    size_t m_occurrences;
    std::mutex m_output_mutex;
    std::map<size_t, FindedLine> m_map_results;
};


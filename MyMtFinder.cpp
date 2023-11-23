#include "MyMtFinder.h"
#include "log_duration.h"
#include <iostream>
#include <fstream>

MyMtFinder::MyMtFinder(const std::string& filename, const std::string& mask)
    : m_filename(filename)
    , m_mask(mask)
    , m_occurrences(0)
    , m_output_mutex(std::mutex())
{}

void MyMtFinder::FindOccurrences(int thread_id, int num_threads)
{
    std::ifstream file(m_filename);
    if (!file)
    {
        std::cerr << "Ошибка открытия файла: " << m_filename << std::endl;
        return;
    }
    file.seekg (0, std::ios::end);
    std::streampos file_size = file.tellg();

    if (file_size > 1024 * 1024 * 1024)
    {
        std::cerr << "Файл слишком большой." << std::endl;
        file.close();
        return;
    }
    file.seekg(0, std::ios::beg);

    std::string line;
    std::map<size_t, FindedLine> results;
    int line_num = 0;

    size_t mask_size = m_mask.size();

    while (std::getline(file, line))
    {
        line_num++;

        if (line.size() < m_mask.size())
            continue;

        if (line_num % num_threads == thread_id)
        {
            size_t line_size = line.size();
            for (size_t i = 0; i <= line_size - mask_size; ++i)
            {
                bool match = true;
                for (size_t j = 0; j < mask_size; ++j)
                {
                    if (m_mask[j] != '?' && m_mask[j] != line[i + j])
                    {
                        match = false;
                        break;
                    }
                }

                if (match)
                {
                    results.insert({ line_num, FindedLine(line_num, i + 1, line.substr(i, mask_size)) });
                    i += mask_size - 1;
                }
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_output_mutex);
        m_map_results.insert(results.begin(), results.end());
    }

    if (results.empty())
    {
        std::lock_guard<std::mutex> lock(m_output_mutex);
        std::cout << "Поток " << thread_id << " не нашёл вхождений" << std::endl;
    }

    file.close();
}

void MyMtFinder::RunSearch()
{
    LOG_DURATION("MyMtFinder::RunSearch");
    if (m_mask.size() > 1000)
    {
        std::cerr << "Mask size exceeds the maximum limit of 1000 characters." << std::endl;
        return;
    }

    std::vector<std::thread> threads;
    int num_threads = std::thread::hardware_concurrency();

    for (int i = 0; i < num_threads; ++i)
        threads.emplace_back(&MyMtFinder::FindOccurrences, this, i, num_threads);

    for (std::thread& thread : threads)
        thread.join();

    std::cout << m_map_results.size() << std::endl;
}

void MyMtFinder::Print()
{
    for (const auto& [_, result] : m_map_results)
        std::cout << result << std::endl;
}
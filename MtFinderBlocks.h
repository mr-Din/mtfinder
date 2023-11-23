#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <algorithm>

#include "log_duration.h"
#include "FindedLine.h"

class MtFinderBlocks
{
public:
    MtFinderBlocks(const std::string& filename, const std::string& mask)
        : m_mutex(std::mutex())
        , m_filename(filename)
        , m_mask(mask)
        , m_num_threads(std::thread::hardware_concurrency())
    {

    }

    void searchWordInLines(int start_line, int end_line)
    {
        std::ifstream file(m_filename);
        std::string line;
        int line_num = 0;
        size_t mask_size = m_mask.size();

        while (std::getline(file, line))
        {
            line_num++;

            if (line.size() < m_mask.size())
                continue;

            if (line_num >= start_line && line_num <= end_line)
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
                        std::lock_guard<std::mutex> lock(m_mutex);
                        m_line_numbers.push_back(line_num);
                        m_result.push_back(FindedLine(line_num, i + 1, line.substr(i, mask_size)));
                        i += mask_size - 1;
                    }
                }
                if (line.find(m_mask) != std::string::npos)
                {
                    m_line_numbers.push_back(line_num);
                }
            }
            else if (line_num > end_line)
            {
                break;  // Выход, если достигнут конец диапазона для данного потока.
            }           
        }

        file.close();
    }

    void RunSearch()
    {
        LOG_DURATION("search with threads");
        std::ifstream file(m_filename);
        const int num_lines = std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n') + 1;
        file.close();

        const int lines_per_thrd = num_lines / m_num_threads;

        std::vector<std::thread> threads;

        for (int i = 0; i < m_num_threads; i++)
        {
            int start_line = i * lines_per_thrd;
            int end_line = (i == m_num_threads - 1) ? num_lines : (i + 1) * lines_per_thrd;

            threads.emplace_back(&MtFinderBlocks::searchWordInLines, this, start_line, end_line);
        }

        for (auto& thread : threads)
        {
            thread.join();
        }

        std::sort(m_line_numbers.begin(), m_line_numbers.end());

        std::cout << m_line_numbers.size() << std::endl;
        /*for (const auto& line_number : m_line_numbers)
        {
            std::cout << line_number << std::endl;
        }*/
    }

    void Print()
    {
        for (const auto& result : m_result)
            std::cout << result << std::endl;
    }

private:
    std::vector<int> m_line_numbers;  // Общий вектор для хранения номеров строк, содержащих искомое слово.
    std::vector<FindedLine> m_result;
    std::mutex m_mutex;  // Мьютекс для безопасного доступа к общему ресурсу.
    std::string m_filename;
    std::string m_mask;
    int m_num_threads;
};


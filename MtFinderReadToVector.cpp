#include "MtFinderReadToVector.h"
#include "log_duration.h"
#include <iostream>
#include <fstream>

MtFinderReadToVector::MtFinderReadToVector(const std::string& filename, const std::string& mask)
    : m_filename(filename)
    , m_mask(mask)
    , m_output_mutex(std::mutex())
    , m_lines(std::vector<std::string>())
    , m_lines_mutex(std::mutex())
{}

void MtFinderReadToVector::RunSearch()
{
    OpenFile();
    LOG_DURATION("MtFinderReadToVector::RunSearch");
    if (m_mask.size() > 1000)
    {
        std::cerr << "Mask size exceeds the maximum limit of 1000 characters." << std::endl;
        return;
    }

    std::vector<std::thread> threads;
    int num_threads = std::thread::hardware_concurrency();

    for (int i = 0; i < num_threads; ++i)
        threads.emplace_back(&MtFinderReadToVector::ProcessLine, this, i, num_threads);

    for (std::thread& thread : threads)
        thread.join();

    std::cout << m_map_results.size() << std::endl;
}

void MtFinderReadToVector::Print()
{
    for (const auto& [_, result] : m_map_results)
        std::cout << result << std::endl;
}

void MtFinderReadToVector::OpenFile()
{
    std::ifstream file(m_filename);
    if (!file)
    {
        std::cerr << "Ошибка открытия файла: " << m_filename << std::endl;
        return;
    }
    const int num_lines = std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n') + 1;
    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();

    if (file_size > 1024 * 1024 * 1024)
    {
        std::cerr << "Файл слишком большой." << std::endl;
        file.close();
        return;
    }
    file.seekg(0, std::ios::beg);

    std::string line;
    m_lines.reserve(num_lines);
    while (std::getline(file, line))
    {
        std::lock_guard<std::mutex> lock(m_lines_mutex);
        m_lines.push_back(line);
    }
}

void MtFinderReadToVector::ProcessLine(int thread_id, int num_threads)
{
    std::map<size_t, FindedLine> results;

    size_t mask_size = m_mask.size();

    for (size_t i = thread_id; i < m_lines.size(); i += num_threads)
    {
        size_t line_size = m_lines[i].size();

        if (line_size < m_mask.size())
            continue;

        for (size_t j = 0; j <= line_size - mask_size; ++j)
        {
            bool match = true;
            for (size_t k = 0; k < mask_size; ++k)
            {
                if (m_mask[k] != '?' && m_mask[k] != m_lines[i][j + k])
                {
                    match = false;
                    break;
                }
            }

            if (match)
            {
                results.insert({ i + 1, FindedLine(i + 1, j + 1, m_lines[i].substr(j, mask_size)) });
                j += mask_size - 1;
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
}

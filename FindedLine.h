#pragma once
#include <string>
#include <iostream>

struct FindedLine
{
    FindedLine()
        : row(0)
        , pos(0)
        , text("")
    {}
    FindedLine(size_t row, size_t pos, std::string text)
        : row(row)
        , pos(pos)
        , text(text)
    {}
    size_t row;
    size_t pos;
    std::string text;
};

std::ostream& operator << (std::ostream& os, const FindedLine& line);

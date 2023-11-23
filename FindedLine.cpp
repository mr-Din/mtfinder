#include "FindedLine.h"

std::ostream& operator << (std::ostream& os, const FindedLine& line)
{
    return os << line.row << " " << line.pos << " " << line.text;
}
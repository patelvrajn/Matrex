#include "square.hpp"

#include "globals.hpp"

std::ostream& operator<<(std::ostream& os, const Square s)
{
    os << SQUARE_STRINGS[s.get_index()];

    return os;
}

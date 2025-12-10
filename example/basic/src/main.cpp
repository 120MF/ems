#include "ems_parser.hpp"
#include <format>
#include <iostream>
using namespace ems::literals;
constexpr auto melody = "(104){4}4s,4s,5,6,6,5,4s,3,2,2,3,4s,4s-,3-3,,4s,4s,5,6,6,5,4s,3,2,2,3,4s,3-,2-2,,3,3,4s,2,3,4s-5-4s,2,"_ems;
int main()
{
    for (const auto& note : melody) {
        auto s = std::format("ratio: {}, duration: {} ", note.ratio, note.duration_ms);
        std::cout << s << "\n";
    }
}
#include "ems_parser.hpp"
#include "Audio.hpp"

#include <format>
#include <iostream>
using namespace ems::literals;
constexpr auto melody =
    R"((200)
        2s`,,1s`,7,,1s`,2s`-,3`-2s`,1s`,,,
        2s`,,1s`,7,,1s`,2s`-,3`-2s`,1s`,,,
        2s`,,1s`,7,,1s`,2s`-,3`-2s`,1s`,,,
        2s`,,1s`,7,,1s`,2s`-,3`-2s`,1s`,,7-1s`-
        2s`,2s`,1s`,3`,2s`,1s`,1s`,1s`,7,3`,2s`,1s`,
        1s`,,7-1s`-2s`,,,,,,7,4s`,7`,
        6s`,,7`,6s`,,7`,6s`-5s`-4s`,,4s`,1s`,3`,
        3`,2s`,2s`,2s`,,,3`,2s`,1s`,2s`,,4s`,
        7,,,
)"_ems;

int main()
{
    // You can hear the melody playing
    for (const auto& [ratio, duration_ms] : melody)
    {
        auto s = std::format("ratio: {}, duration: {} ", ratio, duration_ms);
        std::cout << s << "\n";
    }
    playMelody(melody);
}

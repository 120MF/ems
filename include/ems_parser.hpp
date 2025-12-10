/**
 * @file ems_parser.hpp
 * @brief Embedded Music Score (EMS) C++20 Compile-time Parser
 * @version 0.1.0
 * @author MoonFeather
 * @author WeaveStar
 * @license ISC License
 * 
 * A header-only C++20 library to parse text-based music scores into 
 * frequency ratios and durations at compile-time.
 * 
 * Target: Embedded Systems (Zephyr, Arduino, STM32, etc.)
 * Memory: Zero runtime heap allocation. Score string is discarded after compilation.
 * 
 * Usage:
 *   #include "ems.hpp"
 *   using namespace ems::literals;
 *   
 *   // (BPM){Beat} Note...
 *   constexpr auto melody = "(120){4} 1, 1, 5, 5, 6, 6, 5_"_ems;
 * 
 *   void play() {
 *       for (const auto& note : melody) {
 *           pwm_set(note.ratio);
 *           sleep_ms(note.duration_ms);
 *       }
 *   }
 */

#ifndef EMS_PARSER_HPP
#define EMS_PARSER_HPP

#include <array>
#include <cstdint>
#include <string_view>
#include <algorithm>

namespace ems
{
    /**
     * @brief The compiled note event structure.
     * Use this in your playback loop.
     */
    struct Note
    {
        float ratio; ///< Frequency ratio relative to the base note (1.0 = Do). 0.0 = Rest.
        uint32_t duration_ms; ///< Duration in milliseconds.
    };

    namespace internal
    {
        // Compile-time power function approximation for float
        consteval float power(const float base, const int exp)
        {
            if (exp == 0) return 1.0f;
            if (exp < 0) return 1.0f / power(base, -exp);
            float res = 1.0f;
            for (int i = 0; i < exp; ++i) res *= base;
            return res;
        }

        // 基于 A4=440Hz 的频率比（倍频）计算：返回 freq / 440.0f
        consteval float calculate_ratio(const int note_num,
                                        const int octave_offset,
                                        const int semitone_offset)
        {
            if (note_num == 0) return 0.0f; // Rest

            // 1(C)=0, 2(D)=2, 3(E)=4, 4(F)=5, 5(G)=7, 6(A)=9, 7(B)=11
            constexpr int scale_semitones[] = {0, 0, 2, 4, 5, 7, 9, 11};

            const int semitones_from_C4 =
                scale_semitones[note_num] + octave_offset * 12 + semitone_offset;

            // A4 to C4: +9
            const int semitones_from_A4 = semitones_from_C4 - 9;

            constexpr float SEMITONE_STEP = 1.059463094f; // 2^(1/12)
            return power(SEMITONE_STEP, semitones_from_A4);
        }

        // Helper to parse integer from string_view
        consteval int parse_int(const std::string_view sv, size_t& idx)
        {
            int val = 0;
            while (idx < sv.size() && sv[idx] >= '0' && sv[idx] <= '9')
            {
                val = val * 10 + (sv[idx] - '0');
                idx++;
            }
            return val;
        }

        /**
         * @brief Core Parser Class
         * Implements a two-pass strategy:
         * 1. Count notes to determine std::array size.
         * 2. Parse data into the array.
         */
        class Parser
        {
        public:
            static consteval size_t count_notes(const std::string_view score)
            {
                size_t count = 0;
                size_t i = 0;

                // Skip Header
                while (i < score.size())
                {
                    if (score[i] == '(')
                    {
                        while (i < score.size() && score[i] != ')') i++;
                        i++;
                        continue;
                    }
                    break;
                }

                // Scan Body
                while (i < score.size())
                {
                    // Detect Note Start: 0-7 or prefix `
                    if (const char c = score[i]; c >= '0' && c <= '7')
                    {
                        count++;
                        i++;
                    }
                    else
                    {
                        i++;
                    }
                }
                return count;
            }

            template <size_t N>
            static consteval std::array<Note, N> parse(std::string_view score)
            {
                std::array<Note, N> notes{};
                size_t note_idx = 0;
                size_t i = 0;

                float bpm = 120.0f;

                // --- 1. Header Parsing ---
                if (i < score.size() && score[i] == '(')
                {
                    i++;
                    bpm = static_cast<float>(parse_int(score, i));
                    if (i < score.size() && score[i] == ')') i++;
                }

                const float ms_per_beat = 60000.0f / bpm;

                // --- 2. Body Parsing ---
                while (i < score.size() && note_idx < N)
                {
                    if (const char c = score[i]; (c >= '0' && c <= '7') || c == '`')
                    {
                        int num = 0;
                        int oct = 0;
                        int semi = 0;
                        float dur_mult = 0.0f;

                        // 1. Prefix Octave (Lower)
                        if (score[i] == '`')
                        {
                            oct--; // 降八度
                            i++;
                        }

                        // 2. Note Number
                        if (i < score.size() && score[i] >= '0' && score[i] <= '7')
                        {
                            num = score[i] - '0';
                            i++;
                        }

                        // 3. Suffix Modifiers
                        // 防止混淆 Duration 后面的 Pitch 修饰符
                        bool parsing_duration = false;
                        bool loop = true;

                        while (i < score.size() && loop)
                        {
                            const char mod = score[i];
                            switch (mod)
                            {
                            // --- Pitch Modifiers ---
                            case 's':
                            case 'b':
                            case '`':
                                if (parsing_duration)
                                {
                                    // 如果已经进入时长模式，再次遇到音高修饰符（如 `），
                                    // 说明这是下一个音符的前缀，立即停止当前解析。
                                    loop = false;
                                    continue; // 不消耗字符 i
                                }
                                if (mod == 's') semi++;
                                else if (mod == 'b') semi--;
                                else { oct++; } // `
                                break;

                            // --- Duration Modifiers ---
                            case ',': dur_mult += 1.0f;
                                parsing_duration = true;
                                break;
                            case '-': dur_mult += 0.5f;
                                parsing_duration = true;
                                break;
                            case '.': dur_mult += 0.25f;
                                parsing_duration = true;
                                break;
                            case '_': dur_mult += 2.0f;
                                parsing_duration = true;
                                break;

                            default:
                                loop = false;
                                continue; // 不消耗字符 i
                            }
                            i++; // 消耗有效的修饰符
                        }

                        notes[note_idx].ratio = calculate_ratio(num, oct, semi);
                        notes[note_idx].duration_ms = static_cast<uint32_t>(ms_per_beat * dur_mult);
                        note_idx++;
                    }
                    else
                    {
                        i++;
                    }
                }
                return notes;
            }
        };

        // String Literal Wrapper for CNTTP (C++20)
        template <size_t N>
        struct StringLiteral
        {
            char value[N]{};

            constexpr StringLiteral(const char (&str)[N])
            {
                std::copy_n(str, N, value);
            }
        };
    } // namespace internal

    // Public API: User Defined Literal
    namespace literals
    {
        template <internal::StringLiteral Lit>
        consteval auto operator""_ems()
        {
            constexpr std::string_view sv{Lit.value, sizeof(Lit.value) - 1};
            constexpr size_t N = internal::Parser::count_notes(sv);
            return internal::Parser::parse<N>(sv);
        }
    } // namespace literals
} // namespace ems

#endif // EMS_PARSER_HPP

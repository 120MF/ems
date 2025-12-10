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
        constexpr float power(const float base, const int exp)
        {
            if (exp == 0) return 1.0f;
            if (exp < 0) return 1.0f / power(base, -exp);
            float res = 1.0f;
            for (int i = 0; i < exp; ++i) res *= base;
            return res;
        }

        // Calculate frequency ratio relative to Middle C (Do)
        // 1=Do (0 semitones), 1`=High Do (12 semitones)
        constexpr float calculate_ratio(const int note_num, const int octave_offset, const int semitone_offset)
        {
            if (note_num == 0) return 0.0f; // Rest

            // Mapping scale degree (1-7) to semitone offset from C
            // 1(C)=0, 2(D)=2, 3(E)=4, 4(F)=5, 5(G)=7, 6(A)=9, 7(B)=11
            constexpr int scale_semitones[] = {0, 0, 2, 4, 5, 7, 9, 11};

            const int total_semitones = scale_semitones[note_num]
                + (octave_offset * 12)
                + semitone_offset;

            // 12th root of 2 ~= 1.059463094
            constexpr float SEMITONE_STEP = 1.059463094f;
            return power(SEMITONE_STEP, total_semitones);
        }

        // Helper to parse integer from string_view
        constexpr int parse_int(const std::string_view sv, size_t& idx)
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
            static consteval size_t count_notes(std::string_view score)
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
                    if (score[i] == '{')
                    {
                        while (i < score.size() && score[i] != '}') i++;
                        i++;
                        continue;
                    }
                    break;
                }

                // Scan Body
                while (i < score.size())
                {
                    const char c = score[i];
                    if (c == ',' || c == ' ' || c == '\n' || c == '\r')
                    {
                        i++;
                        continue;
                    }

                    // Detect Note Start: 0-7 or prefix `
                    if ((c >= '0' && c <= '7') || c == '`')
                    {
                        count++;

                        // Consume prefix `
                        if (score[i] == '`') i++;
                        // Consume digit
                        if (i < score.size() && score[i] >= '0' && score[i] <= '7') i++;

                        // Consume modifiers
                        while (i < score.size())
                        {
                            const char n = score[i];
                            const bool is_mod = (n == 's' || n == 'b' || n == ',' ||
                                n == '-' || n == '.' || n == '_' || n == '`');
                            if (!is_mod) break;
                            i++;
                        }
                    }
                    else
                    {
                        i++; // Skip unknown chars
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


                int bpm = 120;
                int beat_base = 4; // Default to quarter note

                // --- 1. Header Parsing ---
                if (i < score.size() && score[i] == '(')
                {
                    i++;
                    bpm = parse_int(score, i);
                    if (i < score.size() && score[i] == ')') i++;
                }
                if (i < score.size() && score[i] == '{')
                {
                    i++;
                    beat_base = parse_int(score, i);
                    if (i < score.size() && score[i] == '}') i++;
                }

                const float ms_per_beat = 60000.0f / bpm;

                // --- 2. Body Parsing ---
                while (i < score.size() && note_idx < N)
                {
                    char c = score[i];
                    if (c == ',' || c == ' ' || c == '\n' || c == '\r')
                    {
                        i++;
                        continue;
                    }

                    if ((c >= '0' && c <= '7') || c == '`')
                    {
                        int num = 0;
                        int oct = 0;
                        int semi = 0;
                        float dur_mult = 1.0f; // Default 1 beat

                        // Prefix Octave (Lower)
                        if (score[i] == '`')
                        {
                            oct--;
                            i++;
                        }

                        // Note Number
                        if (i < score.size() && score[i] >= '0' && score[i] <= '7')
                        {
                            num = score[i] - '0';
                            i++;
                        }

                        // Suffix Modifiers
                        bool loop = true;
                        while (i < score.size() && loop)
                        {
                            switch (score[i])
                            {
                            case 's': semi++;
                                break;
                            case 'b': semi--;
                                break;
                            case '`': oct++;
                                break; // Suffix Octave (Higher)
                            case ',': dur_mult = 1.0f;
                                break; // 1 beat
                            case '-': dur_mult = 0.5f;
                                break; // 1/2 beat
                            case '.': dur_mult = 0.25f;
                                break; // 1/4 beat
                            case '_': dur_mult = 2.0f;
                                break; // 2 beats
                            default: loop = false;
                                continue; // Break switch, don't increment i
                            }
                            i++;
                        }

                        // Calculate Logic
                        notes[note_idx].ratio = calculate_ratio(num, oct, semi);

                        // Duration: (ms_per_beat) * (modifier) * (scale to quarter note)
                        // If base is {4}, scalar is 1. If base is {8}, scalar is 0.5?
                        // Usually {4} means 1 beat = quarter note.
                        // Correct logic based on EMS: {X} defines what "1 beat" is.
                        // So "1," is always ms_per_beat. No extra scaling needed based on beat_base
                        // unless we want to normalize strictly to quarter notes,
                        // but usually BPM is "Beats Per Minute", where Beat is defined by {X}.

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

/*
 * Copyright (c) 2026 Zeeshan Qazi
 *
 * This file is part of Aitvaras.
 *
 * Aitvaras is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Aitvaras is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Aitvaras.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef AITVARAS_UTILS_HPP
#define AITVARAS_UTILS_HPP

#include <cstdint>
#include <type_traits>
#include <string>
#include <string_view>
#include <cmath>
#include <charconv>

#if defined(__GNUC__) || defined(__clang__)
#define AITVARAS_BSWAP16(x) __builtin_bswap16(x)
#define AITVARAS_BSWAP32(x) __builtin_bswap32(x)
#define AITVARAS_BSWAP64(x) __builtin_bswap64(x)
#else
#include <byteswap.h>
#define AITVARAS_BSWAP16(x) bswap_16(x)
#define AITVARAS_BSWAP32(x) bswap_32(x)
#define AITVARAS_BSWAP64(x) bswap_64(x)
#endif

namespace aitvaras {

template <typename T>
constexpr T byteswap(T value) noexcept {
    if constexpr (sizeof(T) == 1) {
        return value;
    } else if constexpr (sizeof(T) == 2) {
        return AITVARAS_BSWAP16(value);
    } else if constexpr (sizeof(T) == 4) {
        return AITVARAS_BSWAP32(value);
    } else if constexpr (sizeof(T) == 8) {
        return AITVARAS_BSWAP64(value);
    } else {
        static_assert(sizeof(T) == 0, "Unsupported size for byteswap");
    }
}

namespace detail {
    constexpr int64_t pow10_constexpr(int exp) {
        int64_t res = 1;
        for (int i = 0; i < exp; ++i) res *= 10;
        return res;
    }
}

template <int DecimalPlaces>
struct fixed_point_value {
    int64_t internal_value;
    
    static constexpr int64_t scale_factor = detail::pow10_constexpr(DecimalPlaces);

    constexpr fixed_point_value() noexcept : internal_value(0) {}
    constexpr explicit fixed_point_value(int64_t val) noexcept : internal_value(val) {}

    // Convert from double
    constexpr fixed_point_value(double val) noexcept {
        internal_value = static_cast<int64_t>(std::round(val * scale_factor));
    }

    // Convert to double
    constexpr explicit operator double() const noexcept {
        return static_cast<double>(internal_value) / scale_factor;
    }

    // Convert to string
    std::string to_string() const {
        std::string s = std::to_string(internal_value);
        if (DecimalPlaces == 0) return s;
        
        bool negative = false;
        if (!s.empty() && s[0] == '-') {
            negative = true;
            s.erase(0, 1);
        }

        if (s.length() <= DecimalPlaces) {
            s.insert(0, DecimalPlaces - s.length() + 1, '0');
        }
        s.insert(s.length() - DecimalPlaces, ".");
        
        if (negative) {
            s.insert(0, "-");
        }
        return s;
    }

    // Convert from string
    static fixed_point_value from_string(std::string_view sv) {
        double d = 0;
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), d);
        if (ec == std::errc()) {
            return fixed_point_value(d);
        }
        return fixed_point_value();
    }
    
    constexpr bool operator==(const fixed_point_value& other) const noexcept {
        return internal_value == other.internal_value;
    }
    
    constexpr bool operator!=(const fixed_point_value& other) const noexcept {
        return internal_value != other.internal_value;
    }
};

} // namespace aitvaras

#endif // AITVARAS_UTILS_HPP

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

#ifndef AITVARAS_JSON_HPP
#define AITVARAS_JSON_HPP

#include <string_view>
#include <ostream>
#include <iomanip>

namespace aitvaras {

struct JsonOptions {
    bool detailed_verbosity = false;
    bool raw_enums = false;
};

namespace detail {
    inline void escape_json_string(std::ostream& os, std::string_view str) {
        os << '"';
        for (char c : str) {
            switch (c) {
                case '"': os << "\\\""; break;
                case '\\': os << "\\\\"; break;
                case '\b': os << "\\b"; break;
                case '\f': os << "\\f"; break;
                case '\n': os << "\\n"; break;
                case '\r': os << "\\r"; break;
                case '\t': os << "\\t"; break;
                default:
                    if ('\x00' <= c && c <= '\x1f') {
                        os << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c) << std::dec;
                    } else {
                        os << c;
                    }
            }
        }
        os << '"';
    }
}

} // namespace aitvaras

#endif // AITVARAS_JSON_HPP

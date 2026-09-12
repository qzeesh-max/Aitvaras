/*
 * Copyright (c) 2026 Zeeshan Qazi <zeeshan@zeeshan.im>
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

#ifndef AITVARAS_ANNOTATIONS_HPP
#define AITVARAS_ANNOTATIONS_HPP

#include <cstddef>
#include <meta>

namespace aitvaras {

struct packed {};



struct default_invoke {
    std::meta::info func;
};

struct padded_string {
    char pad;
    size_t len;
};

struct null_terminated_string {
    size_t max_len;
};

struct enumeration {
    std::meta::info enum_type;
};

struct fixed_point {
    int decimal_places;
};

struct non_fixed_bitmap {
    std::meta::info bitmask_field;
    int bit_index;
};

struct length_precedes {
    std::meta::info type_meta;
    int size_adjustment = 0;
};

struct tlv_appendage {
    uint8_t tag;
};

struct tlv_region_length {};

struct little_endian {};
struct big_endian {};

} // namespace aitvaras

#endif // AITVARAS_ANNOTATIONS_HPP

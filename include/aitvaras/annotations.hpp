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

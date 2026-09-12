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

#ifndef AITVARAS_MARSHALER_HPP
#define AITVARAS_MARSHALER_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <meta>
#include <expected>
#include <array>
#include <string>
#include <aitvaras/json.hpp>
#include <string_view>
#include <tuple>
#include <vector>
#include <array>
#include <bit>
#include <algorithm>
#include "annotations.hpp"
#include "utils.hpp"

namespace aitvaras {

namespace detail {

    template <typename Target, std::meta::info field>
    consteval std::meta::info get_annotation_by_type() {
        static constexpr auto annos = std::define_static_array(std::meta::annotations_of(field));
        template for (constexpr auto a : annos) {
            if constexpr (std::is_same_v<Target, std::remove_cvref_t<typename [: std::meta::type_of(a) :]>>) {
                return a;
            }
        }
        return ^^void;
    }

    template <typename T, std::meta::info field>
    consteval bool has_annotation() {
        return get_annotation_by_type<T, field>() != ^^void;
    }

    template <typename T, std::meta::info field>
    consteval T get_annotation() {
        constexpr auto a = get_annotation_by_type<T, field>();
        return std::meta::extract<T>(a);
    }
    
    template <std::meta::info field>
    consteval bool needs_byteswap() {
        bool field_big = has_annotation<big_endian, field>();
        bool field_little = has_annotation<little_endian, field>();
        
        if (field_big) {
            return std::endian::native == std::endian::little;
        } else if (field_little) {
            return std::endian::native == std::endian::big;
        }
        
        constexpr auto parent_type = std::meta::parent_of(field);
        bool struct_big = has_annotation<big_endian, parent_type>();
        bool struct_little = has_annotation<little_endian, parent_type>();
        
        if (struct_big) {
            return std::endian::native == std::endian::little;
        } else if (struct_little) {
            return std::endian::native == std::endian::big;
        }
        
        return false;
    }

    template <std::meta::info field>
    consteval bool is_variable_field() {
        if constexpr (get_annotation_by_type<length_precedes, field>() != ^^void) return true;
        if constexpr (get_annotation_by_type<non_fixed_bitmap, field>() != ^^void) return true;
        return false;
    }

    template <std::meta::info field>
    consteval size_t get_fixed_field_size() {
        constexpr auto type_info = std::meta::type_of(field);
        
        if constexpr (has_annotation<tlv_appendage, field>()) {
            return 0;
        } else if constexpr (has_annotation<padded_string, field>()) {
            constexpr auto anno = get_annotation<padded_string, field>();
            return anno.len;
        } else if constexpr (has_annotation<null_terminated_string, field>()) {
            constexpr auto anno = get_annotation<null_terminated_string, field>();
            return anno.max_len;
        } else if constexpr (has_annotation<fixed_point, field>()) {
            return sizeof(typename [: type_info :]);
        } else if constexpr (has_annotation<enumeration, field>()) {
            return sizeof(typename [: type_info :]);
        } else if constexpr (std::is_class_v<typename [: type_info :]>) {
            size_t sum = 0;

            static constexpr auto mems = std::define_static_array(
                std::meta::members_of(type_info, std::meta::access_context::current()));
            template for (constexpr auto m : mems) {
                if constexpr (std::meta::is_nonstatic_data_member(m)) {
                    sum += get_fixed_field_size<m>();
                }
            }
            return sum;
        } else {
            return sizeof(typename [: type_info :]);
        }
    }

    template <typename Definition>
    consteval size_t count_variable_fields() {
        size_t count = 0;
        static constexpr auto mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
        template for (constexpr auto m : mems) {
            if constexpr (std::meta::is_nonstatic_data_member(m)) {
                if constexpr (is_variable_field<m>()) {
                    count++;
                }
            }
        }
        return count;
    }

    template <typename Definition>
    consteval size_t get_total_fixed_size() {
        size_t sum = 0;
        static constexpr auto mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
        template for (constexpr auto m : mems) {
            if constexpr (std::meta::is_nonstatic_data_member(m)) {
                if constexpr (!is_variable_field<m>()) {
                    sum += get_fixed_field_size<m>();
                }
            }
        }
        return sum;
    }
    
    // Core Marshaler State
    template <typename Definition, bool ForReading>
    struct MarshalerState {
        using BufferType = std::conditional_t<ForReading, std::span<const char>, std::span<char>>;
        BufferType buffer_;
        
        static constexpr size_t num_var_fields = count_variable_fields<Definition>();
        static constexpr size_t fixed_size = get_total_fixed_size<Definition>();

        template <typename T>
        static consteval size_t count_tlv_fields() {
            size_t count = 0;
            static constexpr auto mems = std::define_static_array(std::meta::members_of(^^T, std::meta::access_context::current()));
            template for (constexpr auto m : mems) {
                if constexpr (std::meta::is_nonstatic_data_member(m)) {
                    if constexpr (has_annotation<tlv_appendage, m>()) {
                        count++;
                    }
                }
            }
            return count;
        }
        
        static constexpr size_t NumTLVFields = count_tlv_fields<Definition>();
        std::array<uint32_t, 64> tlv_offsets_{};
        std::array<uint32_t, 64> tlv_sizes_{};

        template <std::meta::info field>
        static constexpr bool is_tlv_field() {
            return has_annotation<tlv_appendage, field>();
        }

        template <std::meta::info field>
        size_t get_tlv_field_index() const {
            size_t idx = 0;
            static constexpr auto mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
            template for (constexpr auto m : mems) {
                if constexpr (std::meta::is_nonstatic_data_member(m)) {
                    if constexpr (std::meta::identifier_of(m) == std::meta::identifier_of(field)) {
                        return idx;
                    }
                    if constexpr (is_tlv_field<m>()) {
                        idx++;
                    }
                }
            }
            return idx;
        }

        static consteval std::meta::info get_tlv_region_length_field() {
            static constexpr auto mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
            template for (constexpr auto m : mems) {
                if constexpr (std::meta::is_nonstatic_data_member(m)) {
                    if constexpr (has_annotation<tlv_region_length, m>()) {
                        return m;
                    }
                }
            }
            return std::meta::info{};
        }
        
        std::array<uint32_t, num_var_fields > var_sizes_{};
        uint32_t total_size_{fixed_size};
        
        MarshalerState(BufferType buf) : buffer_(buf) {
            if constexpr (ForReading) {
                preparse();
            }
        }
        
        template <std::meta::info field>
        size_t get_field_offset() const {
            size_t offset = 0;
            size_t var_idx = 0;
            
            static constexpr auto mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
            template for (constexpr auto m : mems) {
                if constexpr (std::meta::is_nonstatic_data_member(m)) {
                    if constexpr (std::meta::identifier_of(m) == std::meta::identifier_of(field)) {
                        return offset;
                    }
                    if constexpr (is_variable_field<m>()) {
                        offset += var_sizes_[var_idx++];
                    } else {
                        offset += get_fixed_field_size<m>();
                    }
                }
            }
            return offset;
        }

        template <std::meta::info field>
        size_t get_var_field_index() const {
            size_t var_idx = 0;
            static constexpr auto mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
            template for (constexpr auto m : mems) {
                if constexpr (std::meta::is_nonstatic_data_member(m)) {
                    if constexpr (std::meta::identifier_of(m) == std::meta::identifier_of(field)) {
                        return var_idx;
                    }
                    if constexpr (is_variable_field<m>()) {
                        var_idx++;
                    }
                }
            }
            return var_idx;
        }
        
        void preparse() {
            static constexpr auto mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
            template for (constexpr auto m : mems) {
                if constexpr (std::meta::is_nonstatic_data_member(m)) {
                    if constexpr (is_variable_field<m>()) {
                        if constexpr (has_annotation<length_precedes, m>()) {
                            constexpr auto anno = get_annotation<length_precedes, m>();
                            constexpr std::string_view len_name = std::meta::identifier_of(anno.type_meta);
                            constexpr int size_adj = anno.size_adjustment;
                            
                            size_t len_val = 0;
                            template for (constexpr auto len_m : mems) {
                                if constexpr (std::meta::is_nonstatic_data_member(len_m)) {
                                    constexpr std::string_view name = std::meta::identifier_of(len_m);
                                    if constexpr (name == len_name) {
                                        len_val = this->template _get_fixed<len_m>(this->template get_field_offset<len_m>());
                                    }
                                }
                            }
                            
                            size_t var_idx = get_var_field_index<m>();
                            if (size_adj < 0 && len_val < static_cast<size_t>(-size_adj)) {
                                len_val = 0;
                            } else {
                                len_val += size_adj;
                            }
                            var_sizes_[var_idx] = len_val;
                            total_size_ += len_val;
                        } else if constexpr (has_annotation<non_fixed_bitmap, m>()) {
                            constexpr auto anno = get_annotation<non_fixed_bitmap, m>();
                            constexpr std::string_view bitmask_name = std::meta::identifier_of(anno.bitmask_field);
                            constexpr int bit_index = anno.bit_index;
                            
                            uint64_t bitmask_val = 0;
                            template for (constexpr auto bm_m : mems) {
                                if constexpr (std::meta::is_nonstatic_data_member(bm_m)) {
                                    constexpr std::string_view name = std::meta::identifier_of(bm_m);
                                    if constexpr (name == bitmask_name) {
                                        auto bm_val = this->template _get_fixed<bm_m>(this->template get_field_offset<bm_m>());
                                        bitmask_val = static_cast<uint64_t>(bm_val);
                                    }
                                }
                            }
                            
                            size_t var_idx = get_var_field_index<m>();
                            if (bitmask_val & (1ULL << bit_index)) {
                                size_t len_val = get_fixed_field_size<m>();
                                var_sizes_[var_idx] = len_val;
                                total_size_ += len_val;
                            } else {
                                var_sizes_[var_idx] = 0;
                            }
                        }
                    }
                }
            }

            size_t tlv_len = 0;
            size_t tlv_start = 0;
            template for (constexpr auto m : mems) {
                if constexpr (std::meta::is_nonstatic_data_member(m)) {
                    if constexpr (has_annotation<tlv_region_length, m>()) {
                        size_t tlv_len_field_offset = this->template get_field_offset<m>();
                        tlv_len = this->template _get_fixed<m>(tlv_len_field_offset);
                        // TLV data starts immediately after the length field itself
                        tlv_start = tlv_len_field_offset + get_fixed_field_size<m>();
                    }
                }
            }
            if (tlv_len > 0 && tlv_start > 0) {
                size_t pos = tlv_start;
                size_t end = tlv_start + tlv_len;
                while (pos + 2 <= end && pos + 2 <= buffer_.size()) {
                    uint8_t tag_len = static_cast<uint8_t>(buffer_[pos]);
                    uint8_t tag_id = static_cast<uint8_t>(buffer_[pos + 1]);
                    if (tag_len < 1) break;
                    size_t val_pos = pos + 2;
                    size_t val_len = tag_len - 1;
                    if (val_pos + val_len > buffer_.size() || val_pos + val_len > end) {
                        break;
                    }
                    
                    template for (constexpr auto tm : mems) {
                        if constexpr (std::meta::is_nonstatic_data_member(tm)) {
                            if constexpr (is_tlv_field<tm>()) {
                                constexpr auto tanno = get_annotation<tlv_appendage, tm>();
                                if (tanno.tag == tag_id) {
                                    size_t idx = get_tlv_field_index<tm>();
                                    tlv_offsets_[idx] = val_pos;
                                    tlv_sizes_[idx] = val_len;
                                }
                            }
                        }
                    }
                    pos += 1 + tag_len;
                }
                total_size_ += tlv_len;
            }
        }
        
        template <std::meta::info field>
        auto _get_fixed(size_t offset) const {
            constexpr auto type_info = std::meta::type_of(field);
            using FieldType = typename [: type_info :];
            
            if constexpr (has_annotation<padded_string, field>()) {
                constexpr auto anno = get_annotation<padded_string, field>();
                std::string_view sv(buffer_.data() + offset, anno.len);
                size_t end_pos = sv.find_last_not_of(anno.pad);
                if (end_pos != std::string_view::npos) {
                    return sv.substr(0, end_pos + 1);
                }
                return std::string_view(sv.data(), 0);
            } else if constexpr (has_annotation<null_terminated_string, field>()) {
                constexpr auto anno = get_annotation<null_terminated_string, field>();
                std::string_view sv(buffer_.data() + offset, anno.max_len);
                size_t null_pos = sv.find('\0');
                if (null_pos != std::string_view::npos) {
                    sv = sv.substr(0, null_pos);
                }
                return sv;
            } else if constexpr (std::is_arithmetic_v<FieldType> || std::is_enum_v<FieldType>) {
                FieldType v;
                std::memcpy(&v, buffer_.data() + offset, sizeof(FieldType));
                
                // Endian swap if required
                if constexpr (needs_byteswap<field>()) {
                    if constexpr (std::is_enum_v<FieldType>) {
                        using UnderType = std::underlying_type_t<FieldType>;
                        v = static_cast<FieldType>(byteswap(static_cast<UnderType>(v)));
                    } else {
                        v = byteswap(v);
                    }
                }
                
                if constexpr (has_annotation<fixed_point, field>()) {
                    constexpr auto anno = get_annotation<fixed_point, field>();
                    return static_cast<double>(v) / detail::pow10_constexpr(anno.decimal_places);
                } else {
                    return v;
                }
            } else if constexpr (std::is_array_v<FieldType> && std::is_same_v<std::remove_extent_t<FieldType>, char>) {
                return std::string_view(reinterpret_cast<const char*>(buffer_.data() + offset), sizeof(FieldType));
            } else if constexpr (std::is_array_v<FieldType>) {
                return std::span<const uint8_t>(buffer_.data() + offset, sizeof(FieldType));
            } else {
                return FieldType{};
            }
        }
        
        template <std::meta::info field>
        auto get() const {
            using FieldType = typename [: std::meta::type_of(field) :];
            if constexpr (is_tlv_field<field>()) {
                size_t idx = get_tlv_field_index<field>();
                size_t offset = tlv_offsets_[idx];
                size_t len = tlv_sizes_[idx];
                if (!offset) {
                    using OptType = decltype([]() {
                        if constexpr (std::is_same_v<FieldType, std::span<const uint8_t>>) {
                            return std::span<const uint8_t>{};
                        } else if constexpr (std::is_same_v<FieldType, std::string_view>) {
                            return std::string_view{};
                        } else if constexpr (has_annotation<padded_string, field>() || has_annotation<null_terminated_string, field>() || (std::is_array_v<FieldType> && std::is_same_v<std::remove_extent_t<FieldType>, char>)) {
                            return std::string_view{};
                        } else if constexpr (std::is_array_v<FieldType>) {
                            return std::span<const uint8_t>{};
                        } else if constexpr (std::is_arithmetic_v<FieldType> || std::is_enum_v<FieldType>) {
                            if constexpr (has_annotation<fixed_point, field>()) return double{};
                            else return FieldType{};
                        } else {
                            return FieldType{};
                        }
                    }());
                    return std::optional<OptType>{};
                }
                
                if constexpr (std::is_same_v<FieldType, std::span<const uint8_t>>) {
                    return std::optional(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(buffer_.data() + offset), len));
                } else if constexpr (std::is_same_v<FieldType, std::string_view>) {
                    return std::optional(std::string_view(reinterpret_cast<const char*>(buffer_.data() + offset), len));
                } else if constexpr (has_annotation<padded_string, field>()) {
                    constexpr auto anno = get_annotation<padded_string, field>();
                    std::string_view sv(reinterpret_cast<const char*>(buffer_.data() + offset), anno.len);
                    size_t end_pos = sv.find_last_not_of(anno.pad);
                    if (end_pos != std::string_view::npos) {
                        return std::optional(sv.substr(0, end_pos + 1));
                    }
                    return std::optional(std::string_view(sv.data(), 0));
                } else if constexpr (has_annotation<null_terminated_string, field>()) {
                    constexpr auto anno = get_annotation<null_terminated_string, field>();
                    std::string_view sv(reinterpret_cast<const char*>(buffer_.data() + offset), anno.max_len);
                    size_t null_pos = sv.find('\0');
                    if (null_pos != std::string_view::npos) {
                        sv = sv.substr(0, null_pos);
                    }
                    return std::optional(sv);
                } else if constexpr (std::is_arithmetic_v<FieldType> || std::is_enum_v<FieldType>) {
                    FieldType v;
                    std::memcpy(&v, buffer_.data() + offset, sizeof(FieldType));
                    
                    if constexpr (needs_byteswap<field>()) {
                        if constexpr (std::is_enum_v<FieldType>) {
                            using UnderType = std::underlying_type_t<FieldType>;
                            v = static_cast<FieldType>(byteswap(static_cast<UnderType>(v)));
                        } else {
                            v = byteswap(v);
                        }
                    }
                    
                    if constexpr (has_annotation<fixed_point, field>()) {
                        constexpr auto anno = get_annotation<fixed_point, field>();
                        return std::optional(static_cast<double>(v) / detail::pow10_constexpr(anno.decimal_places));
                    } else {
                        return std::optional(v);
                    }
                } else if constexpr (std::is_array_v<FieldType> && std::is_same_v<std::remove_extent_t<FieldType>, char>) {
                    return std::optional(std::string_view(reinterpret_cast<const char*>(buffer_.data() + offset), len));
                } else if constexpr (std::is_array_v<FieldType>) {
                    return std::optional(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(buffer_.data() + offset), len));
                } else {
                    return std::optional(FieldType{});
                }
            } else {
                size_t offset = get_field_offset<field>();
                
                if constexpr (is_variable_field<field>()) {
                    if constexpr (has_annotation<non_fixed_bitmap, field>()) {
                        size_t var_idx = get_var_field_index<field>();
                        size_t size = var_sizes_[var_idx];
                        
                        constexpr auto BM = get_annotation<non_fixed_bitmap, field>().bitmask_field;
                        constexpr int bit_index = get_annotation<non_fixed_bitmap, field>().bit_index;
                        auto bitmap_val = this->template _get_fixed<BM>(get_field_offset<BM>());
                        bool is_present = (bitmap_val & (1ULL << bit_index)) != 0;
                        
                        if constexpr (has_annotation<length_precedes, field>()) {
                            using BaseRet = std::conditional_t<std::is_same_v<FieldType, std::span<const uint8_t>>,
                                std::span<const uint8_t>, std::string_view>;
                            if (!is_present) return std::optional<BaseRet>{};
                            
                            if constexpr (std::is_same_v<FieldType, std::span<const uint8_t>>) {
                                return std::optional<BaseRet>(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(buffer_.data() + offset), size));
                            } else {
                                return std::optional<BaseRet>(std::string_view(reinterpret_cast<const char*>(buffer_.data() + offset), size));
                            }
                        } else {
                            using RetType = std::optional<decltype(this->template _get_fixed<field>(offset))>;
                            if (!is_present) {
                                return RetType{};
                            } else {
                                return RetType{this->template _get_fixed<field>(offset)};
                            }
                        }
                    } else if constexpr (has_annotation<length_precedes, field>()) {
                        size_t var_idx = get_var_field_index<field>();
                        size_t size = var_sizes_[var_idx];
                        if constexpr (std::is_same_v<FieldType, std::span<const uint8_t>>) {
                            return std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(buffer_.data() + offset), size);
                        } else {
                            return std::string_view(reinterpret_cast<const char*>(buffer_.data() + offset), size);
                        }
                    } else {
                        return _get_fixed<field>(offset);
                    }
                } else {
                    return _get_fixed<field>(offset);
                }
            }
        }
        
        template <std::meta::info field, typename T>
        void _set_fixed(size_t offset, T&& val) {
            constexpr auto type_info = std::meta::type_of(field);
            using FieldType = typename [: type_info :];
            
            if constexpr (has_annotation<padded_string, field>()) {
                constexpr auto anno = get_annotation<padded_string, field>();
                std::string_view sv{val};
                size_t to_copy = std::min<size_t>(sv.size(), anno.len);
                std::memcpy(buffer_.data() + offset, sv.data(), to_copy);
                if (to_copy < anno.len) {
                    std::memset(buffer_.data() + offset + to_copy, anno.pad, anno.len - to_copy);
                }
            } else if constexpr (has_annotation<null_terminated_string, field>()) {
                constexpr auto anno = get_annotation<null_terminated_string, field>();
                std::string_view sv{val};
                size_t to_copy = std::min<size_t>(sv.size(), anno.max_len - 1);
                std::memcpy(buffer_.data() + offset, sv.data(), to_copy);
                std::memset(buffer_.data() + offset + to_copy, 0, anno.max_len - to_copy);
            } else if constexpr (std::is_arithmetic_v<FieldType> || std::is_enum_v<FieldType>) {
                FieldType v;
                if constexpr (has_annotation<fixed_point, field>()) {
                    constexpr auto anno = get_annotation<fixed_point, field>();
                    v = static_cast<FieldType>(std::round(val * detail::pow10_constexpr(anno.decimal_places)));
                } else {
                    v = static_cast<FieldType>(val);
                }
                
                if constexpr (needs_byteswap<field>()) {
                    if constexpr (std::is_enum_v<FieldType>) {
                        using UnderType = std::underlying_type_t<FieldType>;
                        v = static_cast<FieldType>(byteswap(static_cast<UnderType>(v)));
                    } else {
                        v = byteswap(v);
                    }
                }
                std::memcpy(buffer_.data() + offset, &v, sizeof(FieldType));
            }
        }
        
        template <std::meta::info field, typename T>
        requires (!ForReading)
        auto set(T&& val) {
            using FieldType = typename [: std::meta::type_of(field) :];
            
            if constexpr (is_tlv_field<field>()) {
                size_t idx = get_tlv_field_index<field>();
                constexpr auto tanno = get_annotation<tlv_appendage, field>();
                
                size_t req_size = 0;
                if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::span<const uint8_t>>) {
                    req_size = val.size();
                } else if constexpr (std::is_convertible_v<T, std::string_view>) {
                    req_size = std::string_view{val}.size();
                } else {
                    req_size = sizeof(FieldType);
                }
                
                size_t old_len = tlv_sizes_[idx];
                size_t old_offset = tlv_offsets_[idx];
                
                if (old_offset == 0) {
                    size_t new_offset = total_size_;
                    size_t frame_size = 2 + req_size;
                    if (new_offset + frame_size > buffer_.size()) {
                        return std::expected<void, const char*>(std::unexpected("Buffer overflow"));
                    }
                    
                    buffer_[new_offset] = static_cast<char>(req_size + 1);
                    buffer_[new_offset + 1] = static_cast<char>(tanno.tag);
                    
                    tlv_offsets_[idx] = new_offset + 2;
                    tlv_sizes_[idx] = req_size;
                    total_size_ += frame_size;
                    
                    static constexpr auto mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
                    template for (constexpr auto m : mems) {
                        if constexpr (std::meta::is_nonstatic_data_member(m)) {
                            if constexpr (has_annotation<tlv_region_length, m>()) {
                                auto cur_len = this->template _get_fixed<m>(this->template get_field_offset<m>());
                                this->template _set_fixed<m>(this->template get_field_offset<m>(), cur_len + frame_size);
                            }
                        }
                    }
                    
                    if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::span<const uint8_t>>) {
                        std::memcpy(buffer_.data() + new_offset + 2, val.data(), req_size);
                    } else if constexpr (std::is_convertible_v<T, std::string_view>) {
                        std::memcpy(buffer_.data() + new_offset + 2, std::string_view{val}.data(), req_size);
                    } else {
                        this->template _set_fixed<field>(new_offset + 2, std::forward<T>(val));
                    }
                } else {
                    intptr_t diff = static_cast<intptr_t>(req_size) - static_cast<intptr_t>(old_len);
                    if (diff != 0) {
                        if (total_size_ + diff > buffer_.size()) {
                            return std::expected<void, const char*>(std::unexpected("Buffer overflow"));
                        }
                        size_t downstream_start = old_offset + old_len;
                        size_t downstream_len = total_size_ - downstream_start;
                        if (downstream_len > 0) {
                            std::memmove(buffer_.data() + downstream_start + diff, buffer_.data() + downstream_start, downstream_len);
                        }
                        
                        for (size_t i = 0; i < NumTLVFields; i++) {
                            if (tlv_offsets_[i] >= downstream_start) {
                                tlv_offsets_[i] += diff;
                            }
                        }
                        
                        buffer_[old_offset - 2] = static_cast<char>(req_size + 1);
                        tlv_sizes_[idx] = req_size;
                        total_size_ += diff;
                        
                        static constexpr auto mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
                        template for (constexpr auto m : mems) {
                            if constexpr (std::meta::is_nonstatic_data_member(m)) {
                                if constexpr (has_annotation<tlv_region_length, m>()) {
                                    auto cur_len = this->template _get_fixed<m>(this->template get_field_offset<m>());
                                    this->template _set_fixed<m>(this->template get_field_offset<m>(), cur_len + diff);
                                }
                            }
                        }
                    }
                    if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::span<const uint8_t>>) {
                        std::memcpy(buffer_.data() + old_offset, val.data(), req_size);
                    } else if constexpr (std::is_convertible_v<T, std::string_view>) {
                        std::memcpy(buffer_.data() + old_offset, std::string_view{val}.data(), req_size);
                    } else {
                        this->template _set_fixed<field>(old_offset, std::forward<T>(val));
                    }
                }
                return std::expected<void, const char*>{};
            }

            size_t offset = get_field_offset<field>();
            
            if constexpr (is_variable_field<field>()) {
                if constexpr (has_annotation<non_fixed_bitmap, field>()) {
                    constexpr auto BM = get_annotation<non_fixed_bitmap, field>().bitmask_field;
                    constexpr int bit_index = get_annotation<non_fixed_bitmap, field>().bit_index;
                    auto current_bitmask_val = this->template _get_fixed<BM>(this->template get_field_offset<BM>());
                    uint64_t current_bitmask = static_cast<uint64_t>(current_bitmask_val);
                    current_bitmask |= (1ULL << bit_index);
                    this->template _set_fixed<BM>(this->template get_field_offset<BM>(), current_bitmask);
                }
                
                size_t var_idx = get_var_field_index<field>();
                size_t old_size = var_sizes_[var_idx];
                
                if constexpr (has_annotation<length_precedes, field>()) {
                    size_t new_size;
                    if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::span<const uint8_t>>) {
                        new_size = val.size();
                    } else if constexpr (std::is_convertible_v<T, std::string_view>) {
                        new_size = std::string_view{val}.size();
                    } else {
                        new_size = sizeof(FieldType);
                    }
                    
                    if (new_size != old_size) {
                        intptr_t diff = static_cast<intptr_t>(new_size) - static_cast<intptr_t>(old_size);
                        
                        if (total_size_ + diff > buffer_.size()) {
                            return std::expected<void, const char*>(std::unexpected("Buffer overflow"));
                        }
                        
                        size_t end_of_current = offset + old_size;
                        if (total_size_ > end_of_current) {
                            std::memmove(buffer_.data() + offset + new_size,
                                         buffer_.data() + end_of_current,
                                         total_size_ - end_of_current);
                        }
                        
                        for (size_t i = 0; i < NumTLVFields; i++) {
                            if (tlv_offsets_[i] >= end_of_current) {
                                tlv_offsets_[i] += diff;
                            }
                        }
                        
                        var_sizes_[var_idx] = new_size;
                        total_size_ += diff;
                        
                        constexpr auto anno = get_annotation<length_precedes, field>();
                        constexpr std::string_view len_name = std::meta::identifier_of(anno.type_meta);
                        constexpr int size_adj = anno.size_adjustment;
                        
                        static constexpr auto mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
                        template for (constexpr auto m : mems) {
                            if constexpr (std::meta::is_nonstatic_data_member(m)) {
                                constexpr std::string_view name = std::meta::identifier_of(m);
                                if constexpr (name == len_name) {
                                    this->template _set_fixed<m>(this->template get_field_offset<m>(), new_size - size_adj);
                                }
                            }
                        }
                    }
                    
                    if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::span<const uint8_t>>) {
                        std::memcpy(buffer_.data() + offset, val.data(), new_size);
                    } else if constexpr (std::is_convertible_v<T, std::string_view>) {
                        std::memcpy(buffer_.data() + offset, std::string_view{val}.data(), new_size);
                    } else {
                        this->template _set_fixed<field>(offset, std::forward<T>(val));
                    }
                    
                    return std::expected<void, const char*>{};
                } else if constexpr (has_annotation<non_fixed_bitmap, field>()) {
                    size_t new_size = get_fixed_field_size<field>();
                    
                    if (new_size != old_size) {
                        intptr_t diff = new_size - old_size;
                        
                        if (total_size_ + diff > buffer_.size()) {
                            return std::expected<void, const char*>(std::unexpected("Buffer overflow"));
                        }
                        
                        size_t end_of_current = offset + old_size;
                        if (total_size_ > end_of_current) {
                            std::memmove(buffer_.data() + offset + new_size,
                                         buffer_.data() + end_of_current,
                                         total_size_ - end_of_current);
                        }
                        
                        for (size_t i = 0; i < NumTLVFields; i++) {
                            if (tlv_offsets_[i] >= end_of_current) {
                                tlv_offsets_[i] += diff;
                            }
                        }
                        
                        var_sizes_[var_idx] = new_size;
                        total_size_ += diff;
                        
                        static constexpr auto mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
                        template for (constexpr auto m : mems) {
                            if constexpr (std::meta::is_nonstatic_data_member(m)) {
                                if constexpr (has_annotation<tlv_region_length, m>()) {
                                    auto cur_len = this->template _get_fixed<m>(this->template get_field_offset<m>());
                                    this->template _set_fixed<m>(this->template get_field_offset<m>(), cur_len + diff);
                                }
                            }
                        }
                        
                    }
                    this->template _set_fixed<field>(offset, std::forward<T>(val));
                    return std::expected<void, const char*>{};
                } else {
                    return std::expected<void, const char*>(std::unexpected("Invalid variable field setup"));
                }
            } else {
                this->template _set_fixed<field>(offset, std::forward<T>(val));
            }
        }

        size_t get_tlv_size(size_t idx) const {
            return tlv_sizes_[idx];
        }

        template <std::meta::info field>
        requires (!ForReading)
        std::expected<void, const char*> remove() {
            if constexpr (is_tlv_field<field>()) {
                size_t idx        = get_tlv_field_index<field>();
                size_t old_offset = tlv_offsets_[idx];
                size_t old_len    = tlv_sizes_[idx];

                if (old_offset == 0) return {};  // already absent — no-op

                // Full TLV frame: [len_byte][tag_byte][value...]
                size_t frame_start = old_offset - 2;
                size_t frame_size  = 2 + old_len;

                // Slide everything after this frame backwards to fill the gap
                size_t downstream_start = frame_start + frame_size;
                size_t downstream_len   = total_size_ - downstream_start;
                if (downstream_len > 0) {
                    std::memmove(buffer_.data() + frame_start,
                                 buffer_.data() + downstream_start,
                                 downstream_len);
                }
                // Zero the vacated tail so the buffer is deterministic
                std::memset(buffer_.data() + total_size_ - frame_size, 0, frame_size);

                total_size_ -= frame_size;

                // Adjust offsets of all TLV fields that were located after this frame
                for (size_t i = 0; i < NumTLVFields; i++) {
                    if (tlv_offsets_[i] >= downstream_start) {
                        tlv_offsets_[i] -= frame_size;
                    }
                }

                // Clear this field's tracking slot
                tlv_offsets_[idx] = 0;
                tlv_sizes_[idx]   = 0;

                // Decrement the region-length counter stored in the fixed header
                static constexpr auto mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
                template for (constexpr auto m : mems) {
                    if constexpr (std::meta::is_nonstatic_data_member(m)) {
                        if constexpr (has_annotation<tlv_region_length, m>()) {
                            auto cur = this->template _get_fixed<m>(this->template get_field_offset<m>());
                            this->template _set_fixed<m>(this->template get_field_offset<m>(), cur - frame_size);
                        }
                    }
                }
                return {};
            } else if constexpr (has_annotation<non_fixed_bitmap, field>()) {
                size_t var_idx = get_var_field_index<field>();
                size_t old_size = var_sizes_[var_idx];
                
                if (old_size == 0) return {}; // already absent - no-op
                
                size_t offset = get_field_offset<field>();
                
                size_t downstream_start = offset + old_size;
                size_t downstream_len = total_size_ - downstream_start;
                if (downstream_len > 0) {
                    std::memmove(buffer_.data() + offset,
                                 buffer_.data() + downstream_start,
                                 downstream_len);
                }
                
                std::memset(buffer_.data() + total_size_ - old_size, 0, old_size);
                
                for (size_t i = 0; i < NumTLVFields; i++) {
                    if (tlv_offsets_[i] >= downstream_start) {
                        tlv_offsets_[i] -= old_size;
                    }
                }
                
                var_sizes_[var_idx] = 0;
                total_size_ -= old_size;
                
                static constexpr auto mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
                template for (constexpr auto m : mems) {
                    if constexpr (std::meta::is_nonstatic_data_member(m)) {
                        if constexpr (has_annotation<tlv_region_length, m>()) {
                            auto cur = this->template _get_fixed<m>(this->template get_field_offset<m>());
                            this->template _set_fixed<m>(this->template get_field_offset<m>(), cur - old_size);
                        }
                    }
                }
                
                constexpr auto anno = get_annotation<non_fixed_bitmap, field>();
                constexpr std::string_view bitmask_name = std::meta::identifier_of(anno.bitmask_field);
                constexpr int bit_index = anno.bit_index;
                
                template for (constexpr auto m : mems) {
                    if constexpr (std::meta::is_nonstatic_data_member(m)) {
                        constexpr std::string_view name = std::meta::identifier_of(m);
                        if constexpr (name == bitmask_name) {
                            auto current_bitmask_val = this->template _get_fixed<m>(this->template get_field_offset<m>());
                            uint64_t current_bitmask = static_cast<uint64_t>(current_bitmask_val);
                            current_bitmask &= ~(1ULL << bit_index);
                            this->template _set_fixed<m>(this->template get_field_offset<m>(), current_bitmask);
                        }
                    }
                }
                return {};
            } else {
                return std::unexpected("remove() only valid for tlv_appendage or non_fixed_bitmap fields");
            }
        }

    };
    
    template <std::meta::info field, typename GeneratedProxy, typename Definition, bool ForReading>
    struct FieldProxy {
        auto parent() const {
            // Find offset of this field in GeneratedProxy
            static constexpr auto proxy_mems = std::define_static_array(std::meta::members_of(^^GeneratedProxy, std::meta::access_context::current()));
            size_t offset = 0;
            template for (constexpr auto pm : proxy_mems) {
                if constexpr (std::meta::is_nonstatic_data_member(pm)) {
                    if constexpr (std::meta::identifier_of(pm) == std::meta::identifier_of(field)) {
                        offset = std::meta::offset_of(pm).bytes;
                    }
                }
            }
            const char* self_ptr = reinterpret_cast<const char*>(this);
            return reinterpret_cast<const GeneratedProxy*>(self_ptr - offset);
        }
        
        auto parent() {
            static constexpr auto proxy_mems = std::define_static_array(std::meta::members_of(^^GeneratedProxy, std::meta::access_context::current()));
            size_t offset = 0;
            template for (constexpr auto pm : proxy_mems) {
                if constexpr (std::meta::is_nonstatic_data_member(pm)) {
                    if constexpr (std::meta::identifier_of(pm) == std::meta::identifier_of(field)) {
                        offset = std::meta::offset_of(pm).bytes;
                    }
                }
            }
            char* self_ptr = reinterpret_cast<char*>(this);
            return reinterpret_cast<GeneratedProxy*>(self_ptr - offset);
        }
        
        auto get() const {
            return parent()->state_.template get<field>();
        }
        
        template <typename T>
        requires (!ForReading)
        auto operator=(T&& val) {
            return parent()->state_.template set<field>(std::forward<T>(val));
        }

        auto remove() requires (!ForReading) {
            return parent()->state_.template remove<field>();
        }
        
        operator auto() const {
            return get();
        }
    };

template <std::meta::info m, typename Definition>
consteval bool is_bitmap_field() {
    bool result = false;
    static constexpr auto mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
    template for (constexpr auto other_m : mems) {
        if constexpr (has_annotation<non_fixed_bitmap, other_m>()) {
            if constexpr (get_annotation<non_fixed_bitmap, other_m>().bitmask_field == m) {
                result = true;
            }
        }
    }
    return result;
}

template <typename T>
void write_json_val(std::ostream& os, const T& val, const JsonOptions& options) {
    if constexpr (std::is_same_v<T, std::span<const uint8_t>>) {
        os << "[";
        bool first_elem = true;
        for (uint8_t byte : val) {
            if (!first_elem) os << ",";
            first_elem = false;
            os << static_cast<int>(byte);
        }
        os << "]";
    } else if constexpr (std::is_convertible_v<T, std::string_view>) {
        aitvaras::detail::escape_json_string(os, std::string_view{val});
    } else if constexpr (std::is_enum_v<T>) {
        if (options.raw_enums) {
            os << static_cast<std::underlying_type_t<T>>(val);
        } else {
            bool found = false;
            static constexpr auto enums = std::define_static_array(std::meta::enumerators_of(^^T));
            template for (constexpr auto e : enums) {
                if (std::meta::extract<T>(e) == val) {
                    aitvaras::detail::escape_json_string(os, std::string(std::meta::identifier_of(e)) + "(" + std::to_string(static_cast<std::underlying_type_t<T>>(val)) + ")");
                    found = true;
                }
            }
            if (!found) {
                os << static_cast<std::underlying_type_t<T>>(val);
            }
        }
    } else if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, int8_t>) {
        os << static_cast<int>(val);
    } else if constexpr (std::is_arithmetic_v<T>) {
        os << val;
    } else {
        os << "\"<unsupported_type>\"";
    }
}

} // namespace detail

template <typename Definition, bool ForReading = false>
consteval auto generate_marshaler_type() {
    struct GeneratedProxy;
    consteval {
        std::vector<std::meta::info> members;
        
        members.push_back(std::meta::data_member_spec(^^detail::MarshalerState<Definition, ForReading>, {.name = "state_"}));
        
        static constexpr auto def_mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
        template for (constexpr auto m : def_mems) {
            if constexpr (std::meta::is_nonstatic_data_member(m)) {
                using FProxy = detail::FieldProxy<m, GeneratedProxy, Definition, ForReading>;
                members.push_back(std::meta::data_member_spec(^^FProxy, {.name = std::meta::identifier_of(m)}));
            }
        }
        
        std::meta::define_aggregate(^^GeneratedProxy, members);
    }
    
    struct Wrapper : GeneratedProxy {
        void to_json(std::ostream& os, const JsonOptions& options = {}) const {
            os << "{";
            bool first = true;
            static constexpr auto def_mems = std::define_static_array(std::meta::members_of(^^Definition, std::meta::access_context::current()));
            template for (constexpr auto m : def_mems) {
                if constexpr (std::meta::is_nonstatic_data_member(m)) {
                    constexpr bool is_optional = detail::has_annotation<non_fixed_bitmap, m>() || detail::has_annotation<tlv_appendage, m>();
                    
                    bool present = true;
                    if constexpr (is_optional) {
                        auto val_expected = this->state_.template get<m>();
                        if (!val_expected.has_value()) {
                            present = false;
                        }
                    }
                    
                    if (present) {
                        if (!first) os << ",";
                        first = false;
                        
                        aitvaras::detail::escape_json_string(os, std::meta::identifier_of(m));
                        os << ":";
                        
                        if constexpr (is_optional) {
                            auto val = *this->state_.template get<m>();
                            detail::write_json_val(os, val, options);
                        } else {
                            auto val = this->state_.template get<m>();
                            detail::write_json_val(os, val, options);
                        }
                        
                        constexpr bool is_bitmap = detail::is_bitmap_field<m, Definition>();
                        
                        if constexpr (is_bitmap) {
                            auto bitmap_val = this->state_.template get<m>();
                            os << ",\"";
                            os << std::meta::identifier_of(m);
                            os << "_present\":[";
                            bool first_bm = true;
                            template for (constexpr auto other_m : def_mems) {
                                if constexpr (detail::has_annotation<non_fixed_bitmap, other_m>()) {
                                    constexpr auto annot = detail::get_annotation<non_fixed_bitmap, other_m>();
                                    if constexpr (annot.bitmask_field == m) {
                                        if ((static_cast<uint64_t>(bitmap_val) & (1ULL << annot.bit_index)) != 0) {
                                            if (!first_bm) os << ",";
                                            first_bm = false;
                                            aitvaras::detail::escape_json_string(os, std::meta::identifier_of(other_m));
                                        }
                                    }
                                }
                            }
                            os << "]";
                        }
                        
                        if constexpr (detail::has_annotation<tlv_appendage, m>()) {
                            if (options.detailed_verbosity) {
                                size_t idx = this->state_.template get_tlv_field_index<m>();
                                size_t len = this->state_.get_tlv_size(idx);
                                os << ",\"" << std::meta::identifier_of(m) << "_length\":" << len;
                            }
                        }
                    }
                }
            }
            os << "}";
        }
    };
    return std::type_identity<Wrapper>{};
}

template <typename Definition, bool ForReading = false>
using Marshaler = typename decltype(generate_marshaler_type<Definition, ForReading>())::type;

} // namespace aitvaras

#endif // AITVARAS_MARSHALER_HPP

/*
 * TAR file header parser functions.
 *
 * Copyright (C) 2022 Matthew T. Bucknall
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <string.h>

#include "tar.h"


#ifdef TAR_DEBUG
#include <assert.h>
#define TAR_ASSERT(expr)    assert(expr)
#else
#define TAR_ASSERT(expr)    do {} while(0)
#endif


static size_t strnlen_(const char* s, size_t maxlen) {
    size_t len;

    for (len = 0; len < maxlen; len++) {
        if ( s[len] == '\0' ) {
            break;
        }
    }

    return len;
}


static uint64_t parse_octal(const uint8_t* field, size_t n_bytes) {
    const uint8_t* const buffer_e = field + n_bytes;
    uint64_t value = 0;

    while(field < buffer_e) {
        uint8_t b = *field++;

        if ( b == ' ' ) {
            continue;
        } else if ( b >= '0' && b <= '7' ) {
            value = (8 * value) + (b - '0');
        } else {
            break;
        }
    }

    return value;
}


static const char* parse_string(const uint8_t* field, size_t n_bytes, tar_string_buffer_t* buffer) {
    size_t len;

    len = strnlen_((const char*) field, n_bytes);
    memcpy(buffer->str, field, len);
    buffer->str[len] = '\0';

    return buffer->str;
}


bool tar_validate_header(const tar_header_t* header) {
    TAR_ASSERT(header);

    uint32_t checksum_unsigned;
    uint32_t checksum_signed;
    uint32_t checksum_parsed;

    // check ustar indicator is present
    if ( memcmp(header->magic, "ustar", 5) != 0 ) {
        return false;
    }

    if ( header->magic[5] != '\0' && header->magic[5] != ' ' ) {
        return false;
    }

    // check ustar version is 0
    if (parse_octal(header->version, 2) != 0 ) {
        return false;
    }

    // calculate unsigned checksum
    checksum_unsigned = 256;

    for (unsigned int i = 0; i < 148; i++) {
        checksum_unsigned += ((const uint8_t*) header)[i];
    }

    for (unsigned int i = 156; i < 500; i++) {
        checksum_unsigned += ((const uint8_t*) header)[i];
    }

    checksum_signed = 256;

    for (unsigned int i = 0; i < 148; i++) {
        checksum_signed += ((const int8_t*) header)[i];
    }

    for (unsigned int i = 156; i < 500; i++) {
        checksum_signed += ((const int8_t*) header)[i];
    }

    // validate checksum
    checksum_parsed = parse_octal(header->checksum, 8);

    return (checksum_parsed == checksum_unsigned || checksum_parsed == checksum_signed);
}


int tar_get_mode(const tar_header_t* header) {
    TAR_ASSERT(header);
    return (int) parse_octal(header->mode, 8);
}


uint32_t tar_get_uid(const tar_header_t* header) {
    TAR_ASSERT(header);
    return (uint32_t) parse_octal(header->uid, 8);
}


uint32_t tar_get_gid(const tar_header_t* header) {
    TAR_ASSERT(header);
    return (uint32_t) parse_octal(header->gid, 8);
}


uint64_t tar_get_size(const tar_header_t* header) {
    TAR_ASSERT(header);
    return parse_octal(header->size, 12);
}


int64_t tar_get_modify_time(const tar_header_t* header) {
    TAR_ASSERT(header);
    return (int64_t) parse_octal(header->modify_time, 12);
}


tar_type_t tar_get_type(const tar_header_t* header) {
    TAR_ASSERT(header);

    if ( header->type >= '0' && header->type <= '6' ) {
        return (tar_type_t) (header->type);
    } else {
        return TAR_TYPE_UNSUPPORTED;
    }
}


const char* tar_get_filename(const tar_header_t* header, tar_string_buffer_t* buffer) {
    TAR_ASSERT(header);
    TAR_ASSERT(buffer);

    uint32_t prefix_len;
    uint32_t suffix_len;

    // get length of filename prefix
    prefix_len = strnlen_((const char*) (header->filename_prefix), 155);

    // copy prefix into buffer
    memcpy(buffer->str, header->filename_prefix, prefix_len);

    // get length of filename suffix
    suffix_len = strnlen((const char*) (header->filename), 100);

    // copy suffix into buffer
    memcpy(buffer->str + prefix_len, header->filename, suffix_len);

    // ensure filename is null-terminated
    buffer->str[prefix_len + suffix_len] = '\0';

    return buffer->str;
}


const char* tar_get_linked_filename(const tar_header_t* header, tar_string_buffer_t* buffer) {
    TAR_ASSERT(header);
    return parse_string(header->linked_filename, 100, buffer);
}


const char* tar_get_owner_name(const tar_header_t* header, tar_string_buffer_t* buffer) {
    TAR_ASSERT(header);
    return parse_string(header->owner_name, 32, buffer);
}


const char* tar_get_group_name(const tar_header_t* header, tar_string_buffer_t* buffer) {
    TAR_ASSERT(header);
    return parse_string(header->group_name, 32, buffer);
}


uint32_t tar_get_device_major_number(const tar_header_t* header) {
    TAR_ASSERT(header);
    return (uint32_t) parse_octal(header->device_major, 8);
}


uint32_t tar_get_device_minor_number(const tar_header_t* header) {
    TAR_ASSERT(header);
    return (uint32_t) parse_octal(header->device_minor, 8);
}

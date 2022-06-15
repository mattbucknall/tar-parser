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

#ifndef _TAR_H_
#define _TAR_H_

#include <stdbool.h>
#include <stdint.h>


#define TAR_MODE_USER_READ          (1 << 8)
#define TAR_MODE_USER_WRITE         (1 << 7)
#define TAR_MODE_USER_EXECUTE       (1 << 6)
#define TAR_MODE_GROUP_READ         (1 << 5)
#define TAR_MODE_GROUP_WRITE        (1 << 4)
#define TAR_MODE_GROUP_EXECUTE      (1 << 3)
#define TAR_MODE_OTHER_READ         (1 << 2)
#define TAR_MODE_OTHER_WRITE        (1 << 1)
#define TAR_MODE_OTHER_EXECUTE      (1 << 0)


typedef enum {
    TAR_TYPE_NORMAL_FILE = '0',
    TAR_TYPE_HARD_LINK = '1',
    TAR_TYPE_SYMBOLIC_LINK = '2',
    TAR_TYPE_CHARACTER_SPECIAL = '3',
    TAR_TYPE_BLOCK_SPECIAL = '4',
    TAR_TYPE_DIRECTORY = '5',
    TAR_TYPE_FIFO = '6',
    TAR_TYPE_UNSUPPORTED = '?'
} tar_type_t;


typedef struct {
    uint8_t filename[100];
    uint8_t mode[8];
    uint8_t uid[8];
    uint8_t gid[8];
    uint8_t size[12];
    uint8_t modify_time[12];
    uint8_t checksum[8];
    uint8_t type;
    uint8_t linked_filename[100];
    uint8_t magic[6];
    uint8_t version[2];
    uint8_t owner_name[32];
    uint8_t group_name[32];
    uint8_t device_major[8];
    uint8_t device_minor[8];
    uint8_t filename_prefix[155];
    uint8_t padding[12];
} tar_header_t;


typedef struct {
    char str[256];
} tar_string_buffer_t;


bool tar_validate_header(const tar_header_t* header);

int tar_get_mode(const tar_header_t* header);

uint32_t tar_get_uid(const tar_header_t* header);

uint32_t tar_get_gid(const tar_header_t* header);

uint64_t tar_get_size(const tar_header_t* header);

int64_t tar_get_modify_time(const tar_header_t* header);

tar_type_t tar_get_type(const tar_header_t* header);

const char* tar_get_filename(const tar_header_t* header, tar_string_buffer_t* buffer);

const char* tar_get_linked_filename(const tar_header_t* header, tar_string_buffer_t* buffer);

const char* tar_get_owner_name(const tar_header_t* header, tar_string_buffer_t* buffer);

const char* tar_get_group_name(const tar_header_t* header, tar_string_buffer_t* buffer);

uint32_t tar_get_device_major_number(const tar_header_t* header);

uint32_t tar_get_device_minor_number(const tar_header_t* header);

#endif // _TAR_H_

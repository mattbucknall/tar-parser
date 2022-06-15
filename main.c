/*
 * TAR file header parser functions.
 *
 * This file provides a crude demonstration of how to use the functions
 * declared in tar.h
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <inttypes.h>

#include <limits.h>

#include "tar.h"


int main(int argc, char* argv[]) {
    FILE* f;
    tar_header_t header;

    // check filename has been passed to executable
    if (argc < 2) {
        fprintf(stderr, "No input file path provided\n");
        return EXIT_FAILURE;
    }

    // open file
    f = fopen(argv[1], "rb");

    if (!f) {
        fprintf(stderr, "Cannot open file\n");
        return EXIT_FAILURE;
    }

    // traverse file, parsing tar headers and writing their decoded fields to stdout
    while (fread(&header, sizeof(header), 1, f)) {
        tar_type_t type;
        char type_char;
        int mode;
        char user_read_char;
        char user_write_char;
        char user_exec_char;
        char group_read_char;
        char group_write_char;
        char group_exec_char;
        char other_read_char;
        char other_write_char;
        char other_exec_char;
        tar_string_buffer_t buffer;
        uint64_t size;

        // validate header
        if (!tar_validate_header(&header)) {
            break;
        }

        // determine file type/mode characters
        type = tar_get_type(&header);

        switch (type) {
        case TAR_TYPE_NORMAL_FILE:
        case TAR_TYPE_HARD_LINK:
            type_char = '-';
            break;

        case TAR_TYPE_SYMBOLIC_LINK:
            type_char = 'l';
            break;

        case TAR_TYPE_CHARACTER_SPECIAL:
            type_char = 'c';
            break;

        case TAR_TYPE_BLOCK_SPECIAL:
            type_char = 'b';
            break;

        case TAR_TYPE_DIRECTORY:
            type_char = 'd';
            break;

        case TAR_TYPE_FIFO:
            type_char = 'p';
            break;

        default:
            type_char = '?';
        }

        mode = tar_get_mode(&header);

        user_read_char = (mode & TAR_MODE_USER_READ) ? 'r' : '-';
        user_write_char = (mode & TAR_MODE_USER_WRITE) ? 'w' : '-';
        user_exec_char = (mode & TAR_MODE_USER_EXECUTE) ? 'x' : '-';

        group_read_char = (mode & TAR_MODE_GROUP_READ) ? 'r' : '-';
        group_write_char = (mode & TAR_MODE_GROUP_WRITE) ? 'w' : '-';
        group_exec_char = (mode & TAR_MODE_GROUP_EXECUTE) ? 'x' : '-';

        other_read_char = (mode & TAR_MODE_OTHER_READ) ? 'r' : '-';
        other_write_char = (mode & TAR_MODE_OTHER_WRITE) ? 'w' : '-';
        other_exec_char = (mode & TAR_MODE_OTHER_EXECUTE) ? 'x' : '-';

        // output type/mode characters
        printf("%c%c%c%c%c%c%c%c%c%c ",
               type_char,
               user_read_char,
               user_write_char,
               user_exec_char,
               group_read_char,
               group_write_char,
               group_exec_char,
               other_read_char,
               other_write_char,
               other_exec_char);

        // output owner/group names
        printf("%12s ", tar_get_owner_name(&header, &buffer));
        printf("%12s ", tar_get_group_name(&header, &buffer));

        if (type == TAR_TYPE_CHARACTER_SPECIAL || type == TAR_TYPE_BLOCK_SPECIAL) {
            // output major/minor device numbers if type is special
            printf("%8" PRIu32 ", %8" PRIu32 " ", tar_get_device_major_number(&header),
                   tar_get_device_minor_number(&header));
        } else {
            // otherwise output file size
            printf("%8" PRIu64 " ", tar_get_size(&header));
        }

        // output filename
        printf("%s", tar_get_filename(&header, &buffer));

        // if this is a symbol link, output linked filename
        if (type == TAR_TYPE_SYMBOLIC_LINK) {
            printf(" -> %s", tar_get_linked_filename(&header, &buffer));
        }

        // output newline
        putchar('\n');

        // round size up to nearest 512 byte boundary
        size = tar_get_size(&header);
        size = (size + 511) & (~(uint64_t) 511);

        // skip over file data
        while(size > 0) {
            long int offset;

            if ( size > LONG_MAX ) {
                offset = LONG_MAX;
            } else {
                offset = (long int) size;
            }

            fseek(f, offset, SEEK_CUR);

            size -= offset;
        }
    }

    // close file
    fclose(f);

    // done
    return EXIT_SUCCESS;
}

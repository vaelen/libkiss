/*
Copyright (c) 2022 Andrew C. Young (JJ1OKA / NU8W)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <kiss.h>

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

extern int errno;

// Encodes a single packet
int main(int argc, char *argv[]) {
    // Open our input file, or use STDIN if no file was provided
    int file = STDIN_FILENO;
    if (argc > 1) {
        int fd = open(argv[1], O_RDONLY);
        if (fd ==-1) {
            perror(argv[0]);
            return 1;
        }
        file = fd;
    }

    size_t buffer_capacity = 32;
    uint8_t buffer[buffer_capacity];
    uint8_t output_buffer[buffer_capacity * 2];
    kiss_packet_t p = kiss_new_packet(buffer, buffer_capacity);

    ssize_t bytes_read = 1;
    while (bytes_read > 0) {
        bytes_read = read(file, buffer, buffer_capacity);
        if (bytes_read < 0 && errno != 0) {
            // An error occured
            perror(argv[0]);
            return 1;
        }
        if (bytes_read > 0) {
            p.data_length = bytes_read;
            size_t bytes_written = kiss_encode_packet(p, output_buffer, buffer_capacity * 2);
            write(STDOUT_FILENO, output_buffer, bytes_written);
        }
    }

    return 0;
}
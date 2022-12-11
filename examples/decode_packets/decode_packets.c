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

/**
 * This is the number of bytes to drop when the buffer fills up and
 * we still haven't found a packet. Dropping a single byte gives us
 * the best chance to find a new packet, but it is also the most 
 * time consuming because in a worst case scenario we will shift 
 * the whole buffer on each read operation. We drop 32 bytes here,
 * but your results may vary.
*/
#define BYTES_TO_DROP 32

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

    /**
     * Start with an empty buffer to hold incoming encoded data.
          * 
     * The buffer should be able to hold the largest message you expect to receive.
     * The KISS standard suggests that this buffer not be artificially limited in size,
     * but in reality the size depends on the underlying protocol.
     * 
     * The AX.25 standard states that:
     *      The default maximum number of octets allowed in the I field is 256. 
     *      This variable is negotiable between end stations. 
     */
    size_t buffer_capacity = 1024;
    size_t buffer_length = 0;
    uint8_t buffer[buffer_capacity];

    /** 
     * We need another similar buffer to hold the decoded packet data.
     * This buffer will be used internally by the packet, but we create it here so 
     * that the library doesn't allocate any new memory.
     */
    size_t packet_buffer_capacity = buffer_capacity;
    uint8_t packet_buffer[packet_buffer_capacity];

    // Create a packet data structure to hold the incoming packets
    kiss_packet_t packet = kiss_new_packet(packet_buffer, packet_buffer_capacity);

    ssize_t bytes_read = 1;
    while (bytes_read > 0) {
        // Fill remaining buffer
        bytes_read = read(file, buffer + buffer_length, buffer_capacity - buffer_length);
        if (bytes_read < 0 && errno != 0) {
            // An error occured
            perror(argv[0]);
            return 1;
        }
        if (bytes_read > 0) {
            // Increase the length of the buffer by the number of bytes we read
            buffer_length += bytes_read;
            // Try to parse a packet from what we have so far
            size_t bytes_consumed = kiss_decode_packet(&packet, buffer, buffer_length);
            if (bytes_consumed > 0) {
                // Remove the consumed bytes
                buffer_length -= bytes_consumed;
                memcpy(buffer, buffer + bytes_consumed, buffer_length);
            }

            // Check for a complete packet, and print it to the console.
            if (packet.complete_packet) {
                printf("Port: %d, Command: %s, Data: ", packet.port, kiss_command_name(packet.command));
                for (size_t i = 0; i < packet.data_length; i++) {
                    uint8_t b = packet.data[i];
                    if (b >= 20 && b <= 126) {
                        // Printable ASCII character
                        printf("%c", b);
                    } else {
                        // Non-printable character
                        printf("<0x%02X>", b);
                    }
                }
                printf("\n");
                // Reset packet for next use
                kiss_clear_packet(&packet);
            }

            // If the buffer is full, we need to drop bytes to make room for new bytes.
            if (buffer_length == buffer_capacity) {
                buffer_length -= BYTES_TO_DROP;
                memcpy(buffer, buffer + BYTES_TO_DROP, buffer_length);
            }
        }
    }

    return 0;
}
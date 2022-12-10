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

#pragma once

#include <stdint.h>
#include <stddef.h>

#define KISS_FRAME_END      0xC0
#define KISS_FRAME_ESCAPE   0xDB
#define KISS_ESCAPE_FEND    0xDC
#define KISS_ESCAPE_FESC    0xDD

#ifdef __cplusplus
extern "C"
{
#endif

enum kiss_command {
    KISS_DATA_FRAME = 0,
    KISS_TX_DELAY = 1,
    KISS_PERSISTENCE = 2,
    KISS_SLOT_TIME = 3,
    KISS_TX_TAIL = 4,
    KISS_FULL_DUPLEX = 5,
    KISS_SET_HARDWARE = 6,
    KISS_RETURN = 127,
};

typedef enum kiss_command kiss_command_t;

struct kiss_packet {
    kiss_command_t command;
    uint8_t port;
    uint8_t *data;
    size_t data_length;
    size_t data_capacity;
};
typedef struct kiss_packet kiss_packet_t;

// Encode packet data
size_t kiss_encode_data(uint8_t *decoded, size_t decoded_length, uint8_t *encoded, size_t encoded_length);

// Decode packet data
size_t kiss_decode_data(uint8_t *encoded, size_t data_length, uint8_t *buffer, size_t buffer_length);

// Encode the command and port in a single byte
uint8_t kiss_encode_command(kiss_command_t command, uint8_t port);

// Decode the command and port from a single byte
void kiss_decode_command(uint8_t b, kiss_command_t *command, uint8_t *port);

// Encode a packet, returns bytes written to buffer
size_t kiss_encode_packet(kiss_packet_t p, uint8_t *buffer, size_t buffer_size);

// Decode a packet, returns bytes consumed from buffer
size_t kiss_decode_packet(kiss_packet_t *packet, uint8_t *buffer, size_t buffer_size);

// Create and initialize a packet
kiss_packet_t kiss_new_packet(uint8_t *data_buffer, size_t data_buffer_size);

// Return a human readable name for a command
const char* kiss_command_name(kiss_command_t command);

#ifdef __cplusplus
}
#endif
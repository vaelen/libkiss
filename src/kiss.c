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

#include "kiss.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Encode packet data
size_t kiss_encode_data(uint8_t *decoded, size_t decoded_length, uint8_t *encoded, size_t encoded_length) {
    size_t len = 0;
    for (size_t i = 0; i < decoded_length; i++) {
        if (len >= encoded_length) return len;
        uint8_t b = decoded[i];
        switch (b) {
            case KISS_FRAME_END:
                encoded[len++] = KISS_FRAME_ESCAPE;
                if (len >= encoded_length) return len;
                encoded[len++] = KISS_ESCAPE_FEND;
                break;
            case KISS_FRAME_ESCAPE:
                encoded[len++] = KISS_FRAME_ESCAPE;
                if (len >= encoded_length) return len;
                encoded[len++] = KISS_ESCAPE_FESC;
                break;
            default:
                encoded[len++] = b;
                break;
        }
    }
    return len;
}

// Decode packet data
size_t kiss_decode_data(uint8_t *encoded, size_t encoded_length, uint8_t *decoded, size_t decoded_length) {
    size_t len = 0;
    uint8_t in_escape = 0;
    for (size_t i = 0; i < encoded_length; i++) {
        if (len >= decoded_length) return len;
        uint8_t b = encoded[i];
        switch (b) {
            case KISS_FRAME_ESCAPE:
                in_escape = 1;
                break;
            case KISS_ESCAPE_FEND:
                if (in_escape) {
                    decoded[len++] = KISS_FRAME_END;
                    in_escape = 0;
                } else {
                    decoded[len++] = b;
                }
                break;
            case KISS_ESCAPE_FESC:
                if (in_escape) {
                    decoded[len++] = KISS_FRAME_ESCAPE;
                    in_escape = 0;
                } else {
                    decoded[len++] = b;
                }
                break;
            default:
                decoded[len++] = b;
                in_escape = 0;
                break;
        }
    }
    return len;
}

// Encode the command and port in a single byte
uint8_t kiss_encode_command(kiss_command_t command, uint8_t port) {
    return (port << 4) | command;
}

// Decode the command and port from a single byte
void kiss_decode_command(uint8_t b, kiss_command_t *command, uint8_t *port) {
    *command = (kiss_command_t) (b & 0x0f);
    *port = (b >> 4) & 0x0f;
}

// Encode a packet, returns bytes written to buffer
size_t kiss_encode_packet(kiss_packet_t p, uint8_t *buffer, size_t buffer_length) {
    if (buffer_length < (p.data_length + 3)) return 0;
    buffer[0] = KISS_FRAME_END;
    buffer[1] = kiss_encode_command(p.command, p.port);
    size_t len = kiss_encode_data(p.data, p.data_length, buffer + 2, buffer_length - 2) + 2;
    buffer[len++] = KISS_FRAME_END;
    return len;
}

// Decode a packet, returns bytes consumed from buffer
size_t kiss_decode_packet(kiss_packet_t *p, uint8_t *buffer, size_t buffer_size) {
    // TODO: Update this to better handle partial packets in the stream
    size_t bytes_consumed = 0;
    uint8_t *packet_data_start = 0;
    size_t packet_data_length = 0;
    uint8_t found_packet = 0;
    uint8_t complete_packet = 0;

    for (size_t i = 0; i < buffer_size; i++) {
        uint8_t b = buffer[i];
        if (b == KISS_FRAME_END) {
            if (found_packet) {
                // End of packet
                bytes_consumed = i;
                complete_packet = 1;
                found_packet = 0;
                p->data_length = kiss_decode_data(packet_data_start, packet_data_length, p->data, p->data_capacity);
            }
            bytes_consumed++;
        } else {
            if(complete_packet) {
                // Start of second packet, return early
                break;
            } else if(!found_packet) {
                // Start of first packet
                found_packet = 1;
                packet_data_start = buffer + i + 1;
                packet_data_length = 0;
                kiss_decode_command(b, &(p->command), &(p->port));
            } else {
                packet_data_length++;
            }
        }
    }

    return bytes_consumed;
}

// Create and initialize a packet
kiss_packet_t kiss_new_packet(uint8_t *data_buffer, size_t data_buffer_size) {
    kiss_packet_t p = {
        .port = 0,
        .command = KISS_DATA_FRAME,
        .data = data_buffer,
        .data_length = 0,
        .data_capacity = data_buffer_size
    };
    return p;
}

// Return a human readable name for a command
const char* kiss_command_name(kiss_command_t command) {
    switch (command) {
        case KISS_DATA_FRAME: return "Data Frame"; break;
        case KISS_TX_DELAY: return "TX Delay"; break;
        case KISS_PERSISTENCE: return "Persistence"; break;
        case KISS_SLOT_TIME: return "Slot Time"; break;
        case KISS_TX_TAIL: return "TX Tail"; break;
        case KISS_FULL_DUPLEX: return "Full Duplex"; break;
        case KISS_SET_HARDWARE: return "Set Hardware"; break;
        case KISS_RETURN: return "Exit KISS Mode"; break;
        default: return "Unknown"; break;
    }
}

#ifdef __cplusplus
}
#endif

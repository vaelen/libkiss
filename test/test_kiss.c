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
#include <unity.h>

static uint8_t DECODED_DATA[] = {'T','E','S','T',0xC0,0xDB,0xDB,0xC0};
static size_t DECODED_DATA_LEN = sizeof(DECODED_DATA);
static uint8_t ENCODED_DATA[] = {'T','E','S','T',0xDB,0xDC,0xDB,0xDD,0xDB,0xDD,0xDB,0xDC};
static size_t ENCODED_DATA_LEN = sizeof(ENCODED_DATA);
static kiss_packet_t DECODED_PACKET = {.port = 0, .command = KISS_DATA_FRAME, .data = DECODED_DATA, .data_length = sizeof(DECODED_DATA_LEN)};
static uint8_t ENCODED_PACKET[] = {0xC0,0x00,'T','E','S','T',0xDB,0xDC,0xDB,0xDD,0xDB,0xDD,0xDB,0xDC,0xC0};
static size_t ENCODED_PACKET_LEN = sizeof(ENCODED_PACKET);
static uint8_t ENCODED_PACKET_WITH_PADDING[] = {0xC0,0xC0,0xC0,0x00,'T','E','S','T',0xDB,0xDC,0xDB,0xDD,0xDB,0xDD,0xDB,0xDC,0xC0,0xC0,0xC0};
static size_t ENCODED_PACKET_WITH_PADDING_LEN = sizeof(ENCODED_PACKET_WITH_PADDING);
static uint8_t ENCODED_PACKET_WITH_SECOND_PACKET[] = {0xC0,0xC0,0xC0,0x00,'T','E','S','T',0xDB,0xDC,0xDB,0xDD,0xDB,0xDD,0xDB,0xDC,0xC0,0xC0,0xC0,0x02};
static size_t ENCODED_PACKET_WITH_SECOND_PACKET_LEN = sizeof(ENCODED_PACKET_WITH_SECOND_PACKET);

void test_encode_data() {
  uint8_t buffer[256];
  size_t bytes_written = kiss_encode_data(DECODED_DATA, DECODED_DATA_LEN, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_MESSAGE(ENCODED_DATA_LEN, bytes_written, "Encoded data is the wrong length.");
  TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(ENCODED_DATA, buffer, bytes_written, "Encoded data is wrong.");
}

void test_decode_data() {
  uint8_t buffer[256];
  size_t bytes_written = kiss_decode_data(ENCODED_DATA, ENCODED_DATA_LEN, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_MESSAGE(DECODED_DATA_LEN, bytes_written, "Decoded data is the wrong length.");
  TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(DECODED_DATA, buffer, bytes_written, "Decoded data is wrong.");
}

void test_encode_command() {
  uint8_t b;
  b = kiss_encode_command(0,0);
  TEST_ASSERT_EQUAL_CHAR_MESSAGE(0x00, b, "Command 0, Port 0");
  b = kiss_encode_command(0,5);
  TEST_ASSERT_EQUAL_CHAR_MESSAGE(0x50, b, "Command 0, Port 5");
  b = kiss_encode_command(3,0);
  TEST_ASSERT_EQUAL_CHAR_MESSAGE(0x03, b, "Command 3, Port 0");
  b = kiss_encode_command(1,2);
  TEST_ASSERT_EQUAL_CHAR_MESSAGE(0x21, b, "Command 1, Port 2");
}

void test_decode_command() {
  kiss_command_t command = KISS_DATA_FRAME;
  uint8_t port = 0;
  kiss_decode_command(0x00, &command, &port);
  TEST_ASSERT_EQUAL_CHAR_MESSAGE(command, KISS_DATA_FRAME, "Command incorrect on the first example.");
  TEST_ASSERT_EQUAL_CHAR_MESSAGE(port, 0, "Port incorrect on the first example.");
  kiss_decode_command(0x50, &command, &port);
  TEST_ASSERT_EQUAL_CHAR_MESSAGE(command, KISS_DATA_FRAME, "Command incorrect on the second example.");
  TEST_ASSERT_EQUAL_CHAR_MESSAGE(port, 5, "Port incorrect on the second example.");
  kiss_decode_command(0x05, &command, &port);
  TEST_ASSERT_EQUAL_CHAR_MESSAGE(command, KISS_FULL_DUPLEX, "Command incorrect on the third example.");
  TEST_ASSERT_EQUAL_CHAR_MESSAGE(port, 0, "Port incorrect on the hird example.");
  kiss_decode_command(0x55, &command, &port);
  TEST_ASSERT_EQUAL_CHAR_MESSAGE(command, KISS_FULL_DUPLEX, "Command incorrect on the forth example.");
  TEST_ASSERT_EQUAL_CHAR_MESSAGE(port, 5, "Port incorrect on the forth example.");
}

void test_encode_packet() {
  uint8_t buffer[256];
  size_t bytes_written = kiss_encode_packet(DECODED_PACKET, buffer, sizeof(buffer));
  TEST_ASSERT_EQUAL_MESSAGE(ENCODED_PACKET_LEN, bytes_written, "Encoded packet is the wrong length.");
  TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(ENCODED_PACKET, buffer, bytes_written, "Encoded packet is wrong.");
}

void test_decode_packet() {
  uint8_t buffer[256];
  kiss_packet_t p = kiss_new_packet(buffer, sizeof(buffer));
  size_t bytes_consumed = kiss_decode_packet(&p, ENCODED_PACKET, ENCODED_PACKET_LEN);
  TEST_ASSERT_EQUAL_MESSAGE(ENCODED_PACKET_LEN, bytes_consumed, "Wrong number of bytes consumed.");
  TEST_ASSERT_EQUAL_MESSAGE(DECODED_PACKET.port, p.port, "Port is wrong.");
  TEST_ASSERT_EQUAL_MESSAGE(DECODED_PACKET.command, p.command, "Command is wrong.");
  TEST_ASSERT_EQUAL_MESSAGE(DECODED_PACKET.data_length, p.data_length, "Data length is wrong.");
  TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(DECODED_DATA, p.data, p.data_length, "Data is wrong.");
}

void test_decode_padded_packet() {
  uint8_t buffer[256];
  kiss_packet_t p = kiss_new_packet(buffer, sizeof(buffer));
  size_t bytes_consumed = kiss_decode_packet(&p, ENCODED_PACKET_WITH_PADDING, ENCODED_PACKET_WITH_PADDING_LEN);
  TEST_ASSERT_EQUAL_MESSAGE(ENCODED_PACKET_WITH_PADDING_LEN, bytes_consumed, "Wrong number of bytes consumed.");
  TEST_ASSERT_EQUAL_MESSAGE(DECODED_PACKET.port, p.port, "Port is wrong.");
  TEST_ASSERT_EQUAL_MESSAGE(DECODED_PACKET.command, p.command, "Command is wrong.");
  TEST_ASSERT_EQUAL_MESSAGE(DECODED_PACKET.data_length, p.data_length, "Data length is wrong.");
  TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(DECODED_DATA, p.data, p.data_length, "Data is wrong.");
}


void test_decode_with_second_packet() {
  uint8_t buffer[256];
  kiss_packet_t p = kiss_new_packet(buffer, sizeof(buffer));
  size_t bytes_consumed = kiss_decode_packet(&p, ENCODED_PACKET_WITH_SECOND_PACKET, ENCODED_PACKET_WITH_SECOND_PACKET_LEN);
  TEST_ASSERT_EQUAL_MESSAGE(ENCODED_PACKET_WITH_PADDING_LEN, bytes_consumed, "Wrong number of bytes consumed.");
  TEST_ASSERT_EQUAL_MESSAGE(DECODED_PACKET.port, p.port, "Port is wrong.");
  TEST_ASSERT_EQUAL_MESSAGE(DECODED_PACKET.command, p.command, "Command is wrong.");
  TEST_ASSERT_EQUAL_MESSAGE(DECODED_PACKET.data_length, p.data_length, "Data length is wrong.");
  TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(DECODED_DATA, p.data, p.data_length, "Data is wrong.");
}

int runTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_encode_data);
    RUN_TEST(test_decode_data);
    RUN_TEST(test_encode_command);
    RUN_TEST(test_encode_packet);
    RUN_TEST(test_decode_packet);
    RUN_TEST(test_decode_padded_packet);
    RUN_TEST(test_decode_with_second_packet);
    return UNITY_END();
}

void start() {
  runTests();
}

void loop() {}

int main() {
  return runTests();
}
#include <Arduino.h>

constexpr size_t honda_packet_length{12};
constexpr int honda_read_timeout{500};
constexpr uint8_t start_char{0x87};
constexpr uint32_t honda_serial_baud{9600};
constexpr auto honda_serial_config{SERIAL_8E1};

static auto &honda_bms_serial{Serial2};
static void forward_bms_messages();
static auto &honda_vcm_serial{Serial3};

static auto &console_serial{SerialUSB};

void setup()
{
  honda_bms_serial.begin(honda_serial_baud, honda_serial_config);
  honda_vcm_serial.begin(honda_serial_baud, honda_serial_config);
  console_serial.begin(115200);
}

void loop()
{
  forward_bms_messages();
}

static void print_honda_serial_message(const uint8_t *buffer)
{
  console_serial.printf("BMS: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
    buffer[0], buffer[1], buffer[2], buffer[3],
    buffer[4], buffer[5], buffer[6], buffer[7],
    buffer[8], buffer[9], buffer[10], buffer[11]);
}

static bool receive_packet(uint8_t *buffer, Stream &stream)
{
  bool found_start = false;
  while (stream.available())
  {
    if (stream.read() == start_char)
    {
      found_start = true;
      break;
    }
  }

  if (!found_start)
  {
    return false;
  }

  buffer[0] = start_char;
  auto bytes_read{stream.readBytes(buffer + 1, honda_packet_length - 1)};
  return bytes_read == honda_packet_length - 1;
}

static void forward_bms_messages()
{
  uint8_t buffer[honda_packet_length];
  while (honda_bms_serial.available())
  {
    if (receive_packet(buffer, honda_bms_serial))
    {
      honda_vcm_serial.write(buffer, honda_packet_length);
      print_honda_serial_message(buffer);
    }
  }
}

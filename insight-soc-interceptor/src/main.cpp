#include <Arduino.h>

constexpr size_t honda_packet_length{12};
constexpr int honda_read_timeout{50};
constexpr uint8_t start_char{0x87};

static auto& honda_bms_serial{Serial3};
void honda_bms_serial_event();
// void serialEvent1() { honda_bms_serial_event(); }

void honda_vcm_serial_event();
static auto& honda_vcm_serial{Serial2};
// void serialEvent2() { honda_vcm_serial_event(); }

constexpr uint32_t honda_serial_baud{9600};

static auto& console_serial{SerialUSB};

void setup() {
  honda_bms_serial.begin(honda_serial_baud, SERIAL_8E1);
  honda_vcm_serial.begin(honda_serial_baud, SERIAL_8E1);
  console_serial.begin(115200);

  pinMode(13, OUTPUT);
}

void loop() {
  // console_serial.printf("loop\n");
  honda_bms_serial_event();
  honda_vcm_serial_event();
}

void print_serial_message(const char* src, const uint8_t *buffer, size_t len)
{
  console_serial.printf("%s: ", src);

  for (auto i=0U; i < len; ++i) {
    console_serial.printf("%02X ", buffer[i]);
  }

  console_serial.printf("\n");
  console_serial.flush();
}

bool receive_packet(uint8_t *buffer, Stream& stream)
{
  bool found_start = false;
  while(stream.available())
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
  auto bytes_read{stream.readBytes(buffer+1, honda_packet_length - 1)};
  return bytes_read == honda_packet_length - 1;
}

void honda_bms_serial_event()
{
  uint8_t buffer[honda_packet_length];
  if (honda_bms_serial.available())
  {
    if (receive_packet(buffer, honda_bms_serial))
    {
      honda_vcm_serial.write(buffer, honda_packet_length);
      print_serial_message("BMS", buffer, honda_packet_length);
    }
  }
}

void honda_vcm_serial_event()
{
  uint8_t buffer[honda_packet_length];
  if (honda_vcm_serial.available())
  {
    if (receive_packet(buffer, honda_vcm_serial))
    {
      honda_bms_serial.write(buffer, honda_packet_length);
      print_serial_message("VCM", buffer, honda_packet_length);
    }
  }
}

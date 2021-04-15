#include <Arduino.h>

constexpr size_t honda_packet_length{12};
constexpr int honda_read_timeout{10};
constexpr uint8_t start_char1{0x87};
constexpr uint8_t start_char2{0xAA};
constexpr uint32_t honda_serial_baud{9600};
constexpr auto honda_serial_config{SERIAL_8E1};

static auto &honda_bms_serial{Serial2};
static bool forward_bms_messages(uint8_t start_char);
static auto &honda_vcm_serial{Serial3};

static auto &console_serial{SerialUSB};

void setup()
{
  honda_bms_serial.begin(honda_serial_baud, honda_serial_config);
  honda_bms_serial.setTimeout(honda_read_timeout);
  honda_vcm_serial.begin(honda_serial_baud, honda_serial_config);
  console_serial.begin(115200);
}

void loop()
{
  forward_bms_messages(start_char1);
  forward_bms_messages(start_char2);
}

static void print_honda_serial_message(const uint8_t *buffer, bool ioio)
{
  if(ioio)
  {
    console_serial.printf(" In: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
      buffer[0], buffer[1], buffer[2], buffer[3],
      buffer[4], buffer[5], buffer[6], buffer[7],
      buffer[8], buffer[9], buffer[10], buffer[11]);
  }
  else
  {
    console_serial.printf("Out: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
      buffer[0], buffer[1], buffer[2], buffer[3],
      buffer[4], buffer[5], buffer[6], buffer[7],
      buffer[8], buffer[9], buffer[10], buffer[11]);
  }

}

static void adjust_honda_packet(uint8_t *buffer)
{
  uint8_t check_sum = 0;

  if(buffer[0] == 0x87)
  {
    buffer[3]=0x15;
    buffer[4]=0x6F;
    buffer[8]=0x39;
    buffer[9]=0x39;
  }
  if(buffer[0] == 0xAA)
  {

  }

  for(uint8_t i=0; i<11; i++)
  {
    check_sum = check_sum + buffer[i];
  }
  check_sum = ~check_sum;
  check_sum = check_sum + 1;
  buffer[11]=check_sum & 0x7F ;
}

static bool receive_packet(uint8_t *buffer, Stream &stream, uint8_t start_char)
{
  while (stream.read() != start_char) ;

  buffer[0] = start_char;
  auto bytes_read{stream.readBytes(buffer + 1, honda_packet_length - 1)};
  return bytes_read == honda_packet_length - 1;
}

static bool forward_bms_messages(uint8_t start_char)
{
  uint8_t buffer[honda_packet_length];
  if (receive_packet(buffer, honda_bms_serial, start_char))
  {
    print_honda_serial_message(buffer,1);
    adjust_honda_packet(buffer);
    honda_vcm_serial.write(buffer, honda_packet_length);
    print_honda_serial_message(buffer,0);
    return 1;
  }

  return 0;
}

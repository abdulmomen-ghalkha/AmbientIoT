#include <bluefruit.h>

// BLE UART Service
BLEUart bleuart;

char rx_buffer[64];
uint8_t rx_index = 0;
bool receiving = false;


void setup()
{
    // USB Serial (PC <-> XIAO)
    Serial.begin(115200);
    Serial1.begin(115200);

    // Wait a moment for Serial Monitor
    delay(1000);

    // Allow a large ATT MTU (up to 247) so one whole frame fits in a single
    // notification. Must be called before Bluefruit.begin().
    Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

    // Initialize BLE
    Bluefruit.begin();
    Bluefruit.setTxPower(4);
    Bluefruit.setName("IoT_Node");

    // Start BLE UART service
    bleuart.begin();

    // Configure advertising
    Bluefruit.Advertising.addFlags(
        BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);

    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addService(bleuart);

    Bluefruit.ScanResponse.addName();

    // Advertise forever
    Bluefruit.Advertising.start(0);

    Serial.println("=================================");
    Serial.println("XIAO BLE UART Bridge Ready");
    Serial.println("Device Name: IoT_Node");
    Serial.println("Connect using nRF Connect");
    Serial.println("Type in Serial Monitor to send");
    Serial.println("=================================");
}

void loop()
{
  // USB Serial --> BLE
  while (Serial1.available())
  {
    char c = (char) Serial1.read();

    if (c == 0x02)
    {
      rx_index = 0;
      receiving = true;
    }

    else if (c == 0x03)
    {
      if (receiving)
      {
        // Strip the STM32's trailing "\r\n" so we control the line ending
        while (rx_index > 0 &&
               (rx_buffer[rx_index - 1] == '\r' || rx_buffer[rx_index - 1] == '\n'))
        {
          rx_index--;
        }
        rx_buffer[rx_index] = '\0';

        Serial.print("Sending: ");
        Serial.println(rx_buffer);

        // Send the frame plus one '\n' as a SINGLE notification. A separate
        // write("\n") would arrive as its own notification and overwrite the
        // value shown in nRF Connect, which then looks empty.
        rx_buffer[rx_index] = '\n';
        bleuart.write((uint8_t*)rx_buffer, rx_index + 1);

        rx_index = 0;
        receiving = false;

      }
    }
    else if (receiving)
    {
      if (rx_index < sizeof(rx_buffer) - 1)
      {
        rx_buffer[rx_index++] = c;
      }
      else
      {
        rx_index = 0;
        receiving = false;
        Serial.println("Packet overflow");
      }
    }

  }





}
#include <bluefruit.h>

// BLE Service & Characteristic
BLEUart bleuart;

// Static buffer for non-blocking reception
char rx_buffer[32]; 
int rx_index = 0;
bool receiving = false;

void setup() {

  // Serial1 = UART to STM32, Serial = USB to PC Debug
  Serial1.begin(115200); 
//  Serial.begin(115200);
  
  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  Bluefruit.setName("IoT_Node");
  
  bleuart.begin();

  // Advertising setup
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.Advertising.start(0); 
  
  Serial.println("Xiao BLE Ready. Waiting for STM32 data...");
}

void loop() {
  // Only process if BLE is connected
  if (Bluefruit.connected()) {
    
    while (Serial1.available()) {
      char c = (char)Serial1.read();
//      Serial.print("Received char: ");
//      Serial.println(c);

      // Start of Packet (0x02)
      if (c == 0x02) { 
        rx_index = 0;
        receiving = true;
      } 
      // End of Packet (0x03)
      else if (c == 0x03) { 
        if (receiving) {
          rx_buffer[rx_index] = '\0'; // Null-terminate string
          
//          // Debugging
//          Serial.print("Sending: ");
//          Serial.println(rx_buffer);
          
          // Transmit over BLE
          bleuart.write((uint8_t*)rx_buffer, rx_index);
          bleuart.write('\n');
          
          receiving = false;
        }
      } 
      // Payload accumulation
      else if (receiving) {
        if (rx_index < (sizeof(rx_buffer) - 1)) {
          rx_buffer[rx_index++] = c;
        }
      }
    }
  }
}
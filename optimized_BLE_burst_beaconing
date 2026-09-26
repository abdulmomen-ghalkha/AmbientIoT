#include <bluefruit.h>

#define RX_PIN PIN_SERIAL1_RX 

char rx_buffer[32]; 
int rx_index = 0;
bool receiving = false;

// We will store the packed sensor data here and broadcast it as Manufacturer Data
// Byte 0-1: Company ID (0xFFFF for testing/custom use)
// Byte 2-3: Temperature (Value * 100)
// Byte 4-5: Humidity (Value * 100)
// Byte 6-7: Raw Soil Moisture ADC
uint8_t mfg_data[8] = {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void wake_isr() {
  resumeLoop();
}

void setup() {
  // 1. Dark Mode
  pinMode(LED_RED, OUTPUT); digitalWrite(LED_RED, HIGH);
  pinMode(LED_GREEN, OUTPUT); digitalWrite(LED_GREEN, HIGH);
  pinMode(LED_BLUE, OUTPUT); digitalWrite(LED_BLUE, HIGH);
  
  // KILL THE BLUE ADAFRUIT STATUS LED
  Bluefruit.autoConnLed(false); 
  
  // 2. Enable Hardware DC-DC for the absolute lowest radio current
  sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);

  Bluefruit.begin();
  Bluefruit.setTxPower(-4); 
  Bluefruit.setName("EcoNode");

  // 3. Set up as a Broadcast Beacon
  Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED);
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, mfg_data, sizeof(mfg_data));
  
  // Set interval to 100ms (160 * 0.625) so the gateway catches it quickly
  Bluefruit.Advertising.setInterval(160, 160); 
  
  // Broadcast for exactly 2 seconds on boot, then automatically turn off the radio!
  Bluefruit.Advertising.start(2); 
}

void loop() {
  pinMode(RX_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RX_PIN), wake_isr, FALLING);

  // ==========================================
  // CPU SLEEPS HERE IN MICROAMPS
  // ==========================================
  suspendLoop();

  // ==========================================
  // AWAKE: Fetch the new string from the STM32
  // ==========================================
  detachInterrupt(digitalPinToInterrupt(RX_PIN)); 
  Serial1.begin(115200);
  
  uint32_t start_time = millis();
  bool packet_complete = false;

  while (millis() - start_time < 50) { 
    if (Serial1.available()) {
      char c = (char)Serial1.read();
      if (c == 0x02) { 
        rx_index = 0;
        receiving = true;
      } 
      else if (c == 0x03) { 
        if (receiving) {
          rx_buffer[rx_index] = '\0';
          packet_complete = true;
          break; 
        }
      } 
      else if (receiving && rx_index < 31) {
        rx_buffer[rx_index++] = c;
      }
    }
  }

  receiving = false;
  Serial1.end(); 

  // ==========================================
  // UPDATE THE ADVERTISING BILLBOARD
  // ==========================================
  if (packet_complete) {
    // 1. Parse the incoming string ("23.5,45.2,1234")
    char* token = strtok(rx_buffer, ",");
    if (token != NULL) {
      float t = atof(token);
      token = strtok(NULL, ",");
      float h = (token != NULL) ? atof(token) : 0.0;
      token = strtok(NULL, ",");
      int s = (token != NULL) ? atoi(token) : 0;

      int16_t t_int = (int16_t)(t * 100);
      uint16_t h_int = (uint16_t)(h * 100);
      uint16_t s_int = (uint16_t)s;

      mfg_data[2] = t_int & 0xFF;
      mfg_data[3] = (t_int >> 8) & 0xFF;
      mfg_data[4] = h_int & 0xFF;
      mfg_data[5] = (h_int >> 8) & 0xFF;
      mfg_data[6] = s_int & 0xFF;
      mfg_data[7] = (s_int >> 8) & 0xFF;

      // 3. Stop, Update the payload, and Fire the 2-second Burst manually!
      Bluefruit.Advertising.stop();
      Bluefruit.Advertising.clearData();
      Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
      Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, mfg_data, sizeof(mfg_data));
      
      // Start broadcasting indefinitely (0)
      Bluefruit.Advertising.start(0);
      
      // Sleep the CPU for exactly 2 seconds while the radio does its job.
      // (The CPU will stay in the microamp range between the 100ms radio spikes)
      delay(2000); 
      
      // MANUALLY KILL THE RADIO! 
      Bluefruit.Advertising.stop();
      
      // Now the loop ends, restarts, and hits suspendLoop() with the radio safely dead.
    }
  }
}
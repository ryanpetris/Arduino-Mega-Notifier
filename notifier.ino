#include <SPI.h>
#include <Ethernet.h>

byte mac_address[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
const int pin_min = 22;
const int pin_max = 49;
const int pin_count = pin_max - pin_min + 1;
int pin_states[pin_count];
int silent_loops = 0;
const int max_silent_loops = 100;

void setup() {
  Serial.begin(115200);

  for (int i = pin_min; i <= pin_max; i++) {
    pinMode(i, INPUT_PULLUP);
  }

  setup_ethernet();
}

void setup_ethernet() {
  Serial.println("Configuring Ethernet...");
  
  while(Ethernet.begin(mac_address) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP. Trying again...");
    delay(1000);
  }

  Serial.print("Ethernet configured with IP address ");
  Serial.print(Ethernet.localIP());
  Serial.print(" and subnet mask ");
  Serial.print(Ethernet.subnetMask());
  Serial.println(".");
}

void setPinState(int pin, int state) {
  int pin_index = pin - pin_min;
  
  pin_states[pin_index] = state;
}

int getPinState(int pin) {
  int pin_index = pin - pin_min;
  
  return pin_states[pin_index];
}

void send_pin_states() {
  silent_loops = 0;
  
  char data[100];
  
  Serial.println("{");
  
  sprintf(data, "    \"device\": \"%02x:%02x:%02x:%02x:%02x:%02x\",", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
  Serial.println(data);

  Serial.println("    \"pins\": {");
  
  for (int i = pin_min; i <= pin_max; i++) {
    int state = getPinState(i);

    Serial.print("        \"");
    Serial.print(i, DEC);
    Serial.print("\": ");

    if (state > 0) {
      Serial.print("false");
    } else {
      Serial.print("true");
    }

    if (i < pin_max) {
      Serial.print(",");
    }

    Serial.println("");
  }

  Serial.println("    }");
  Serial.println("}");
}

void loop() {
  if (Ethernet.linkStatus() != LinkON) {
    setup_ethernet();
  }

  bool send_update = false;

  for (int i = pin_min; i <= pin_max; i++) {
    int state = digitalRead(i);
    int prev_state = getPinState(i);

    if (state != prev_state) {
      send_update = true;
    }

    setPinState(i, state);
  }

  if (silent_loops > max_silent_loops) {
    send_update = true;
  }

  silent_loops++;

  if (send_update) {
    send_pin_states();
  }
  
  delay(50);
}

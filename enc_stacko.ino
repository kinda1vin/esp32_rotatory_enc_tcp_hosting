#include <WiFi.h>
#include <WiFiClient.h>

// Wi-Fi credentials
const char* ssid = "****";
const char* password = "*********";

// TCP server settings
const uint16_t serverPort = 8080;
WiFiServer server(serverPort);

#define ENCODER_PIN_A 3  //black
#define ENCODER_PIN_B 8  //white
#define ENCODER_PIN_Z 18  // orange

volatile int encoder_value = 0;      
int lastEncoderValue = 0;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 5;
const float degrees_per_pulse = 360.0 / 1800;  // Conversion factor for 1800 P/R

void IRAM_ATTR encoder_isr() {
    static uint8_t old_AB = 3;  // Remember previous state (0b11)
    static int8_t encval = 0;   // Accumulator for encoder steps
    static const int8_t enc_states[] = {
        0, -1, 1, 0,  // 0b00 -> 0, 0b01 -> -1, 0b10 -> 1, 0b11 -> 0
        1, 0, 0, -1,
       -1, 0, 0, 1,
        0, 1, -1, 0 
    };  // State transition table

    old_AB <<= 2;  // Shift the old state to make room for the new one
    old_AB |= (digitalRead(ENCODER_PIN_A) << 1) | digitalRead(ENCODER_PIN_B);  // Add the current state of A and B

    // Add or subtract encoder value based on state transition
    encval += enc_states[(old_AB & 0x0f)];

    // Update the encoder position when a full step (4 counts) is detected
    if (encval > 3) {  // Four steps forward
        encoder_value++;
        encval = 0;
    } else if (encval < -3) {  // Four steps backward
        encoder_value--;
        encval = 0;
    }

    lastDebounceTime = millis();
}

void setup() {
    Serial.begin(115200);
    pinMode(ENCODER_PIN_A, INPUT_PULLUP);
    pinMode(ENCODER_PIN_B, INPUT_PULLUP);
    //pinMode(ENCODER_PIN_Z, INPUT_PULLUP);

    // Attach interrupts to both A and B channels
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), encoder_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), encoder_isr, CHANGE);

    // Connect to Wi-Fi network
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Start the TCP server
    server.begin();
    Serial.println("TCP server started on port " + String(serverPort));
}

void loop() {
    // Check if a client has connected
    WiFiClient client = server.available();
    if (client) {
        Serial.println("New client connected");
        while (client.connected()) {
            // Only send data if the encoder value has changed
            if (encoder_value != lastEncoderValue) {
                // Calculate the degrees turned
                float total_degrees = encoder_value * degrees_per_pulse;
                String dataOnSerial = "Encoder Value: " + String(encoder_value) + ", Degrees Turned: " + String(total_degrees, 2) + "\n";
                String dataToSend = String(encoder_value) + " " + String(total_degrees, 2) + "\n";
                client.print(dataToSend);
                client.flush();

                // Update last known encoder value
                lastEncoderValue = encoder_value;

                Serial.println(dataOnSerial);
            }
            delay(10);
        }

        // Client disconnected
        client.stop();
        Serial.println("Client disconnected");
    }
}

#include <Homie.h>

#define HW_UART_SPEED 115200L
#define FRONT_DOOR 34
#define LED_NOTIFY 32
#define LED_STATUS 27

int frontVal = 1;

bool FD_Reading;
int FD_Counter = 0;
int FD_Max = 0;

bool broadcastHandler(const String& level, const String& value) {
  Serial << "Received broadcast level " << level << ": " << value << endl;
  return true;
}

void loopHandler() {
  FD_Reading = digitalRead(FRONT_DOOR);
    
  //If statements to increment the counter and determine if a new max value has been set
  if (FD_Reading) {
    FD_Counter += 1;
  }
  else {
    FD_Counter = 0;
  }
  if (FD_Counter > FD_Max) {
    FD_Max = FD_Counter;
  }
  /*
  Running on Xenon with SYSTEM_THREAD(ENABLED) it normally peaks at ~173 from AC voltage, seen it peak to ~300
  Very fast doorbell push as low as ~433
  Disable these if statements if you need to calibrate your setup 
  */
  if (FD_Counter > 400) {
    Serial.println("Ding dong");
    digitalWrite(LED_NOTIFY, HIGH);
    Homie.getMqttClient().publish("homie/$broadcast/doorbell", 1, false, "Ding dong");
    delay(3000); //A delay to allow for the doorbell to stop being pushed, to avoid multiple publishes for a single doorbell push
    digitalWrite(LED_NOTIFY, LOW);
    FD_Counter = 0;
    FD_Max = 0;
  }
}

void setup() {
  pinMode(FRONT_DOOR, INPUT);
  pinMode(LED_NOTIFY, OUTPUT);
  pinMode(LED_STATUS, OUTPUT);
  Serial.begin(HW_UART_SPEED);
  Serial << endl << endl;

  Homie_setFirmware("doorbell", "1.0.0"); // The underscore is not a typo! See Magic bytes
  Homie.setLoopFunction(loopHandler);
  Homie.setBroadcastHandler(broadcastHandler);
  Homie.setLedPin(LED_STATUS, HIGH);
  Homie.setup();
}

void loop() {
  Homie.loop();
}
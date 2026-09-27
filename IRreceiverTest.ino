/*
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRsend.h>



const uint16_t kRecvPin = 15;
const uint16_t kBlasterPin = 5;

IRsend irsend(kBlasterPin);
IRrecv irrecv(kRecvPin);
decode_results results;

void setup() {
  Serial.begin(115200);
  irrecv.enableIRIn();
  irsend.begin();
  Serial.println("IR Ready 🔥");
}
/*
void repeat(){
  for(int i = 0; i < 3; i++){
    irsend.sendRaw((uint16_t*)results.rawbuf, results.rawlen, 38);
    delay(100);
  }
}*/


/*void loop() {
  if (irrecv.decode(&results)) {

    uint64_t lastCode = results.value;
    

    Serial.print("Received: ");
    Serial.println(typeToString(results.decode_type));
    Serial.println(lastCode, HEX);

    // safer test send
    //irsend.sendNEC(lastCode, 32);
    irsend.sendRaw((uint16_t*)results.rawbuf, results.rawlen, 36);
    Serial.println("IR Transmitted");

    irrecv.resume();
  }

  delay(100);
}
*/
/*
void loop() {
  if (irrecv.decode(&results)) {

    Serial.print("Type: ");
    Serial.println(typeToString(results.decode_type));

    Serial.print("Code: 0x");
    Serial.println(results.value, HEX);

    // Check if the library successfully decoded it as RC5
    if (results.decode_type == decode_type_t::RC5) {
      
      // Temporarily disable the receiver so it doesn't blind itself
      irrecv.disableIRIn();

      // Send the decoded value. 
      // RC5 standard is usually 12 bits. 
      irsend.sendRC5(results.value, 12);
      
      Serial.println("IR Transmitted via RC5 Protocol");

      // Turn receiver back on
      irrecv.enableIRIn();
    } else {
      Serial.println("Decoded protocol is not RC5. Skipping transmission.");
    }

    irrecv.resume();
  }

  delay(100);
}
//RC5 = 12 
//NEC = 32
*/

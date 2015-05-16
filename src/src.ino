#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <IridiumSBD.h>

#define GPS_WAIT 7U * 60UL * 1000UL //The max time to wait for an GPS signal
#define IRIDIUM_BAUD 19200
#define GPS_BAUD 4800
#define SIZEOFSTR 50

SoftwareSerial ssIridium(0, 1); //RX, TX
SoftwareSerial ssGPS(A5, A4); //RX, TX
IridiumSBD isbd(ssIridium, 10); //The sleep line for the RockBlock
TinyGPSPlus gps;

char str[SIZEOFSTR];

void setup() {
  isbd.setPowerProfile(1); //Option for low current devices
}

void loop() {
  //Wake up the RockBlock and listen to it
  ssIridium.begin(IRIDIUM_BAUD);
  delay(1000); //wait a bit
  ssIridium.listen();
  delay(1000); //wait a bit
  
  if( isbd.begin() == ISBD_SUCCESS ) {
    //Get the values and send the message
    //Other sensor function need to be added
    if( get_gps() ) {
      sprintf(str,"%f,%f,", gps.location.lat(), gps.location.lng());
    }
    else
      sprintf(str, "invalid, invalid,");
  }
  isbd.sendSBDBinary((uint8_t *) str, SIZEOFSTR);
  //Fall in sleep again
  isbd.sleep();
  delay(1000); //wait one sec
  ssIridium.end();
  //Sleep, use avr/sleep and the Timer/Counter Hardware to wait, so put the Arduino in idle Mode
  //Right now just a delay as placeholder
  delay(7UL * 60UL * 1000UL); //Delay 7 min, just a placeholder!
}

//Updates the gps and returns non zero on success
int get_gps() {
  //Millis will overflow (go back to zero) after about 50 days, 
  //but as we are using unsigned longs it should still work 
  //Wake up the GPS and then wait for a valid signal
  ssGPS.begin(GPS_BAUD);
  delay(1000);
  unsigned long now = 0;
  for(now = millis(); millis() - now < GPS_WAIT;) {
    while(ssGPS.available() > 0) {
      if(gps.encode(ssGPS.read())) {
        if(gps.location.isValid() && gps.location.isUpdated()) {
          ssGPS.end();
          return 1; //Succes, got valid updated GPS signal
        }//End inner if
      }//End outer if
    }//End while
  }//End for
  return 0; 
}

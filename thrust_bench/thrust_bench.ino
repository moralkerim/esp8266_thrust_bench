#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define control_pin 5
#define pwm_freq    250
#define ssid        "Kerim"
#define pass        "wikiboy123"
#include "HX711.h"

#define calibration_factor 32840.0 //This value is obtained using the SparkFun_HX711_Calibration sketch

#define LOADCELL_DOUT_PIN  14
#define LOADCELL_SCK_PIN  12

#define MAX 100 

HX711 scale;


unsigned int localPort = 8888;      // local port to listen on
char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1];
IPAddress dest_ip(0, 0, 0, 0);
int dest_port = 9000;
int thrust;
unsigned long now;
float weight;
char buf[MAX];

WiFiUDP Udp;

int prc2pwm(int percent) {
  int pwm = map(percent, 0, 100, 250, 500);
  return pwm;
}

void connect_wifi(char *Wifi_name, char *Pass) {
  WiFi.begin(Wifi_name, Pass);
  int Wifi_stat = WiFi.waitForConnectResult();

  if (Wifi_stat != WL_CONNECTED) {
    WiFi.disconnect(true);

    if (Wifi_stat == WL_NO_SSID_AVAIL) {
      Serial.println("Wifi fail: Wrong SSID.");
    }
    else if (Wifi_stat == WL_CONNECT_FAILED ) {
      Serial.println("Wifi fail: Wrong pass.");
    }

  }

  else {
    Serial.println("Wifi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.printf("UDP server on port %d\n", localPort);
    Udp.begin(localPort);
  }
}

void checkUdp() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    /*Serial.printf("Received packet of size %d from %s:%d\n    (to %s:%d, free heap = %d B)\n",
                  packetSize,
                  Udp.remoteIP().toString().c_str(), Udp.remotePort(),
                  Udp.destinationIP().toString().c_str(), Udp.localPort(),
                  ESP.getFreeHeap()); */

    // read the packet into packetBufffer
    int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    packetBuffer[n] = 0;
    thrust = atoi(packetBuffer);
    Serial.print("Thrust: ");
    Serial.println(thrust);

  }
}

void sendUdp() {
  if (millis() - now > 1000) {
    gcvt(weight, 4, buf); 
    //Serial.println(buf);
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(buf);
    Udp.endPacket();
    now = millis();
  }

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  connect_wifi(ssid, pass);
  pinMode(control_pin, OUTPUT);
  analogWriteFreq(pwm_freq);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0


}

void loop() {  
//  Serial.print("Reading: ");
  weight = scale.get_units();
//  Serial.print(weight); //scale.get_units() returns a float
//  Serial.print(" kg"); //You can change this to kg but you'll need to refactor the calibration_factor
//  Serial.println();
  analogWrite(control_pin, prc2pwm(thrust));
  checkUdp();
  sendUdp();
}

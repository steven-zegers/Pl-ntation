#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <dht.h>

const int delayTime = 2000;
const int pumpTime = 1500;
const int buzzTime = 1000;

/*
+==============+===========+=========+=========+==========+
|  component   | phys. pin | pin nr. | ON msg. | OFF msg. |
+==============+===========+=========+=========+==========+
| light sensor | A0        | A0      | /       | /        |
+--------------+-----------+---------+---------+----------+
| DHT11        | D4        | 2       | /       | /        |
+--------------+-----------+---------+---------+----------+
| green LED    | D6        | 12      | 0       | 1        |
+--------------+-----------+---------+---------+----------+
| Relais       | D7        | 13      | 2       | 3        |
+--------------+-----------+---------+---------+----------+
| red LED      | D5        | 14      | 4       | 5        |
+--------------+-----------+---------+---------+----------+
| Buzzer       | D8        | 15      | 6       | 7        |
+--------------+-----------+---------+---------+----------+
*/

const int lightPin = A0;
const int DHT11Pin = 2;
const int greenLedPin = 12;
const int relaisPin = 13;
const int redLedPin = 14;
const int buzzerPin = 15;

dht DHT;
int lightValue;
float tempValue;
int humValue;

// Watson IoT connection details
#define MQTT_HOST "408twf.messaging.internetofthings.ibmcloud.com"
#define MQTT_PORT 1883
#define MQTT_DEVICEID "d:408twf:ESP8266:plantSensors"
#define MQTT_USER "use-token-auth"
#define MQTT_TOKEN "secrettoken"
#define MQTT_TOPIC "iot-2/evt/status/fmt/json"
#define MQTT_TOPIC_DISPLAY "iot-2/cmd/update/fmt/json"

// Update these with values suitable for your network.
const char* ssid = "ucll-projectweek-IoT";
const char* password = "Foo4aiHa";
const char* mqtt_server = "408twf.messaging.internetofthings.ibmcloud.com";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  Serial.begin(9600);
  pinMode (lightPin, INPUT);
  pinMode (DHT11Pin, INPUT);
  pinMode (greenLedPin, OUTPUT);
  pinMode (relaisPin, OUTPUT);
  pinMode (redLedPin, OUTPUT);
  pinMode (buzzerPin, OUTPUT);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[12] == '0') {
    digitalWrite(greenLedPin, HIGH);
  } else if ((char)payload[12] == '1') {
    digitalWrite(greenLedPin, LOW);
  } else if ((char)payload[12] == '2') {
    digitalWrite(relaisPin, HIGH);
    delay(pumpTime);
    digitalWrite(relaisPin, LOW);
  } else if ((char)payload[12] == '3') {
    digitalWrite(relaisPin, LOW);
  } else if ((char)payload[12] == '4') {
    digitalWrite(redLedPin, HIGH);
  } else if ((char)payload[12] == '5') {
    digitalWrite(redLedPin, LOW);
  } else if ((char)payload[12] == '6') {
    digitalWrite(buzzerPin, HIGH);
    delay(buzzTime);
    digitalWrite(buzzerPin, LOW);
  } else if ((char)payload[12] == '7') {
    digitalWrite(buzzerPin, LOW);
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
      Serial.println("connected");
      client.subscribe(MQTT_TOPIC_DISPLAY);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  lightValue = 100 - (analogRead(lightPin) * 100 / 1024);
  int chk = DHT.read11(DHT11Pin);
  tempValue = DHT.temperature;
  humValue = DHT.humidity;
  
  Serial.println("");
  Serial.print("Brightness = ");
  Serial.println(lightValue);
  Serial.print("Temperature = ");
  Serial.println(tempValue);
  Serial.print("Humidity = ");
  Serial.println(humValue);
  
  String payload = "{ \"d\" : {";
  payload += "\"light\":\""; payload += lightValue; payload += "\",";
  payload += "\"temp\":\""; payload += tempValue; payload += "\",";
  payload += "\"hum\":\""; payload += humValue; payload += "\"";
  payload += "}}";
  Serial.println(payload);

  if (client.publish(MQTT_TOPIC, (char*) payload.c_str())) {
    Serial.println("Publish ok");
    } 
  else {
    Serial.println("Publish failed");
    }
  delay(delayTime);
}

/*

Simulação com caixa 19.5 x 18.0 x 31.0 cm (ja descontando as margens)
Volume máximo 10.881 ml(cm³)

*/
#include <Ultrasonic.h>
#include <ESP8266WiFi.h> 
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define RELAY 0 //D3

const char* wifi_name = "iot_net";
const char* wifi_pass = "ufersaPPGCC#2024";
const char* mqtt_server = "192.168.0.3";
const char* mqtt_user = "raspi";
const char* mqtt_pass = "iotnet";

WiFiClient espClient; //acesso à camada de transporte(TCP)
PubSubClient client(espClient);

Ultrasonic ultrasonic(5, 4); //D1 e D2 | echo e trigger

JsonDocument mensagem;

void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void mqtt_connect();
int calc_vol();
float calc_per();


void setup() {
  Serial.begin(115200);
  pinMode(RELAY, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  mqtt_connect();
}
void loop() {

  int altura = ultrasonic.read();
  float percentual = calc_per(altura); //altura da superficie da água
  int volume = calc_vol(altura); //altura da superficie da água
  mensagem["nivel"] = percentual;
  mensagem["volume"]= volume;
  mensagem["bomba"]= "desligada"; 
  char saida[60];
  serializeJson(mensagem, saida);
  
  if(percentual<=0.5){
    while (percentual<=100){
      digitalWrite(RELAY, HIGH);
      altura = ultrasonic.read();
      percentual = calc_per(altura); 
      volume = calc_vol(altura);
      mensagem["nivel"] = percentual;
      mensagem["volume"]= volume;
      mensagem["bomba"] = "ligada";
      saida[60];
      serializeJson(mensagem, saida);
      if (client.connected()) {
        client.publish("CaixaDagua",saida);       
      }else{
        mqtt_connect();
      }
      delay(50);
    }
  }
  if (client.connected()) {
    client.publish("CaixaDagua",saida);      
  }else{
    mqtt_connect();
  }
  client.loop();
  delay(1000);
}
//========== FUNÇÕES =============
void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_name);
  WiFi.mode(WIFI_STA); //stação
  WiFi.begin(wifi_name, wifi_pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());//??
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}
void mqtt_connect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP_CaixaDagua";
    if (client.connect(clientId.c_str(),mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
int calc_vol(int altura){
  //altura x 18.0L x 31.0P
  return (19.5-altura)*558;
}
float calc_per(int altura){
  //19.5A x 18.0L x 31.0P = 10881ml
  return (calc_vol(altura)/10881)*100;
}
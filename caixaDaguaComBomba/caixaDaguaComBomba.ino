#include <Ultrasonic.h>   //Erick Simões
#include <ESP8266WiFi.h>  //Espressif
#include <PubSubClient.h> //Nick O'Larry
#include <ArduinoJson.h>  //Benoit Blanchon 
#include <math.h> 

#define RELAY 0 //D3

//FORMATO DA CAIXA D'ÁGUA
#define PARALELEPIPEDO true
#define TRONCO_CONE false

#if PARALELEPIPEDO
/*
  Simulação com caixa 19.5 x 18.0 x 31.0 cm (ja descontando as margens)
  Volume máximo 10.881 ml(cm³)
*/
  #define ALTURA 19.5
  #define LARGURA 18.0
  #define PROFUNDIDADE 31
  #define ALTURA_TAMPA 2 - ALTURA

#elif TRONCO_CONE
/*
  As medidas (em cm) abaixo são de uma caixa d'água de 500l, conforme especificações do fabricante FortLev
*/
  #define ALTURA 58
  #define RAIO_BASE 45
  #define RAIO_TOPO 61
  #define ALTURA_TAMPA 72 - ALTURA
#endif

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
float calcula_capacidade();
float calc_vol(float altura_da_agua);
float calc_per(int distancia_tampa);

float capacidade;

void setup() {
  Serial.begin(115200);
  pinMode(RELAY, OUTPUT);
  setup_wifi();
  mqtt_connect();
  capacidade = calcula_capacidade();
}

void loop() {
  int distancia_tampa = ultrasonic.read();
  int altura_agua = ALTURA - distancia_tampa; // Calcula a altura da água na caixa
  float percentual = calc_per(distancia_tampa); // altura da superficie da água
  float volume = calc_vol(altura_agua); // altura da água
  mensagem["nivel"] = percentual;
  mensagem["volume"] = volume;
  mensagem["bomba"] = "desligada"; 
  char saida[60];
  serializeJson(mensagem, saida);
  
  if(percentual <= 0.5){
    while (percentual <= 100){
      digitalWrite(RELAY, HIGH);
      distancia_tampa = ultrasonic.read();
      altura_agua = ALTURA - distancia_tampa;
      percentual = calc_per(distancia_tampa); 
      volume = calc_vol(altura_agua);
      mensagem["nivel"] = percentual;
      mensagem["volume"] = volume;
      mensagem["bomba"] = "ligada";
      serializeJson(mensagem, saida);
      if (client.connected()) {
        client.publish("CaixaDagua", saida);       
      } else {
        mqtt_connect();
      }
      delay(50);
    }
  }
  if (client.connected()) {
    client.publish("CaixaDagua", saida);      
  } else {
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
  WiFi.mode(WIFI_STA); //estação
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

// void callback(char* topic, byte* payload, unsigned int length) {
//   Serial.print("Message arrived [");
//   Serial.print(topic);
//   Serial.print("] ");
//   for (int i = 0; i < length; i++) {
//     Serial.print((char)payload[i]);
//   }
//   Serial.println();
//   if ((char)payload[0] == '1') {
//     digitalWrite(BUILTIN_LED, LOW);
//   } else {
//     digitalWrite(BUILTIN_LED, HIGH);
//   }
// }

void mqtt_connect() {
  client.setServer(mqtt_server, 1883);
  // client.setCallback(callback);

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP_CaixaDagua";
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

float calcula_capacidade() {
  #if PARALELEPIPEDO
    return ALTURA * LARGURA * PROFUNDIDADE;
  #elif TRONCO_CONE
    return (1.0 / 3.0) * M_PI * ALTURA * (pow(RAIO_BASE, 2) + RAIO_BASE * RAIO_TOPO + pow(RAIO_TOPO, 2));
  #endif
}

#if PARALELEPIPEDO
  float calc_vol(float altura_da_agua) { // caixa d'água em formato de paralelepípedo ou cubo
    altura_da_agua = altura_da_agua - ALTURA_TAMPA
    return altura_da_agua * LARGURA * PROFUNDIDADE;
  }
#elif TRONCO_CONE
  float calc_vol(float altura_da_agua) { // caixa d'água em formato de tronco de cone
    altura_da_agua = altura_da_agua - ALTURA_TAMPA
    return (1.0 / 3.0) * M_PI * altura_da_agua * (pow(RAIO_BASE, 2) + RAIO_BASE * RAIO_TOPO + pow(RAIO_TOPO, 2));
  }
#endif

float calc_per(int altura_da_agua) {
  return (calc_vol(altura_da_agua)/capacidade) * 100;
}

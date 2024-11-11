#include <ESP8266WiFi.h>   //Espressif
#include <PubSubClient.h>  //Nick O'Larry
#include <ArduinoJson.h>   //Benoit Blanchon
#include <math.h>
#include "credenciais.h"  //dados de login e conexão

#define RELAY 0  //D3

//FORMATO DA CAIXA D'ÁGUA
#define PARALELEPIPEDO true
#define TRONCO_CONE false

#if PARALELEPIPEDO
/*
  Simulação com caixa plastica de 20A x 31.0L x 20P (cm) (12,4 litros)
  -2 de nível mímimo (peça da saída de água) e -3 de nível máximo para não transbordar
*/
#define ALTURA_TAMPA 20.0
#define NIVEL_MIN 2
#define NIVEL_MAX (ALTURA_TAMPA - 3)
#define LARGURA 31.0
#define PROFUNDIDADE 20.0

#elif TRONCO_CONE
/*
  As medidas (em cm) abaixo são de uma caixa d'água de 500l, conforme especificações do fabricante FortLev
*/
#define NIVEL_MAX 58
#define NIVEL_MIN 2
#define ALTURA_TOTAL 72.0  // Altura total da caixa d'água com tampa
#define RAIO_BASE 45
#define RAIO_TOPO 61
#endif

WiFiClient espClient;  //acesso à camada de transporte(TCP)
PubSubClient client(espClient);

JsonDocument mensagem;

const int trigPin = 5;  //d1
const int echoPin = 4;  //d2
float capacidade;

void setup_wifi();
// void callback(char* topic, byte* payload, unsigned int length);
void mqtt_connect();
void publica(char saida[60]);
float calcula_capacidade();
float calcula_volume(float altura_da_agua);
float calcula_percentual(int distancia_tampa);
float distancia_cm();


void setup() {
  Serial.begin(115200);
  pinMode(RELAY, OUTPUT);
  //define os terminais do hc-sr04
  pinMode(trigPin, OUTPUT);  
  pinMode(echoPin, INPUT);  
  
  setup_wifi();
  mqtt_connect();
  capacidade = calcula_capacidade();
}

void loop() {
  float distancia_tampa = distancia_cm();  //distancia entre o sensor e a superficie da água
  float volume = calcula_volume(distancia_tampa);
  float percentual = calcula_percentual(distancia_tampa);

  // mensagem["distancia"] = distancia_tampa;
  mensagem["volume"] = volume;
  mensagem["nivel"] = percentual;
  mensagem["bomba"] = String("desligada");

  char saida[60];
  serializeJson(mensagem, saida);
  serializeJson(mensagem, Serial);
  publica(saida);

  if (percentual <= 2) {
    digitalWrite(RELAY, HIGH);
    while (percentual < 100) {
      // distancia_tampa = ultrasonic.read();
      distancia_tampa = distancia_cm();
      volume = calcula_volume(distancia_tampa);
      percentual = calcula_percentual(distancia_tampa);

      // mensagem["distancia"] = distancia_tampa;
      mensagem["volume"] = volume;
      mensagem["nivel"] = percentual;
      mensagem["bomba"] = String("ligada");;

      serializeJson(mensagem, saida);
      serializeJson(mensagem, Serial);
      publica(saida);
      delay(500);
    }
    digitalWrite(RELAY, LOW);
  }
  client.loop();
  delay(500);
}

//========== FUNÇÕES =============

void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_name);
  WiFi.mode(WIFI_STA);  //estação
  WiFi.begin(wifi_name, wifi_pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());  //??
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

void publica(char saida[60]) {
  if (client.connected()) {
    client.publish("CaixaDagua", saida);
  } else {
    mqtt_connect();
  }
}
float calcula_capacidade() {
#if PARALELEPIPEDO
  return (NIVEL_MAX - NIVEL_MIN) * LARGURA * PROFUNDIDADE;  // Capacidade útil
#elif TRONCO_CONE
  return (1.0 / 3.0) * M_PI * (NIVEL_MAX - NIVEL_MIN) * (pow(RAIO_BASE, 2) + RAIO_BASE * RAIO_TOPO + pow(RAIO_TOPO, 2));
#endif
}

#if PARALELEPIPEDO
float calcula_volume(float distancia_tampa) {          // caixa d'água em formato de paralelepípedo ou cubo
  float nivel_total = ALTURA_TAMPA - distancia_tampa; 
  float nivel_util = nivel_total - NIVEL_MIN;          
  if (nivel_util < 0) {
    nivel_util = 0;
  } else if (nivel_util > (NIVEL_MAX - NIVEL_MIN)) {  //acima do máximo
    nivel_util = NIVEL_MAX - NIVEL_MIN;
    digitalWrite(RELAY, LOW);  //força desligamento da bomba
  }
  // Calcular o volume útil
  return nivel_util * LARGURA * PROFUNDIDADE;
}

#elif TRONCO_CONE
float calcula_volume(float distancia_tampa) {  // caixa d'água em formato de tronco de cone
  float nivel_total = ALTURA_TOTAL - distancia_tampa;
  if (nivel_total <= NIVEL_MIN) {
    return 0.0;
  }
  if (nivel_total >= NIVEL_MAX) {
    nivel_total = NIVEL_MAX;
    digitalWrite(RELAY, LOW);  //força desligamento da bomba
  }
  float nivel_util = nivel_total - NIVEL_MIN;
  return (1.0 / 3.0) * M_PI * nivel_util * (pow(RAIO_BASE, 2) + RAIO_BASE * RAIO_TOPO + pow(RAIO_TOPO, 2));
}
#endif

float calcula_percentual(float distancia_tampa) {
  float volume = calcula_volume(distancia_tampa);
  if (volume <= 0) {
    volume = 0;
  }
  return (volume / capacidade) * 100;
}

float distancia_cm() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  float distance = (pulseIn(echoPin, HIGH) * 0.0343)/2;
  return distance ;
}
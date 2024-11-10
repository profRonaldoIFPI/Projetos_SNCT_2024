# Projeto: Monitoramento de Caixa D'Água com ESP8266

Este repositório contém o código para um sistema de monitoramento de uma caixa d'água utilizando um ESP8266. O sistema permite o controle do nível de água na caixa d'água, o acionamento automático de uma bomba de água e a publicação dos dados via MQTT para um broker.

## Descrição Geral

O projeto consiste em um dispositivo IoT que monitoriza o nível de água em uma caixa d'água. Utiliza-se um sensor ultrassônico para medir a altura da água, um relé para controlar a bomba de abastecimento e o ESP8266 para comunicação via WiFi com um servidor MQTT.

### Funcionalidades
- **Monitoramento do Nível de Água**: Utiliza um sensor ultrassônico para medir a altura da água na caixa.
- **Controle Automático da Bomba**: Liga a bomba quando o nível de água está abaixo de um limite especificado e desliga quando o tanque está cheio.
- **Comunicação via MQTT**: Envia os dados de nível de água e estado da bomba para um servidor MQTT, que pode ser visualizado em dashboards.

### Componentes Utilizados
- **ESP8266**: Microcontrolador com WiFi integrado.
- **Sensor Ultrassônico (HC-SR04)**: Mede a distância entre o sensor e a superfície da água para calcular o nível da caixa.
- **Relé**: Para acionamento da bomba d'água.
- **Software MQTT**: Broker MQTT para coleta dos dados do sistema.

## Configuração do Projeto

1. **Conexões de Hardware**:
   - O sensor ultrassônico deve ser conectado nos pinos D1 e D2 do ESP8266 (Echo e Trigger, respectivamente).
   - O relé está conectado ao pino D3 do ESP8266, sendo usado para ligar e desligar a bomba.

2. **Configurações de Software**:
   - Defina o nome e senha da rede WiFi e os dados do servidor MQTT no arquivo `credenciais.h`. Este arquivo deve ser criado pelo usuário e é ignorado pelo Git para manter as credenciais seguras.
   - Exemplo de arquivo `credenciais.h`:
     
     ```cpp
     // credenciais.h
     const char* wifi_name = "seu_wifi";
     const char* wifi_pass = "sua_senha";
     const char* mqtt_server = "192.168.0.3"; 
     const char* mqtt_user = "usuario_mqtt";
     const char* mqtt_pass = "senha_mqtt";
     ```
   - Inclua o arquivo `credenciais.h` no seu código principal utilizando `#include "credenciais.h"`.

3. **Escolha do Formato da Caixa D'Água**:
   - Escolha o formato da caixa d'água alterando as constantes `PARALELEPIPEDO` e `TRONCO_CONE` para `true` ou `false`.

4. **Bibliotecas Utilizadas**:
   - **Ultrasonic** (Erick Simões): Para operação do sensor ultrassônico.
   - **ESP8266WiFi** (Espressif): Para conexão WiFi.
   - **PubSubClient** (Nick O'Larry): Para comunicação MQTT.
   - **ArduinoJson** (Benoit Blanchon): Para formatação e serialização das mensagens JSON enviadas via MQTT.

## Como Utilizar
1. **Clonar o Repositório**: Faça o clone do repositório em sua máquina.
2. **Criar Arquivo de Credenciais**: Crie o arquivo `credenciais.h` na raiz do projeto com as suas credenciais conforme descrito acima.
3. **Upload do Código para o ESP8266**: Abra o código no Arduino IDE e faça o upload para o ESP8266.
4. **Configurar Broker MQTT**: Configure um broker MQTT (pode ser local ou na nuvem) para coletar os dados enviados pelo ESP8266.
5. **Montar o Circuito**: Monte o circuito conforme descrito, conectando o sensor ultrassônico e o relé ao ESP8266.

## Estrutura do Código
- **setup_wifi()**: Configura a conexão WiFi.
- **mqtt_connect()**: Configura e conecta ao servidor MQTT.
- **loop()**: Faz a leitura do sensor, calcula o volume da água, aciona a bomba quando necessário e envia os dados via MQTT.
- **calcula_capacidade()**: Calcula a capacidade total da caixa d'água com base no formato especificado.
- **calcula_volume()**: Calcula o volume atual da água na caixa com base na altura medida.
- **calcula_percentual()**: Calcula o percentual da água na caixa com base na capacidade total.

## Observações
- Certifique-se de definir o formato correto da caixa d'água, pois isso afeta o cálculo do volume e da capacidade.
- O código está configurado para funcionar com dois formatos de caixa d'água: paralelepípedo e tronco de cone.

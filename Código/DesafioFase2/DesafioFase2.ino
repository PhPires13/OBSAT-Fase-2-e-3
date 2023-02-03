/*
 * Autor: Pedro Henrique Gonçalves Pires
 * Data: 29/04/2022
 * CEFET Contagem
 * CEFAST AeroSpace
 * 
 * Kit Educacional
 * Pion Labs
 * CubeSat
*/

/*---------------------------------------- Error Codes ----------------------------------------
 *
 * A serem mostrados em binario nos LEDs frontais juntamente com o led RGB em vermelho
 * 
 * (0001) 1: WiFi nao conectado
 * (0010) 2: Cartao microSD nao conectado ou nao reconhecido
 * (0011) 3: Erro no cartao microSD
 * (0100) 4: HTTP Post retornou codigo de erro
 * 
 * Nota: 1, 2 e 3 prendem enquanto estiver com erro
 * 
 * ---------------------------------------- Error Solutions ----------------------------------------
 * 
 * 1:
 * 2: Desligar o kit e tentar novamente a insercao do cartao
 * 3: 
 * 4: 
*/


/*---------------------------------------- Bibliotecas ----------------------------------------*/

#include "PION_System.h"
#include <time.h>
#include "WiFi.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>


/*---------------------------------------- Structs ----------------------------------------*/

// Estrutura para os parametros do satelite que possuem 3 eixos
struct Axes3 {
  float x, y, z ;
};

// Estrutura para os dados do satelite
struct SateliteData {
  // Dados Atmosfericos
  // Payload da missao
  float CO2Level ; // ppm
  float humidity ; // %
  float luminosity ; // %
  float temperature ; // °C
  // Demais d.a.
  float pressure ; // Pa
  
  // Parametros do Satelite
  uint8_t battery ; // %
  Axes3 gyroscope ; // graus/s com eixos X, Y e Z
  Axes3 accelerometer ; // m/s^2 com eixos X, Y e Z
  Axes3 magnetometer ; // microT com eixos X, Y e Z
};


/*---------------------------------------- Assinaturas Funcoes ----------------------------------------*/

// Assinatura funcoes criadas
//bool isFalling() ; // funçcao para verificar se esta caindo
//bool isRising() ; // funçcao para verificar se esta subindo
void readPayloadAtmosphericData() ; // funcao que le os dados atmosfericos do payload da missao
void readOtherAtmosphericData() ; // funcao que le o restante dos dados atmosfericos
void readSateliteParameters() ; // funcao que le os parametros do satelite
void readAllData() ; // funcao que le todos os dados do satelite
String creatJSONString() ; // Cria um string em formato JSON com os dados
uint8_t sendJSONDataHTTP() ; // funcao que envia os dados em JSON via HTTP e retorna o exito (1) ou falha (0)


/*---------------------------------------- Satelite ----------------------------------------*/

System cubeSat ;
SateliteData sateliteData ;
int numeroEquipe = 270 ;


/*---------------------------------------- Rede ----------------------------------------*/

// Modificação para conectar a uma wifi
void networkConnect(){
//  String networkName = "NET_2GD70E7C" ;
//  String networkPasword = "4BD70E7C" ; //TODO: Trocar para a rede da competicao

  // Para qualquer aplicação bluetooth que possa existir
  btStop();

  // Começa WiFi se conectando ao SSID fornecido com a senha fornecida
  WiFi.begin("OBSAT_WIFI", "OBSatZenith1000"); // "OBSAT_WIFI", "OBSatZenith1000" //TODO: Trocar para a rede da competicao // "NET_2GD70E7C", "4BD70E7C"
  // Espera o Status de conectado
  while (WiFi.status() != WL_CONNECTED) {
    // Error code 1
    cubeSat.setRGB(RED);
    cubeSat.setLed(4, HIGH) ;
    cubeSat.setLed(3, LOW) ;
    cubeSat.setLed(2, LOW) ;
    cubeSat.setLed(1, LOW) ;
    
    delay(500);
  }

  // Apaga os LEDs
  cubeSat.setRGB(OFF) ;
  cubeSat.setLed(ALL_OFF) ;
  
  // Exibe via serial o endereço IP do seu kit na rede 
//  Serial.println("");
//  Serial.println("WiFi conectada");
//  Serial.println("IP address: ");
//  Serial.println(WiFi.localIP());
}


/*---------------------------------------- Setup ----------------------------------------*/
void setup(){
  // Inicializa o CubeSat, e seus periféricos
  cubeSat.init() ;
  cubeSat.createSDLogTask() ; // Criar a Thread que ira salvar no arquivo de armazenamento no cartaoSD

  cubeSat.setLed(1, HIGH);
  delay(250);
  cubeSat.setLed(2, HIGH);
  delay(250);
  cubeSat.setLed(1, LOW);
  delay(250);
  cubeSat.setLed(3, HIGH);
  delay(250);
  cubeSat.setLed(2, LOW);
  delay(250);
  cubeSat.setLed(4, HIGH);
  delay(250);
  cubeSat.setLed(3, LOW);
  delay(250);
  cubeSat.setLed(4, LOW);
  delay(500);
  
  cubeSat.setLed(1, HIGH);
  cubeSat.setLed(2, HIGH);
  cubeSat.setLed(3, HIGH);
  cubeSat.setLed(4, HIGH);
  delay(1000);
  cubeSat.setLed(ALL_OFF);

  cubeSat.buzzer(750);

  // Verificar estado do cartao SD
  uint8_t estadoSD = cubeSat.getSDStatus() ;
  if (estadoSD == 0) {
    // Error code 2
    cubeSat.setRGB(RED);
    cubeSat.setLed(4, LOW) ;
    cubeSat.setLed(3, HIGH) ;
    cubeSat.setLed(2, LOW) ;
    cubeSat.setLed(1, LOW) ;
    // Prende ate que sai do codigo de erro
    while (estadoSD != 1 && estadoSD != 2) {
      delay(250) ;
      estadoSD = cubeSat.getSDStatus() ;
    }
  } else if (estadoSD == 3) {
    // Error code 3
    cubeSat.setRGB(RED);
    cubeSat.setLed(4, HIGH) ;
    cubeSat.setLed(3, HIGH) ;
    cubeSat.setLed(2, LOW) ;
    cubeSat.setLed(1, LOW) ;
    // Prende ate que sai do codigo de erro
    while (estadoSD != 1 && estadoSD != 2) {
      delay(250) ;
      estadoSD = cubeSat.getSDStatus() ;
    }
  }

  // Apaga os LEDs
  cubeSat.setRGB(OFF) ;
  cubeSat.setLed(ALL_OFF) ;
}


/*---------------------------------------- Loop ----------------------------------------*/

//// Estado do satelite - 2: subindo, 3: descendo
//uint8_t estado = 2 ;

// Guarda os tempos em segundo
const time_t beginTime=time(NULL);
time_t lastMeasureTime=time(NULL)-240, actualTime=time(NULL) ;

void loop() {
  // Atualiza os dados do satelite
  readSateliteParameters() ;
  actualTime = time(NULL) ;

  // Verifica o estado do cartao SD
  if (cubeSat.getSDStatus() != 2) { // Caso nao estiver gravando
    cubeSat.activateSDLog(); // Liga a gravacao a cada meio segundo
  }

  // Verifica se deu o tempo de realizar a leitura
  if ((actualTime - lastMeasureTime) >= 240) { // 4 min = 240 s
    //Leitura dos Dados
    readAllData() ;

    // Envia os dados em formato JSON via HTTP
    uint8_t httpReturnCode = sendJSONDataHTTP() ;

    // Verifica o envio do HTTP
    if (httpReturnCode == 1) {
      // Atualiza o tempo da ultima medida
      lastMeasureTime = time(NULL); 
    } else {
      // Faz com que a proxima medida seje daqui a 5 segundos
      lastMeasureTime += 5 ;
    }
  }

//  // Maquina de estados
//  if (estado == 2 && (actualTime - beginTime) > 3600 && isFalling()) {
//    estado = 3 ;
//    //cubeSat.setRGB(PURPLE) ;
//  }
//  if (estado == 3 && isRising()) {
//    estado = 2 ;
//    //cubeSat.setRGB(BLUE) ;
//  }
//
//  // Acoes de cada estado
//  if (estado == 2) { // Estado de subida
//    // Verifica o estado do cartao SD
//    if (cubeSat.getSDStatus() != 2) { // Caso nao estiver gravando
//      // Liga a gravacao a cada meio segundo
//      cubeSat.activateSDLog();
//    }
//  
//    // Verifica se deu o tempo de realizar a leitura
//    if ((actualTime - lastMeasureTime) >= 240) { // 4 min = 240 s
//      //Leitura dos Dados
//      readAllData() ;
//
//      // Envia os dados em formato JSON via HTTP
//      sendJSONDataHTTP() ;
//
//      // Atualiza o tempo da ultima medida
//      lastMeasureTime = time(NULL);
//    }
//    
//  } else if (estado == 3) { // Estado de queda
//    // Apaga os LEDs
//    cubeSat.setRGB(OFF) ;
//    cubeSat.setLed(ALL_OFF) ;
//    
//    // Desativa a gravacao de dados
//    //cubeSat.deactivateSDLog();
//  }
}


/*---------------------------------------- Corpo Funcoes ----------------------------------------*/

//bool isFalling() {
//  if (sateliteData.accelerometer.z < -11) {
//    return true ; 
//  }
//
//  return false ;
//}
////TODO: Achar maneiras de saber se esta subindo ou descendo
//bool isRising() {
//  if (sateliteData.accelerometer.z >= -9.5) {
//    return true ; 
//  }
//
//  return false ;
//}

void readPayloadAtmosphericData() {
  sateliteData.CO2Level = cubeSat.getCO2Level() ;
  sateliteData.humidity = cubeSat.getHumidity() ;
  sateliteData.luminosity = cubeSat.getLuminosity() ;
  sateliteData.temperature = cubeSat.getTemperature() ;
}
void readOtherAtmosphericData() {
  sateliteData.pressure = cubeSat.getPressure() ;
}

void readSateliteParameters() {
  sateliteData.battery = cubeSat.getBattery() ;
  
  sateliteData.gyroscope.x = cubeSat.getGyroscope(0) ;
  sateliteData.gyroscope.y = cubeSat.getGyroscope(1) ;
  sateliteData.gyroscope.z = cubeSat.getGyroscope(2) ;
  
  sateliteData.accelerometer.x = cubeSat.getAccelerometer(0) ;
  sateliteData.accelerometer.y = cubeSat.getAccelerometer(1) ;
  sateliteData.accelerometer.z = cubeSat.getAccelerometer(2) ;
  
  sateliteData.magnetometer.x = cubeSat.getMagnetometer(0) ;
  sateliteData.magnetometer.y = cubeSat.getMagnetometer(1) ;
  sateliteData.magnetometer.z = cubeSat.getMagnetometer(2) ;
}

void readAllData() {
  readPayloadAtmosphericData() ;
  readOtherAtmosphericData() ;
  readSateliteParameters() ;
}

String creatJSONString() {
  // Faz a alocação de 512 Bytes para o JSON
  DynamicJsonDocument json(512);
  
  // Adiciona todas as leituras necessárias com o formato chave:valor do JSON
  json["equipe"] = numeroEquipe;
  json["bateria"] = sateliteData.battery;
  json["temperatura"] = sateliteData.temperature;
  json["pressao"] = sateliteData.pressure;
  // Adiciona as leituras a um array dentro do JSON
  JsonArray gyro = json.createNestedArray("giroscopio");
    gyro.add(sateliteData.gyroscope.x);
    gyro.add(sateliteData.gyroscope.y);
    gyro.add(sateliteData.gyroscope.z); 
  JsonArray accel = json.createNestedArray("acelerometro");
    accel.add(sateliteData.accelerometer.x);
    accel.add(sateliteData.accelerometer.y);
    accel.add(sateliteData.accelerometer.z);
  JsonArray mag = json.createNestedArray("magnetometro");
    mag.add(sateliteData.magnetometer.x);
    mag.add(sateliteData.magnetometer.y);
    mag.add(sateliteData.magnetometer.z);
  json["payload"]["co2"] = sateliteData.CO2Level ;
  json["payload"]["umidade"] = sateliteData.humidity ;
  json["payload"]["luminosidade"] = sateliteData.luminosity ;
  json["payload"]["temperatura"] = sateliteData.temperature ;
  json["payload"]["tempo"] = actualTime ;

  // Serialize JSON document
  String jsonString ;
  serializeJson(json, jsonString);

  return jsonString ;
}

uint8_t sendJSONDataHTTP() {
  String jsonString = creatJSONString() ; // Recebe o JSON

  WiFiClient client;  // WiFiClient for HTTP and WiFiClientSecure for HTTPS
  HTTPClient http;
  
  // Send request
  http.begin(client, "http://192.168.0.1/"); // "http://192.168.0.1/" // TODO: Trocar para endereco da competicao // "http://ptsv2.com/t/DesafioFase2OBSAT3/post"
  int httpReturnCode = 0 ;
  httpReturnCode = http.POST(jsonString);
  
  // Read response
//  Serial.print(http.getString());
  
  // Disconnect
  http.end();

  if (httpReturnCode > 0) {
    // Apaga os LEDs
    cubeSat.setRGB(OFF) ;
    cubeSat.setLed(ALL_OFF) ;

    return 1 ;
  } else {
    // Error code 4
    // Apenas exibe o codigo de erro nos primeiros 10 minutos ja que o restante com certeza vai estar no ceu
    if ((actualTime - beginTime) < 600) {
      cubeSat.setRGB(RED) ;
      cubeSat.setLed(4, LOW) ;
      cubeSat.setLed(3, LOW) ;
      cubeSat.setLed(2, HIGH) ;
      cubeSat.setLed(1, LOW) ;

      delay(1500) ;
    }

    return 0 ;
  }
}

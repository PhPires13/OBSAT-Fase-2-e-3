/*
 * Autor: Pedro Henrique Gonçalves Pires
 * Data: 29/04/2022
 * CEFET Contagem
 * 
 * Kit Educacional
 * Pion Labs
 * CubeSat
*/

#include "PION_System.h"

System cubeSat;

void setup(){
  // Inicializa seu CanSat, e seus periféricos    
  cubeSat.initNoNetwork();
}

void loop() {
  // put your main code here, to run repeatedly:
  
  // Configura os RGB para a cor Azul
  cubeSat.setRGB(BLUE);
  
  cubeSat.setLed(1, HIGH);
  cubeSat.setLed(2, HIGH);
  cubeSat.setLed(3, HIGH);
  cubeSat.setLed(4, HIGH);
  delay(500);
  cubeSat.setLed(ALL_OFF);
  delay(500);
  
  cubeSat.setLed(1, HIGH);
  delay(200);
  
  cubeSat.setLed(2, HIGH);
  delay(100);
  cubeSat.setLed(1, LOW);
  delay(200);
  
  cubeSat.setLed(3, HIGH);
  delay(100);
  cubeSat.setLed(2, LOW);
  delay(200);
  
  cubeSat.setLed(4, HIGH);
  delay(100);
  cubeSat.setLed(3, LOW);
  delay(200);
  
  cubeSat.setLed(3, HIGH);
  delay(100);
  cubeSat.setLed(4, LOW);
  delay(200);
  
  cubeSat.setLed(3, HIGH);
  delay(100);
  cubeSat.setLed(4, LOW);
  delay(200);
  
  cubeSat.setLed(2, HIGH);
  delay(100);
  cubeSat.setLed(3, LOW);
  delay(200);

  cubeSat.setLed(1, HIGH);
  delay(100);
  cubeSat.setLed(2, LOW);
  delay(200);
}

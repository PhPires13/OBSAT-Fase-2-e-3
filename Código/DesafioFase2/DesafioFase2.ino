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
  
  cubeSat.setLed(1, HIGH);
  cubeSat.setLed(2, HIGH);
  cubeSat.setLed(3, HIGH);
  cubeSat.setLed(4, HIGH);
  delay(1000);
  cubeSat.setLed(ALL_OFF);
}

void loop() {
  // put your main code here, to run repeatedly:

  float luminosidade = cubeSat.getLuminosity();
  if (luminosidade <= 1) {
    cubeSat.setRGB(RED);
  } else if (luminosidade > 1 && luminosidade < 100) {
    cubeSat.setRGB(GREEN);
  } else {
    cubeSat.setRGB(BLUE);
  }
}

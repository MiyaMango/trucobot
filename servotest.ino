#include <Servo.h>

Servo servo1, servo2, servo3;

void setup() {
Serial.begin(9600);
servo1.attach(7,500,2500);  // conectar e inicializar todos os servos na posiçao neutra (pins 7,8,9)
servo2.attach(8,500,2500);
servo3.attach(9,500,2500);
servo1.write(15);
servo2.write(15);
servo3.write(15);
Serial.println("Inicializando teste de servos...");
Serial.println("Esperando entrada.");
}

int mexer(char n){
  if(n == '0') delay(100);
  if(n == '1'){
  Serial.println(F("Movendo servo 1"));
  servo1.write(130);
  delay(400);
  servo1.write(15);
  }
  if(n == '2'){
  Serial.println(F("Movendo servo 2"));
  servo2.write(130);
  delay(400);
  servo2.write(15);
  }
  if(n == '3'){
  Serial.println(F("Movendo servo 3"));
  servo3.write(130);
  delay(400);
  servo3.write(15);
  }
  return 1;
}

void loop() {
  while (Serial.available() == 0) {
    delay(100);
  }
  char n = Serial.read();
  mexer(n);
}

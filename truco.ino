/*
 --- pinout do leitor nfc ---
 RST          3 / qualquer PWM, configurar
 SDA(SS)      5 / qualquer PWM, configurar
 MOSI         11
 MISO         12 
 SCK          13
 --- pinout dos servos ---
 servo1       7
 servo2       8
 servo3       9
 --- pinout dos pushbuttons (ñ implementado)---
 pedir ou aumentar truco x 
 fugir truco             x 
 aceitar truco           x
 --- pinout do buzzer ---
 buzzer           2
 --- pinout do display LCD ---
 SDA             A4
 SCL             A5
 */

#include <Buzzer.h>
#include <SPI.h>
#include <Wire.h>
#include <LCD-I2C.h>
#include <Vector.h>
#include <Servo.h>
#include <MFRC522.h>

#define RST_PIN 3         // configuravel, pwm
#define SS_PIN 5         // configuravel, pwm

//instancias
MFRC522 mfrc522(SS_PIN, RST_PIN);  // instancia para leitor NFC
Servo servo1, servo2, servo3; // instancias para os servos
LCD_I2C lcd(0x27, 16, 2); // instancia para o display lcd
Buzzer buzzer(2); // instancia para o buzzer 

void setup() {
	Serial.begin(9600);		// inicializar serial
	SPI.begin();			// inicializar SPI
  Wire.begin();      // inicializar wire
  lcd.begin(&Wire);     //inicializar display
  lcd.display();
  lcd.backlight();
  buzzer.begin(100);      //inicializar buzzer
	mfrc522.PCD_Init();		//inicializar leitor NFC
	delay(50);				// delay p inicializaçao do leitor
  servo1.attach(7,500,2500);  // conectar e inicializar todos os servos na posiçao neutra (pins 7,8,9)
  servo2.attach(8,500,2500);
  servo3.attach(9,500,2500);
  servo1.write(15);
  servo2.write(15);
  servo3.write(15); 
  Serial.println("");
	mfrc522.PCD_DumpVersionToSerial();	//mostrar versao do leitor
	Serial.println(F("Se a versão do leitor é algo estranho, resetar o arduino! (Provável mal contato!)"));
}

// variaveis globais
byte Card[16][4]; //matriz para armazenar os dados do ultimo NFC lido
int carta1, carta2, carta3, cartajogador, j1, j2, j3; //variaveis para armazenar as cartas
int pjogador, probo, pontosemjogo; //variaveis para armazenar os pontos
int maoatual, rodadaatual, vez = 2; //variaveis relacionadas ao estado de jogo
int quemchamou, confianca; //variaveis relacionadas ao truco
int tempo=30, pensamento = 2000; //velocidade que as letras aparecem no display (delay em ms), e tempo de pensamento (ms)

typedef struct{
  int mao, forca1, forca2;
}tabela;

//tabela de confianças de cada mao
const tabela confiancas[503] PROGMEM = {
{.mao = 0b000010000100001, .forca1 = 0, .forca2 = 0},
{.mao = 0b000010000100010, .forca1 = 0, .forca2 = 0},
{.mao = 0b000010000100011, .forca1 = 0, .forca2 = 3},
{.mao = 0b000010000100100, .forca1 = 0, .forca2 = 5},
{.mao = 0b000010000100101, .forca1 = 0, .forca2 = 7},
{.mao = 0b000010000100110, .forca1 = 0, .forca2 = 9},
{.mao = 0b000010000100111, .forca1 = 0, .forca2 = 11},
{.mao = 0b000010000101000, .forca1 = 0, .forca2 = 13},
{.mao = 0b000010000101001, .forca1 = 0, .forca2 = 14},
{.mao = 0b000010000101010, .forca1 = 0, .forca2 = 16},
{.mao = 0b000010000101100, .forca1 = 0, .forca2 = 18},
{.mao = 0b000010000101110, .forca1 = 0, .forca2 = 18},
{.mao = 0b000010000110000, .forca1 = 0, .forca2 = 19},
{.mao = 0b000010000110100, .forca1 = 0, .forca2 = 19},
{.mao = 0b000010001000010, .forca1 = 0, .forca2 = 3},
{.mao = 0b000010001000011, .forca1 = 1, .forca2 = 5},
{.mao = 0b000010001000100, .forca1 = 1, .forca2 = 8},
{.mao = 0b000010001000101, .forca1 = 1, .forca2 = 10},
{.mao = 0b000010001000110, .forca1 = 1, .forca2 = 12},
{.mao = 0b000010001000111, .forca1 = 1, .forca2 = 14},
{.mao = 0b000010001001000, .forca1 = 1, .forca2 = 15},
{.mao = 0b000010001001001, .forca1 = 1, .forca2 = 17},
{.mao = 0b000010001001010, .forca1 = 1, .forca2 = 18},
{.mao = 0b000010001001100, .forca1 = 1, .forca2 = 21},
{.mao = 0b000010001001110, .forca1 = 1, .forca2 = 21},
{.mao = 0b000010001010000, .forca1 = 1, .forca2 = 21},
{.mao = 0b000010001010100, .forca1 = 1, .forca2 = 21},
{.mao = 0b000010001100011, .forca1 = 2, .forca2 = 6},
{.mao = 0b000010001100100, .forca1 = 2, .forca2 = 9},
{.mao = 0b000010001100101, .forca1 = 2, .forca2 = 11},
{.mao = 0b000010001100110, .forca1 = 2, .forca2 = 13},
{.mao = 0b000010001100111, .forca1 = 2, .forca2 = 15},
{.mao = 0b000010001101000, .forca1 = 2, .forca2 = 17},
{.mao = 0b000010001101001, .forca1 = 2, .forca2 = 19},
{.mao = 0b000010001101010, .forca1 = 2, .forca2 = 20},
{.mao = 0b000010001101100, .forca1 = 2, .forca2 = 23},
{.mao = 0b000010001101110, .forca1 = 2, .forca2 = 24},
{.mao = 0b000010001110000, .forca1 = 2, .forca2 = 24},
{.mao = 0b000010001110100, .forca1 = 2, .forca2 = 24},
{.mao = 0b000010010000100, .forca1 = 2, .forca2 = 10},
{.mao = 0b000010010000101, .forca1 = 3, .forca2 = 12},
{.mao = 0b000010010000110, .forca1 = 3, .forca2 = 15},
{.mao = 0b000010010000111, .forca1 = 3, .forca2 = 17},
{.mao = 0b000010010001000, .forca1 = 3, .forca2 = 19},
{.mao = 0b000010010001001, .forca1 = 3, .forca2 = 21},
{.mao = 0b000010010001010, .forca1 = 3, .forca2 = 23},
{.mao = 0b000010010001100, .forca1 = 4, .forca2 = 27},
{.mao = 0b000010010001110, .forca1 = 4, .forca2 = 28},
{.mao = 0b000010010010000, .forca1 = 4, .forca2 = 29},
{.mao = 0b000010010010100, .forca1 = 4, .forca2 = 30},
{.mao = 0b000010010100101, .forca1 = 6, .forca2 = 13},
{.mao = 0b000010010100110, .forca1 = 7, .forca2 = 17},
{.mao = 0b000010010100111, .forca1 = 7, .forca2 = 20},
{.mao = 0b000010010101000, .forca1 = 7, .forca2 = 23},
{.mao = 0b000010010101001, .forca1 = 7, .forca2 = 25},
{.mao = 0b000010010101010, .forca1 = 7, .forca2 = 27},
{.mao = 0b000010010101100, .forca1 = 8, .forca2 = 32},
{.mao = 0b000010010101110, .forca1 = 8, .forca2 = 34},
{.mao = 0b000010010110000, .forca1 = 8, .forca2 = 36},
{.mao = 0b000010010110100, .forca1 = 8, .forca2 = 37},
{.mao = 0b000010011000110, .forca1 = 11, .forca2 = 18},
{.mao = 0b000010011000111, .forca1 = 11, .forca2 = 22},
{.mao = 0b000010011001000, .forca1 = 11, .forca2 = 26},
{.mao = 0b000010011001001, .forca1 = 11, .forca2 = 29},
{.mao = 0b000010011001010, .forca1 = 11, .forca2 = 32},
{.mao = 0b000010011001100, .forca1 = 13, .forca2 = 38},
{.mao = 0b000010011001110, .forca1 = 13, .forca2 = 41},
{.mao = 0b000010011010000, .forca1 = 13, .forca2 = 44},
{.mao = 0b000010011010100, .forca1 = 13, .forca2 = 46},
{.mao = 0b000010011100111, .forca1 = 16, .forca2 = 23},
{.mao = 0b000010011101000, .forca1 = 16, .forca2 = 29},
{.mao = 0b000010011101001, .forca1 = 16, .forca2 = 33},
{.mao = 0b000010011101010, .forca1 = 16, .forca2 = 38},
{.mao = 0b000010011101100, .forca1 = 20, .forca2 = 44},
{.mao = 0b000010011101110, .forca1 = 20, .forca2 = 49},
{.mao = 0b000010011110000, .forca1 = 20, .forca2 = 52},
{.mao = 0b000010011110100, .forca1 = 20, .forca2 = 56},
{.mao = 0b000010100001000, .forca1 = 23, .forca2 = 31},
{.mao = 0b000010100001001, .forca1 = 23, .forca2 = 37},
{.mao = 0b000010100001010, .forca1 = 23, .forca2 = 43},
{.mao = 0b000010100001100, .forca1 = 28, .forca2 = 50},
{.mao = 0b000010100001110, .forca1 = 28, .forca2 = 56},
{.mao = 0b000010100010000, .forca1 = 28, .forca2 = 61},
{.mao = 0b000010100010100, .forca1 = 28, .forca2 = 66},
{.mao = 0b000010100101001, .forca1 = 32, .forca2 = 39},
{.mao = 0b000010100101010, .forca1 = 32, .forca2 = 47},
{.mao = 0b000010100101100, .forca1 = 39, .forca2 = 56},
{.mao = 0b000010100101110, .forca1 = 39, .forca2 = 63},
{.mao = 0b000010100110000, .forca1 = 39, .forca2 = 70},
{.mao = 0b000010100110100, .forca1 = 39, .forca2 = 77},
{.mao = 0b000010101001010, .forca1 = 43, .forca2 = 49},
{.mao = 0b000010101001100, .forca1 = 53, .forca2 = 60},
{.mao = 0b000010101001110, .forca1 = 53, .forca2 = 69},
{.mao = 0b000010101010000, .forca1 = 53, .forca2 = 78},
{.mao = 0b000010101010100, .forca1 = 53, .forca2 = 87},
{.mao = 0b000010110001110, .forca1 = 64, .forca2 = 70},
{.mao = 0b000010110010000, .forca1 = 64, .forca2 = 84},
{.mao = 0b000010110010100, .forca1 = 64, .forca2 = 97},
{.mao = 0b000010111010000, .forca1 = 81, .forca2 = 84},
{.mao = 0b000010111010100, .forca1 = 81, .forca2 = 100},
{.mao = 0b000011000010100, .forca1 = 100, .forca2 = 100},
{.mao = 0b000100001000010, .forca1 = 3, .forca2 = 3},
{.mao = 0b000100001000011, .forca1 = 5, .forca2 = 8},
{.mao = 0b000100001000100, .forca1 = 6, .forca2 = 13},
{.mao = 0b000100001000101, .forca1 = 6, .forca2 = 17},
{.mao = 0b000100001000110, .forca1 = 7, .forca2 = 21},
{.mao = 0b000100001000111, .forca1 = 7, .forca2 = 25},
{.mao = 0b000100001001000, .forca1 = 8, .forca2 = 28},
{.mao = 0b000100001001001, .forca1 = 10, .forca2 = 30},
{.mao = 0b000100001001010, .forca1 = 11, .forca2 = 33},
{.mao = 0b000100001001100, .forca1 = 15, .forca2 = 38},
{.mao = 0b000100001001110, .forca1 = 17, .forca2 = 39},
{.mao = 0b000100001010000, .forca1 = 19, .forca2 = 39},
{.mao = 0b000100001010100, .forca1 = 21, .forca2 = 39},
{.mao = 0b000100001100011, .forca1 = 6, .forca2 = 8},
{.mao = 0b000100001100100, .forca1 = 9, .forca2 = 13},
{.mao = 0b000100001100101, .forca1 = 9, .forca2 = 17},
{.mao = 0b000100001100110, .forca1 = 9, .forca2 = 21},
{.mao = 0b000100001100111, .forca1 = 10, .forca2 = 25},
{.mao = 0b000100001101000, .forca1 = 11, .forca2 = 28},
{.mao = 0b000100001101001, .forca1 = 12, .forca2 = 31},
{.mao = 0b000100001101010, .forca1 = 13, .forca2 = 33},
{.mao = 0b000100001101100, .forca1 = 16, .forca2 = 38},
{.mao = 0b000100001101110, .forca1 = 18, .forca2 = 39},
{.mao = 0b000100001110000, .forca1 = 20, .forca2 = 40},
{.mao = 0b000100001110100, .forca1 = 22, .forca2 = 40},
{.mao = 0b000100010000100, .forca1 = 10, .forca2 = 15},
{.mao = 0b000100010000101, .forca1 = 11, .forca2 = 18},
{.mao = 0b000100010000110, .forca1 = 12, .forca2 = 22},
{.mao = 0b000100010000111, .forca1 = 12, .forca2 = 26},
{.mao = 0b000100010001000, .forca1 = 13, .forca2 = 29},
{.mao = 0b000100010001001, .forca1 = 13, .forca2 = 32},
{.mao = 0b000100010001010, .forca1 = 14, .forca2 = 35},
{.mao = 0b000100010001100, .forca1 = 18, .forca2 = 40},
{.mao = 0b000100010001110, .forca1 = 20, .forca2 = 42},
{.mao = 0b000100010010000, .forca1 = 21, .forca2 = 42},
{.mao = 0b000100010010100, .forca1 = 23, .forca2 = 43},
{.mao = 0b000100010100101, .forca1 = 14, .forca2 = 18},
{.mao = 0b000100010100110, .forca1 = 16, .forca2 = 23},
{.mao = 0b000100010100111, .forca1 = 16, .forca2 = 28},
{.mao = 0b000100010101000, .forca1 = 16, .forca2 = 31},
{.mao = 0b000100010101001, .forca1 = 17, .forca2 = 35},
{.mao = 0b000100010101010, .forca1 = 18, .forca2 = 38},
{.mao = 0b000100010101100, .forca1 = 21, .forca2 = 44},
{.mao = 0b000100010101110, .forca1 = 23, .forca2 = 45},
{.mao = 0b000100010110000, .forca1 = 24, .forca2 = 47},
{.mao = 0b000100010110100, .forca1 = 26, .forca2 = 47},
{.mao = 0b000100011000110, .forca1 = 19, .forca2 = 24},
{.mao = 0b000100011000111, .forca1 = 20, .forca2 = 30},
{.mao = 0b000100011001000, .forca1 = 21, .forca2 = 34},
{.mao = 0b000100011001001, .forca1 = 21, .forca2 = 38},
{.mao = 0b000100011001010, .forca1 = 22, .forca2 = 42},
{.mao = 0b000100011001100, .forca1 = 26, .forca2 = 48},
{.mao = 0b000100011001110, .forca1 = 27, .forca2 = 51},
{.mao = 0b000100011010000, .forca1 = 28, .forca2 = 53},
{.mao = 0b000100011010100, .forca1 = 29, .forca2 = 54},
{.mao = 0b000100011100111, .forca1 = 25, .forca2 = 30},
{.mao = 0b000100011101000, .forca1 = 26, .forca2 = 37},
{.mao = 0b000100011101001, .forca1 = 26, .forca2 = 41},
{.mao = 0b000100011101010, .forca1 = 27, .forca2 = 46},
{.mao = 0b000100011101100, .forca1 = 32, .forca2 = 53},
{.mao = 0b000100011101110, .forca1 = 32, .forca2 = 57},
{.mao = 0b000100011110000, .forca1 = 33, .forca2 = 60},
{.mao = 0b000100011110100, .forca1 = 34, .forca2 = 62},
{.mao = 0b000100100001000, .forca1 = 31, .forca2 = 39},
{.mao = 0b000100100001001, .forca1 = 33, .forca2 = 44},
{.mao = 0b000100100001010, .forca1 = 33, .forca2 = 50},
{.mao = 0b000100100001100, .forca1 = 39, .forca2 = 58},
{.mao = 0b000100100001110, .forca1 = 39, .forca2 = 63},
{.mao = 0b000100100010000, .forca1 = 40, .forca2 = 67},
{.mao = 0b000100100010100, .forca1 = 41, .forca2 = 71},
{.mao = 0b000100100101001, .forca1 = 40, .forca2 = 46},
{.mao = 0b000100100101010, .forca1 = 42, .forca2 = 54},
{.mao = 0b000100100101100, .forca1 = 48, .forca2 = 63},
{.mao = 0b000100100101110, .forca1 = 49, .forca2 = 69},
{.mao = 0b000100100110000, .forca1 = 49, .forca2 = 74},
{.mao = 0b000100100110100, .forca1 = 50, .forca2 = 80},
{.mao = 0b000100101001010, .forca1 = 51, .forca2 = 55},
{.mao = 0b000100101001100, .forca1 = 60, .forca2 = 67},
{.mao = 0b000100101001110, .forca1 = 60, .forca2 = 74},
{.mao = 0b000100101010000, .forca1 = 60, .forca2 = 81},
{.mao = 0b000100101010100, .forca1 = 61, .forca2 = 88},
{.mao = 0b000100110001110, .forca1 = 71, .forca2 = 76},
{.mao = 0b000100110010000, .forca1 = 71, .forca2 = 87},
{.mao = 0b000100110010100, .forca1 = 71, .forca2 = 98},
{.mao = 0b000100111010000, .forca1 = 84, .forca2 = 87},
{.mao = 0b000100111010100, .forca1 = 84, .forca2 = 100},
{.mao = 0b000101000010100, .forca1 = 100, .forca2 = 100},
{.mao = 0b000110001100011, .forca1 = 8, .forca2 = 8},
{.mao = 0b000110001100100, .forca1 = 13, .forca2 = 15},
{.mao = 0b000110001100101, .forca1 = 13, .forca2 = 22},
{.mao = 0b000110001100110, .forca1 = 14, .forca2 = 27},
{.mao = 0b000110001100111, .forca1 = 15, .forca2 = 33},
{.mao = 0b000110001101000, .forca1 = 17, .forca2 = 38},
{.mao = 0b000110001101001, .forca1 = 19, .forca2 = 42},
{.mao = 0b000110001101010, .forca1 = 21, .forca2 = 45},
{.mao = 0b000110001101100, .forca1 = 28, .forca2 = 52},
{.mao = 0b000110001101110, .forca1 = 31, .forca2 = 54},
{.mao = 0b000110001110000, .forca1 = 35, .forca2 = 54},
{.mao = 0b000110001110100, .forca1 = 39, .forca2 = 54},
{.mao = 0b000110010000100, .forca1 = 15, .forca2 = 17},
{.mao = 0b000110010000101, .forca1 = 18, .forca2 = 22},
{.mao = 0b000110010000110, .forca1 = 18, .forca2 = 28},
{.mao = 0b000110010000111, .forca1 = 19, .forca2 = 33},
{.mao = 0b000110010001000, .forca1 = 20, .forca2 = 38},
{.mao = 0b000110010001001, .forca1 = 22, .forca2 = 42},
{.mao = 0b000110010001010, .forca1 = 24, .forca2 = 46},
{.mao = 0b000110010001100, .forca1 = 30, .forca2 = 53},
{.mao = 0b000110010001110, .forca1 = 33, .forca2 = 55},
{.mao = 0b000110010010000, .forca1 = 36, .forca2 = 55},
{.mao = 0b000110010010100, .forca1 = 40, .forca2 = 55},
{.mao = 0b000110010100101, .forca1 = 19, .forca2 = 22},
{.mao = 0b000110010100110, .forca1 = 23, .forca2 = 28},
{.mao = 0b000110010100111, .forca1 = 23, .forca2 = 34},
{.mao = 0b000110010101000, .forca1 = 24, .forca2 = 39},
{.mao = 0b000110010101001, .forca1 = 25, .forca2 = 43},
{.mao = 0b000110010101010, .forca1 = 27, .forca2 = 47},
{.mao = 0b000110010101100, .forca1 = 33, .forca2 = 54},
{.mao = 0b000110010101110, .forca1 = 35, .forca2 = 56},
{.mao = 0b000110010110000, .forca1 = 38, .forca2 = 57},
{.mao = 0b000110010110100, .forca1 = 41, .forca2 = 57},
{.mao = 0b000110011000110, .forca1 = 25, .forca2 = 29},
{.mao = 0b000110011000111, .forca1 = 28, .forca2 = 35},
{.mao = 0b000110011001000, .forca1 = 29, .forca2 = 41},
{.mao = 0b000110011001001, .forca1 = 30, .forca2 = 46},
{.mao = 0b000110011001010, .forca1 = 31, .forca2 = 50},
{.mao = 0b000110011001100, .forca1 = 37, .forca2 = 57},
{.mao = 0b000110011001110, .forca1 = 39, .forca2 = 60},
{.mao = 0b000110011010000, .forca1 = 41, .forca2 = 61},
{.mao = 0b000110011010100, .forca1 = 44, .forca2 = 62},
{.mao = 0b000110011100111, .forca1 = 32, .forca2 = 36},
{.mao = 0b000110011101000, .forca1 = 35, .forca2 = 43},
{.mao = 0b000110011101001, .forca1 = 35, .forca2 = 48},
{.mao = 0b000110011101010, .forca1 = 36, .forca2 = 53},
{.mao = 0b000110011101100, .forca1 = 42, .forca2 = 61},
{.mao = 0b000110011101110, .forca1 = 43, .forca2 = 64},
{.mao = 0b000110011110000, .forca1 = 45, .forca2 = 66},
{.mao = 0b000110011110100, .forca1 = 47, .forca2 = 68},
{.mao = 0b000110100001000, .forca1 = 39, .forca2 = 45},
{.mao = 0b000110100001001, .forca1 = 41, .forca2 = 51},
{.mao = 0b000110100001010, .forca1 = 42, .forca2 = 56},
{.mao = 0b000110100001100, .forca1 = 48, .forca2 = 65},
{.mao = 0b000110100001110, .forca1 = 49, .forca2 = 69},
{.mao = 0b000110100010000, .forca1 = 50, .forca2 = 72},
{.mao = 0b000110100010100, .forca1 = 52, .forca2 = 75},
{.mao = 0b000110100101001, .forca1 = 47, .forca2 = 52},
{.mao = 0b000110100101010, .forca1 = 50, .forca2 = 59},
{.mao = 0b000110100101100, .forca1 = 56, .forca2 = 69},
{.mao = 0b000110100101110, .forca1 = 57, .forca2 = 74},
{.mao = 0b000110100110000, .forca1 = 58, .forca2 = 79},
{.mao = 0b000110100110100, .forca1 = 59, .forca2 = 82},
{.mao = 0b000110101001010, .forca1 = 57, .forca2 = 61},
{.mao = 0b000110101001100, .forca1 = 67, .forca2 = 72},
{.mao = 0b000110101001110, .forca1 = 67, .forca2 = 79},
{.mao = 0b000110101010000, .forca1 = 67, .forca2 = 85},
{.mao = 0b000110101010100, .forca1 = 68, .forca2 = 90},
{.mao = 0b000110110001110, .forca1 = 76, .forca2 = 81},
{.mao = 0b000110110010000, .forca1 = 76, .forca2 = 90},
{.mao = 0b000110110010100, .forca1 = 76, .forca2 = 98},
{.mao = 0b000110111010000, .forca1 = 87, .forca2 = 90},
{.mao = 0b000110111010100, .forca1 = 87, .forca2 = 100},
{.mao = 0b000111000010100, .forca1 = 100, .forca2 = 100},
{.mao = 0b001000010000101, .forca1 = 17, .forca2 = 17},
{.mao = 0b001000010000110, .forca1 = 18, .forca2 = 25},
{.mao = 0b001000010000111, .forca1 = 20, .forca2 = 32},
{.mao = 0b001000010001000, .forca1 = 22, .forca2 = 38},
{.mao = 0b001000010001001, .forca1 = 25, .forca2 = 43},
{.mao = 0b001000010001010, .forca1 = 29, .forca2 = 47},
{.mao = 0b001000010001100, .forca1 = 39, .forca2 = 55},
{.mao = 0b001000010001110, .forca1 = 44, .forca2 = 57},
{.mao = 0b001000010010000, .forca1 = 51, .forca2 = 58},
{.mao = 0b001000010010100, .forca1 = 58, .forca2 = 58},
{.mao = 0b001000010100101, .forca1 = 22, .forca2 = 22},
{.mao = 0b001000010100110, .forca1 = 28, .forca2 = 30},
{.mao = 0b001000010100111, .forca1 = 29, .forca2 = 37},
{.mao = 0b001000010101000, .forca1 = 30, .forca2 = 44},
{.mao = 0b001000010101001, .forca1 = 32, .forca2 = 50},
{.mao = 0b001000010101010, .forca1 = 34, .forca2 = 54},
{.mao = 0b001000010101100, .forca1 = 42, .forca2 = 63},
{.mao = 0b001000010101110, .forca1 = 46, .forca2 = 65},
{.mao = 0b001000010110000, .forca1 = 51, .forca2 = 66},
{.mao = 0b001000010110100, .forca1 = 56, .forca2 = 66},
{.mao = 0b001000011000110, .forca1 = 30, .forca2 = 30},
{.mao = 0b001000011000111, .forca1 = 35, .forca2 = 38},
{.mao = 0b001000011001000, .forca1 = 35, .forca2 = 45},
{.mao = 0b001000011001001, .forca1 = 37, .forca2 = 51},
{.mao = 0b001000011001010, .forca1 = 38, .forca2 = 56},
{.mao = 0b001000011001100, .forca1 = 46, .forca2 = 65},
{.mao = 0b001000011001110, .forca1 = 49, .forca2 = 67},
{.mao = 0b001000011010000, .forca1 = 53, .forca2 = 68},
{.mao = 0b001000011010100, .forca1 = 57, .forca2 = 68},
{.mao = 0b001000011100111, .forca1 = 37, .forca2 = 39},
{.mao = 0b001000011101000, .forca1 = 42, .forca2 = 46},
{.mao = 0b001000011101001, .forca1 = 42, .forca2 = 53},
{.mao = 0b001000011101010, .forca1 = 43, .forca2 = 58},
{.mao = 0b001000011101100, .forca1 = 50, .forca2 = 67},
{.mao = 0b001000011101110, .forca1 = 53, .forca2 = 70},
{.mao = 0b001000011110000, .forca1 = 56, .forca2 = 72},
{.mao = 0b001000011110100, .forca1 = 59, .forca2 = 73},
{.mao = 0b001000100001000, .forca1 = 45, .forca2 = 48},
{.mao = 0b001000100001001, .forca1 = 48, .forca2 = 55},
{.mao = 0b001000100001010, .forca1 = 49, .forca2 = 61},
{.mao = 0b001000100001100, .forca1 = 56, .forca2 = 71},
{.mao = 0b001000100001110, .forca1 = 57, .forca2 = 74},
{.mao = 0b001000100010000, .forca1 = 60, .forca2 = 77},
{.mao = 0b001000100010100, .forca1 = 63, .forca2 = 78},
{.mao = 0b001000100101001, .forca1 = 53, .forca2 = 56},
{.mao = 0b001000100101010, .forca1 = 56, .forca2 = 63},
{.mao = 0b001000100101100, .forca1 = 63, .forca2 = 74},
{.mao = 0b001000100101110, .forca1 = 64, .forca2 = 78},
{.mao = 0b001000100110000, .forca1 = 66, .forca2 = 82},
{.mao = 0b001000100110100, .forca1 = 68, .forca2 = 84},
{.mao = 0b001000101001010, .forca1 = 62, .forca2 = 65},
{.mao = 0b001000101001100, .forca1 = 72, .forca2 = 77},
{.mao = 0b001000101001110, .forca1 = 72, .forca2 = 82},
{.mao = 0b001000101010000, .forca1 = 73, .forca2 = 87},
{.mao = 0b001000101010100, .forca1 = 74, .forca2 = 91},
{.mao = 0b001000110001110, .forca1 = 80, .forca2 = 84},
{.mao = 0b001000110010000, .forca1 = 80, .forca2 = 92},
{.mao = 0b001000110010100, .forca1 = 81, .forca2 = 98},
{.mao = 0b001000111010000, .forca1 = 90, .forca2 = 92},
{.mao = 0b001000111010100, .forca1 = 90, .forca2 = 100},
{.mao = 0b001001000010100, .forca1 = 100, .forca2 = 100},
{.mao = 0b001010010100101, .forca1 = 24, .forca2 = 24},
{.mao = 0b001010010100110, .forca1 = 32, .forca2 = 33},
{.mao = 0b001010010100111, .forca1 = 32, .forca2 = 42},
{.mao = 0b001010010101000, .forca1 = 34, .forca2 = 50},
{.mao = 0b001010010101001, .forca1 = 36, .forca2 = 57},
{.mao = 0b001010010101010, .forca1 = 40, .forca2 = 63},
{.mao = 0b001010010101100, .forca1 = 50, .forca2 = 73},
{.mao = 0b001010010101110, .forca1 = 54, .forca2 = 76},
{.mao = 0b001010010110000, .forca1 = 60, .forca2 = 77},
{.mao = 0b001010010110100, .forca1 = 67, .forca2 = 77},
{.mao = 0b001010011000110, .forca1 = 33, .forca2 = 34},
{.mao = 0b001010011000111, .forca1 = 40, .forca2 = 43},
{.mao = 0b001010011001000, .forca1 = 40, .forca2 = 51},
{.mao = 0b001010011001001, .forca1 = 42, .forca2 = 58},
{.mao = 0b001010011001010, .forca1 = 44, .forca2 = 64},
{.mao = 0b001010011001100, .forca1 = 53, .forca2 = 73},
{.mao = 0b001010011001110, .forca1 = 57, .forca2 = 76},
{.mao = 0b001010011010000, .forca1 = 62, .forca2 = 77},
{.mao = 0b001010011010100, .forca1 = 68, .forca2 = 77},
{.mao = 0b001010011100111, .forca1 = 41, .forca2 = 43},
{.mao = 0b001010011101000, .forca1 = 47, .forca2 = 52},
{.mao = 0b001010011101001, .forca1 = 48, .forca2 = 59},
{.mao = 0b001010011101010, .forca1 = 50, .forca2 = 65},
{.mao = 0b001010011101100, .forca1 = 58, .forca2 = 75},
{.mao = 0b001010011101110, .forca1 = 60, .forca2 = 78},
{.mao = 0b001010011110000, .forca1 = 64, .forca2 = 79},
{.mao = 0b001010011110100, .forca1 = 69, .forca2 = 79},
{.mao = 0b001010100001000, .forca1 = 50, .forca2 = 53},
{.mao = 0b001010100001001, .forca1 = 55, .forca2 = 60},
{.mao = 0b001010100001010, .forca1 = 55, .forca2 = 67},
{.mao = 0b001010100001100, .forca1 = 63, .forca2 = 77},
{.mao = 0b001010100001110, .forca1 = 65, .forca2 = 80},
{.mao = 0b001010100010000, .forca1 = 68, .forca2 = 82},
{.mao = 0b001010100010100, .forca1 = 71, .forca2 = 83},
{.mao = 0b001010100101001, .forca1 = 58, .forca2 = 61},
{.mao = 0b001010100101010, .forca1 = 62, .forca2 = 69},
{.mao = 0b001010100101100, .forca1 = 69, .forca2 = 80},
{.mao = 0b001010100101110, .forca1 = 70, .forca2 = 83},
{.mao = 0b001010100110000, .forca1 = 72, .forca2 = 86},
{.mao = 0b001010100110100, .forca1 = 75, .forca2 = 87},
{.mao = 0b001010101001010, .forca1 = 67, .forca2 = 70},
{.mao = 0b001010101001100, .forca1 = 77, .forca2 = 82},
{.mao = 0b001010101001110, .forca1 = 77, .forca2 = 87},
{.mao = 0b001010101010000, .forca1 = 78, .forca2 = 90},
{.mao = 0b001010101010100, .forca1 = 80, .forca2 = 93},
{.mao = 0b001010110001110, .forca1 = 85, .forca2 = 88},
{.mao = 0b001010110010000, .forca1 = 85, .forca2 = 94},
{.mao = 0b001010110010100, .forca1 = 86, .forca2 = 99},
{.mao = 0b001010111010000, .forca1 = 92, .forca2 = 94},
{.mao = 0b001010111010100, .forca1 = 92, .forca2 = 100},
{.mao = 0b001011000010100, .forca1 = 100, .forca2 = 100},
{.mao = 0b001100011000110, .forca1 = 35, .forca2 = 35},
{.mao = 0b001100011000111, .forca1 = 43, .forca2 = 44},
{.mao = 0b001100011001000, .forca1 = 44, .forca2 = 54},
{.mao = 0b001100011001001, .forca1 = 46, .forca2 = 62},
{.mao = 0b001100011001010, .forca1 = 49, .forca2 = 69},
{.mao = 0b001100011001100, .forca1 = 59, .forca2 = 80},
{.mao = 0b001100011001110, .forca1 = 64, .forca2 = 83},
{.mao = 0b001100011010000, .forca1 = 70, .forca2 = 85},
{.mao = 0b001100011010100, .forca1 = 77, .forca2 = 85},
{.mao = 0b001100011100111, .forca1 = 44, .forca2 = 44},
{.mao = 0b001100011101000, .forca1 = 51, .forca2 = 54},
{.mao = 0b001100011101001, .forca1 = 52, .forca2 = 62},
{.mao = 0b001100011101010, .forca1 = 54, .forca2 = 70},
{.mao = 0b001100011101100, .forca1 = 63, .forca2 = 81},
{.mao = 0b001100011101110, .forca1 = 67, .forca2 = 84},
{.mao = 0b001100011110000, .forca1 = 72, .forca2 = 85},
{.mao = 0b001100011110100, .forca1 = 78, .forca2 = 85},
{.mao = 0b001100100001000, .forca1 = 53, .forca2 = 56},
{.mao = 0b001100100001001, .forca1 = 59, .forca2 = 63},
{.mao = 0b001100100001010, .forca1 = 60, .forca2 = 71},
{.mao = 0b001100100001100, .forca1 = 68, .forca2 = 82},
{.mao = 0b001100100001110, .forca1 = 71, .forca2 = 85},
{.mao = 0b001100100010000, .forca1 = 74, .forca2 = 87},
{.mao = 0b001100100010100, .forca1 = 79, .forca2 = 87},
{.mao = 0b001100100101001, .forca1 = 62, .forca2 = 64},
{.mao = 0b001100100101010, .forca1 = 67, .forca2 = 72},
{.mao = 0b001100100101100, .forca1 = 74, .forca2 = 84},
{.mao = 0b001100100101110, .forca1 = 76, .forca2 = 87},
{.mao = 0b001100100110000, .forca1 = 78, .forca2 = 89},
{.mao = 0b001100100110100, .forca1 = 82, .forca2 = 90},
{.mao = 0b001100101001010, .forca1 = 71, .forca2 = 73},
{.mao = 0b001100101001100, .forca1 = 82, .forca2 = 85},
{.mao = 0b001100101001110, .forca1 = 82, .forca2 = 90},
{.mao = 0b001100101010000, .forca1 = 83, .forca2 = 93},
{.mao = 0b001100101010100, .forca1 = 85, .forca2 = 94},
{.mao = 0b001100110001110, .forca1 = 88, .forca2 = 91},
{.mao = 0b001100110010000, .forca1 = 88, .forca2 = 96},
{.mao = 0b001100110010100, .forca1 = 90, .forca2 = 99},
{.mao = 0b001100111010000, .forca1 = 94, .forca2 = 96},
{.mao = 0b001100111010100, .forca1 = 94, .forca2 = 100},
{.mao = 0b001101000010100, .forca1 = 100, .forca2 = 100},
{.mao = 0b001110011100111, .forca1 = 46, .forca2 = 46},
{.mao = 0b001110011101000, .forca1 = 54, .forca2 = 55},
{.mao = 0b001110011101001, .forca1 = 55, .forca2 = 65},
{.mao = 0b001110011101010, .forca1 = 58, .forca2 = 74},
{.mao = 0b001110011101100, .forca1 = 68, .forca2 = 86},
{.mao = 0b001110011101110, .forca1 = 72, .forca2 = 89},
{.mao = 0b001110011110000, .forca1 = 78, .forca2 = 91},
{.mao = 0b001110011110100, .forca1 = 85, .forca2 = 91},
{.mao = 0b001110100001000, .forca1 = 56, .forca2 = 57},
{.mao = 0b001110100001001, .forca1 = 63, .forca2 = 65},
{.mao = 0b001110100001010, .forca1 = 64, .forca2 = 74},
{.mao = 0b001110100001100, .forca1 = 73, .forca2 = 86},
{.mao = 0b001110100001110, .forca1 = 76, .forca2 = 89},
{.mao = 0b001110100010000, .forca1 = 80, .forca2 = 91},
{.mao = 0b001110100010100, .forca1 = 86, .forca2 = 91},
{.mao = 0b001110100101001, .forca1 = 65, .forca2 = 66},
{.mao = 0b001110100101010, .forca1 = 71, .forca2 = 75},
{.mao = 0b001110100101100, .forca1 = 79, .forca2 = 87},
{.mao = 0b001110100101110, .forca1 = 80, .forca2 = 91},
{.mao = 0b001110100110000, .forca1 = 83, .forca2 = 93},
{.mao = 0b001110100110100, .forca1 = 87, .forca2 = 93},
{.mao = 0b001110101001010, .forca1 = 75, .forca2 = 75},
{.mao = 0b001110101001100, .forca1 = 85, .forca2 = 88},
{.mao = 0b001110101001110, .forca1 = 85, .forca2 = 92},
{.mao = 0b001110101010000, .forca1 = 87, .forca2 = 95},
{.mao = 0b001110101010100, .forca1 = 90, .forca2 = 95},
{.mao = 0b001110110001110, .forca1 = 91, .forca2 = 94},
{.mao = 0b001110110010000, .forca1 = 91, .forca2 = 97},
{.mao = 0b001110110010100, .forca1 = 93, .forca2 = 99},
{.mao = 0b001110111010000, .forca1 = 96, .forca2 = 97},
{.mao = 0b001110111010100, .forca1 = 96, .forca2 = 100},
{.mao = 0b001111000010100, .forca1 = 100, .forca2 = 100},
{.mao = 0b010000100001000, .forca1 = 62, .forca2 = 62},
{.mao = 0b010000100001001, .forca1 = 66, .forca2 = 66},
{.mao = 0b010000100001010, .forca1 = 67, .forca2 = 75},
{.mao = 0b010000100001100, .forca1 = 76, .forca2 = 89},
{.mao = 0b010000100001110, .forca1 = 80, .forca2 = 93},
{.mao = 0b010000100010000, .forca1 = 85, .forca2 = 95},
{.mao = 0b010000100010100, .forca1 = 92, .forca2 = 95},
{.mao = 0b010000100101001, .forca1 = 67, .forca2 = 67},
{.mao = 0b010000100101010, .forca1 = 74, .forca2 = 76},
{.mao = 0b010000100101100, .forca1 = 82, .forca2 = 90},
{.mao = 0b010000100101110, .forca1 = 83, .forca2 = 93},
{.mao = 0b010000100110000, .forca1 = 87, .forca2 = 95},
{.mao = 0b010000100110100, .forca1 = 92, .forca2 = 95},
{.mao = 0b010000101001010, .forca1 = 77, .forca2 = 77},
{.mao = 0b010000101001100, .forca1 = 88, .forca2 = 90},
{.mao = 0b010000101001110, .forca1 = 88, .forca2 = 94},
{.mao = 0b010000101010000, .forca1 = 90, .forca2 = 97},
{.mao = 0b010000101010100, .forca1 = 93, .forca2 = 97},
{.mao = 0b010000110001110, .forca1 = 93, .forca2 = 95},
{.mao = 0b010000110010000, .forca1 = 93, .forca2 = 99},
{.mao = 0b010000110010100, .forca1 = 96, .forca2 = 99},
{.mao = 0b010000111010000, .forca1 = 97, .forca2 = 99},
{.mao = 0b010000111010100, .forca1 = 97, .forca2 = 100},
{.mao = 0b010001000010100, .forca1 = 100, .forca2 = 100},
{.mao = 0b010010100101001, .forca1 = 69, .forca2 = 69},
{.mao = 0b010010100101010, .forca1 = 76, .forca2 = 77},
{.mao = 0b010010100101100, .forca1 = 84, .forca2 = 92},
{.mao = 0b010010100101110, .forca1 = 86, .forca2 = 96},
{.mao = 0b010010100110000, .forca1 = 90, .forca2 = 98},
{.mao = 0b010010100110100, .forca1 = 95, .forca2 = 98},
{.mao = 0b010010101001010, .forca1 = 78, .forca2 = 77},
{.mao = 0b010010101001100, .forca1 = 90, .forca2 = 92},
{.mao = 0b010010101001110, .forca1 = 90, .forca2 = 96},
{.mao = 0b010010101010000, .forca1 = 92, .forca2 = 98},
{.mao = 0b010010101010100, .forca1 = 96, .forca2 = 98},
{.mao = 0b010010110001110, .forca1 = 95, .forca2 = 96},
{.mao = 0b010010110010000, .forca1 = 95, .forca2 = 99},
{.mao = 0b010010110010100, .forca1 = 98, .forca2 = 100},
{.mao = 0b010010111010000, .forca1 = 99, .forca2 = 99},
{.mao = 0b010010111010100, .forca1 = 99, .forca2 = 100},
{.mao = 0b010011000010100, .forca1 = 100, .forca2 = 100},
{.mao = 0b010100101001010, .forca1 = 80, .forca2 = 80},
{.mao = 0b010100101001100, .forca1 = 92, .forca2 = 93},
{.mao = 0b010100101001110, .forca1 = 92, .forca2 = 97},
{.mao = 0b010100101010000, .forca1 = 94, .forca2 = 100},
{.mao = 0b010100101010100, .forca1 = 98, .forca2 = 100},
{.mao = 0b010100110001110, .forca1 = 96, .forca2 = 97},
{.mao = 0b010100110010000, .forca1 = 96, .forca2 = 100},
{.mao = 0b010100110010100, .forca1 = 99, .forca2 = 100},
{.mao = 0b010100111010000, .forca1 = 99, .forca2 = 100},
{.mao = 0b010100111010100, .forca1 = 99, .forca2 = 100},
{.mao = 0b010101000010100, .forca1 = 100, .forca2 = 100},
{.mao = 0b011000111010000, .forca1 = 100, .forca2 = 100},
{.mao = 0b011000111010100, .forca1 = 100, .forca2 = 100},
{.mao = 0b011001000010100, .forca1 = 100, .forca2 = 100},
{.mao = 0b011101000010100, .forca1 = 100, .forca2 = 100}
};


// inicializa a matriz de leitura NFC com zeros
void ResetInfo(){
	for (int i=0; i<=15; i++){
		for (int j=0; j<=4; j++){
			Card[i][j]=0;
		}
	}
}

// preenche a matriz com dados lidos de um tag
void ReadInfo(){

	ResetInfo();
  
  MFRC522::StatusCode status;

  byte buffer[18]; // 16 (data) + 2 (CRC)

  for (byte page=0; page<=15; page+=4){

    byte byteCount = sizeof(buffer);
    status= mfrc522.MIFARE_Read(page,buffer,&byteCount);
    if (status != mfrc522.STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }

    //      [page][index]
    //       0-15   0-3
    //  Card [16]   [4]
    //
    //      0-3   page 0
    //      4-7   page 1
    //      8-11  page 2
    //     12-15  page 3
    //  buffer[16+2]

    int i_=0;
    for (int i=page; i<=page+3; i++){
      for (int j=0; j<=3; j++){
        Card[i][j]=buffer[4*i_ + j];
      }
      i_++;
    }
  }
  
  //  This is to stop the card from sending the info over and over again
  mfrc522.PICC_HaltA();
}

// mostra os dados armazenados na matriz (debug)
/*
void ShowInfo(){

  ReadInfo();

  Serial.println("--------------------------");
  for (int i=0; i<16; i++){
    for (int j=0; j<4; j++){
      Serial.print(Card[i][j],DEC);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println("--------------------------");
}
*/

//mexe servo n e tira carta n da mao do robo. retorna o id da carta jogada
int jogar(int n){
  int idjogada = -1;
  if(n == 1){
  Serial.print(F("Jogando carta 1: "));
  Serial.println(nomear(carta1));
  servo1.write(130);
  delay(400);
  servo1.write(15);
  idjogada = carta1;
  carta1 = -1;
  }
  if(n == 2){
  Serial.print(F("Jogando carta 2: "));
  Serial.println(nomear(carta2));
  servo2.write(130);
  delay(400);
  servo2.write(15);
  idjogada = carta2;
  carta2 = -1;
  }
  if(n == 3){
  Serial.print(F("Jogando carta 3: "));
  Serial.println(nomear(carta3));
  servo3.write(130);
  delay(400);
  servo3.write(15);
  idjogada = carta3;
  carta3 = -1;
  }
  if(n < 1 || n > 3){
    Serial.print(F("Tentando jogar carta inválida! n = "));
    Serial.println(n, DEC);
  }
  return idjogada;
}

//funcao hardcoded que recebe o numero de uma carta e retorna uma string com seu nome
String nomear(int numcarta){
  if(numcarta == 1) return(F("Quatro de ouros"));
  if(numcarta == 2) return(F("Quatro de espadas"));
  if(numcarta == 3) return(F("Quatro de copas"));
  if(numcarta == 4) return(F("Cinco de copas"));
  if(numcarta == 5) return(F("Cinco de espadas"));
  if(numcarta == 6) return(F("Cinco de ouros"));
  if(numcarta == 7) return(F("Cinco de paus"));
  if(numcarta == 8) return(F("Seis de copas"));
  if(numcarta == 9) return(F("Seis de ouros"));
  if(numcarta == 10) return(F("Seis de paus"));
  if(numcarta == 11) return(F("Seis de espadas"));
  if(numcarta == 12) return(F("Sete de espadas"));
  if(numcarta == 13) return(F("Sete de paus"));
  if(numcarta == 14) return(F("Rainha de paus"));
  if(numcarta == 15) return(F("Rainha de ouros"));
  if(numcarta == 16) return(F("Rainha de espadas"));
  if(numcarta == 17) return(F("Rainha de copas"));
  if(numcarta == 18) return(F("Valete de espadas"));
  if(numcarta == 19) return(F("Valete de paus"));
  if(numcarta == 20) return(F("Valete de copas"));
  if(numcarta == 21) return(F("Valete de ouros"));
  if(numcarta == 22) return(F("Rei de ouros"));
  if(numcarta == 23) return(F("Rei de paus"));
  if(numcarta == 24) return(F("Rei de copas"));
  if(numcarta == 25) return(F("Rei de espadas"));
  if(numcarta == 26) return(F("As de copas"));
  if(numcarta == 27) return(F("As de paus"));
  if(numcarta == 28) return(F("As de ouros"));
  if(numcarta == 29) return(F("Dois de ouros")); 
  if(numcarta == 30) return(F("Dois de paus"));
  if(numcarta == 31) return(F("Dois de espadas"));
  if(numcarta == 32) return(F("Dois de copas"));
  if(numcarta == 33) return(F("Tres de ouros"));
  if(numcarta == 34) return(F("Tres de espadas"));
  if(numcarta == 35) return(F("Tres de copas"));
  if(numcarta == 36) return(F("Tres de paus"));
  if(numcarta == 37) return(F("Pica fumo"));
  if(numcarta == 38) return(F("Espadilha"));
  if(numcarta == 39) return(F("Copeta"));
  if(numcarta == 40) return(F("Zap"));
  return("erro ao obter nome da carta");
}

//funcao hardcoded que recebe o numero de uma carta e retorna o valor de sua força no truco mineiro
int forca(int numcarta){
  if(numcarta == 1) return(1); // quatro de ouros
  if(numcarta == 2) return(1); // quatro de espadas
  if(numcarta == 3) return(1); // quatro de copas
  if(numcarta == 4) return(2); // cinco de copas
  if(numcarta == 5) return(2); // cinco de espadas
  if(numcarta == 6) return(2); // cinco de ouros
  if(numcarta == 7) return(2); // cinco de paus 
  if(numcarta == 8) return(3); // seis de copas 
  if(numcarta == 9) return(3); // seis de ouros
  if(numcarta == 10) return(3); // seis de paus
  if(numcarta == 11) return(3); // seis de espadas
  if(numcarta == 12) return(4); // sete de espadas
  if(numcarta == 13) return(4); // sete de copas
  if(numcarta == 14) return(5); // rainha de paus
  if(numcarta == 15) return(5); // rainha de ouros
  if(numcarta == 16) return(5); // rainha de espadas
  if(numcarta == 17) return(5); // rainha de copas
  if(numcarta == 18) return(6); // valete de espadas
  if(numcarta == 19) return(6); // valete de paus
  if(numcarta == 20) return(6); // valete de copas
  if(numcarta == 21) return(6); // valete de ouros
  if(numcarta == 22) return(7); // rei de ouros
  if(numcarta == 23) return(7); // rei de paus
  if(numcarta == 24) return(7); // rei de copas
  if(numcarta == 25) return(7); // rei de espadas
  if(numcarta == 26) return(8); // as de copas
  if(numcarta == 27) return(8); // as de paus
  if(numcarta == 28) return(8); // as de ouros
  if(numcarta == 29) return(9); // dois de ouros 
  if(numcarta == 30) return(9); // dois de paus
  if(numcarta == 31) return(9); // dois de espadas
  if(numcarta == 32) return(9); // dois de copas
  if(numcarta == 33) return(10); // tres de ouros
  if(numcarta == 34) return(10); // tres de espadas
  if(numcarta == 35) return(10); // tres de copas
  if(numcarta == 36) return(10); // tres de paus
  if(numcarta == 37) return(12); // pica fumo
  if(numcarta == 38) return(14); // espadilha
  if(numcarta == 39) return(16); // copas
  if(numcarta == 40) return(20); // zap
  return(-1);
}

//funcao para sons aleatorios no buzzer
void randomsound(int tempo){
  randomSeed(analogRead(0));
  int rng = random(0,4);
  if(rng==0) buzzer.sound(NOTE_E5, tempo);
  if(rng==1) buzzer.sound(NOTE_G5, tempo);
  if(rng==2) buzzer.sound(NOTE_C4, tempo);
  if(rng==3) buzzer.sound(NOTE_A5, tempo);
  if(rng==4) buzzer.sound(NOTE_B5, tempo);
}

//funcao para printar no display lcd. 2 argumentos: primeira linha e segunda linha
void printarlcd(String linha1, String linha2){
  lcd.clear();
  if(linha1.length() > 16 || linha2.length() > 16){
    Serial.println(F("AVISO! Linha maior que 16 tentando ser printada no LCD! Caracteres serão perdidos!"));
  }
  lcd.setCursor(0,0);
  for(int i=0; i<linha1.length(); i++){
    lcd.print(linha1[i]);
    randomsound(tempo);
  }
  lcd.setCursor(0,1);
  for(int i=0; i<linha2.length(); i++){
    lcd.print(linha2[i]);
    randomsound(tempo);
  }
}

//funcao que espera uma tag nfc ser aproximada do leitor, e volta o valor na posiçao (1,5) (posição escolhida para ID na tag)
//também guarda a carta lida para não reler a mesma carta
int lercarta(){
  	// entrar em idle até aproximar um novo cartao nfc
  while( ! mfrc522.PICC_IsNewCardPresent()) {
		delay(500);
	}
  ReadInfo();
  while(Card[5][0] == 0 || Card[5][0] == j1 || Card[5][0] == j2 || Card[5][0] == j3 || Card[5][0] == carta1 || Card[5][0] == carta2 || Card[5][0] == carta3){
    Serial.println(F("Erro na leitura ou carta repetida. Tentando novamente..."));
    printarlcd(F("Erro ou carta"), F("repetida!"));
    while( ! mfrc522.PICC_IsNewCardPresent()) {
		delay(500);
	}
  ReadInfo();
  } 
  int valor = Card[5][0];
  ResetInfo();

  switch(maoatual){
    case 0: Serial.println(F("Lembrando da carta..."));
    break;
    case 1: j1 = valor;
    break;
    case 2: j2 = valor;
    break;
    case 3: j3 = valor;
    break;
    default: Serial.println(F("Erro na leitura de carta: Mão atual não encontrada"));
  }
  return valor;
}

//funcao que recebe uma carta e printa seu numero, forca e nome
void printarcarta(int carta){
  int f = forca(carta);
  Serial.print(F("Seu ID eh: "));
  Serial.println(carta, DEC);
  Serial.print(F("Sua força eh: "));
  Serial.println(f, DEC);
  Serial.print(F("Seu nome eh: "));
  Serial.println(nomear(carta));
}

//encodar uma mao pro formato da tabela
int encode(Vector<int> mao){
  int maoencoded = ((mao[0] << 10) + (mao[1] << 5) + mao[2]);
  return maoencoded;
}

//puxar o nivel de confianca da tabela
int calcularconfianca(){
  int storage_array[3];
  Vector<int> ordenado(storage_array);
  ordenado.push_back(carta1);
  ordenado.push_back(carta2);
  ordenado.push_back(carta3);

  //bubble sort para ordenar cartas por força
  for(int i=0; i<3; i++){
    for(int j=0; j<3-i; j++){
      if(forca(ordenado[j]) > forca(ordenado[j+1])){
        int aux = ordenado[j];
        ordenado[j]=ordenado[j+1];
        ordenado[j+1]=aux;
      }
    }
  }
  int codigo = encode(ordenado);
  Serial.println("O código da mão é:" + String(codigo));

  for(int i=0;i<503;i++){
    if(codigo == confiancas[i].mao){
      if(vez == 1) return confiancas[i].forca2;
      if(vez == 2) return confiancas[i].forca1;
    }
  }

  return -1;
}

//funçao que decide se o robo vai aceitar, fugir ou retrucar um truco. -1 fugir, 0 aceitar, 1 retrucar
int pensartrucorobo(){
  if(confianca < 20) return -1;
  if(confianca < 70) return 0;
  return 1;
}

//funçao que espera uma jogada do jogador, que pode ser uma funçao relacionada a truco, ou uma jogada normal. -1 fugir, 0 jogar normal, 1 truco
int esperarjogador(){
  /*
  todo: esperar qualquer uma das 3 seguintes entradas:
  se botãofugir for apertado, retornar -1
  se uma carta chegar perto do leitor, retornar 0
  se botãotrucar for apertado, retornar 1

  questão: como esperar por algum desses 3 tipos de input diferentes, quando não se sabe qual vai vir?
  */
}

//funçao para o truco. parametro indica quem esta chamando. retorna 0 se o truco nao foi valido, ou 1 se foi
int truco(int p){
  if(p == quemchamou || pontosemjogo == 12) return 0;
  quemchamou = p;

  if(quemchamou == 1){
    Serial.print(F("JOGADOR pede "));
    switch(pontosemjogo){
      case 1: Serial.println(F("truco!"));
      break;
      case 3: Serial.println(F("SEIS!"));
      break;
      case 6: Serial.println(F("NOVE!"));
      break;
      case 9: Serial.println(F("DOZE!"));
      break;
    }
    switch(pensartrucorobo()){
      case -1: pjogador += pontosemjogo;
      Serial.print(F("Robo foge. Jogador ganhou "));
      Serial.print(String(pontosemjogo));
      Serial.println(F(" pontos."));
      novarodada();
      break;
      case 0: if(pontosemjogo == 1) pontosemjogo = 1;
      else pontosemjogo = pontosemjogo +3;
      Serial.print(F("Robo aceita. A partida agora vale "));
      Serial.print(String(pontosemjogo));
      Serial.println(F(" pontos."));
      break;
      case 1: truco(2);
      break;
    }
  }
  if(quemchamou == 2){
    Serial.print(F("ROBO pede "));
    switch(pontosemjogo){
      case 1: Serial.println(F("truco!"));
      break;
      case 3: Serial.println(F("SEIS!"));
      break;
      case 6: Serial.println(F("NOVE!"));
      break;
      case 9: Serial.println(F("DOZE!"));
      break;
    }
    switch(esperarjogador()){
      case -1: probo += pontosemjogo;
      Serial.print(F("Jogador foge. Robo ganhou "));
      Serial.print(String(pontosemjogo));
      Serial.println(F(" pontos."));
      novarodada();
      break;
      case 0: if(pontosemjogo == 1) pontosemjogo = 1;
      else pontosemjogo = pontosemjogo +3;
      Serial.print(F("Jogador aceita. A partida agora vale "));
      Serial.print(String(pontosemjogo));
      Serial.println(F(" pontos."));
      break;
      case 1: truco(1);
      break;
    }
  }
  return 1;
}

//reseta as cartas para começar nova rodada
void resetarcartas(){
  carta1 = 0;
  carta2 = 0;
  carta3 = 0;
  cartajogador = 0;
  j1 = 0;
  j2 = 0;
  j3 = 0;
}

//troca a vez, reseta as cartas, incrementa a rodada, reseta pontos em jogo e le as 3 cartas do robo
void inicializarrodada(){
  vez = vez%2 +1;
  resetarcartas();
  maoatual = 0;
  rodadaatual++;
  pontosemjogo = 1;
  quemchamou = 0;
  
  Serial.print(F("Começando rodada "));
  Serial.println(rodadaatual, DEC);
  Serial.print(F("Pontuacao atual - Jogador "));
  Serial.print(pjogador, DEC);
  Serial.print(F(" X Robo "));
  Serial.println(probo, DEC);

  printarlcd("Rodada atual: " + String(rodadaatual), "Voce " + String(pjogador) +"x"+ String(probo) + " Robo");
  delay(pensamento);

  // ler as 3 cartas do robo
  Serial.println(F("Esperando ler a primeira carta do robo..."));
  printarlcd(F("Esperando carta"), F("#1 do robo..."));
  carta1 = lercarta();

  Serial.println(F("Carta 1 lida!"));
  printarlcd(F("Carta 1 lida!"), F("Ler carta #2"));
  printarcarta(carta1);
  Serial.println(F("Esperando ler a segunda carta do robo..."));
  carta2 = lercarta();

  Serial.println(F("Carta 2 lida!"));
  printarlcd(F("Carta 2 lida!"), F("Ler carta #3"));
  printarcarta(carta2);
  Serial.println(F("Esperando ler a terceira carta do robo..."));
  carta3 = lercarta();

  Serial.println(F("Carta 3 lida!"));
  printarlcd(F("Carta 3 lida!"), F("Comecando rodada"));
  printarcarta(carta3);

  confianca = calcularconfianca();
  Serial.println("A confianca nessa mao é:" + String(confianca));
  delay(pensamento);
}

//joga a maior carta na mao do robo. volta o id da carta
int jogarmaior(){
  int id = -1;
  //Serial.println("Procurando maior carta pra jogar...");
  if(carta1 != -1 && forca(carta1)>=forca(carta2) && forca(carta1)>=forca(carta3)){
    id = carta1;
    jogar(1);
    return id;
  }
  if(carta2 != -1 && forca(carta2)>=forca(carta1) && forca(carta2)>=forca(carta3)){
    id = carta2;
    jogar(2);
    return id;
  }
  if(carta3 != -1 && forca(carta3)>=forca(carta1) && forca(carta3)>=forca(carta2)){
    id = carta3;
    jogar(3);
    return id;
  }
  //Serial.println("Maior carta não encontrada.");
}

//joga a maior não-manilha na mao do robo. volta o id da carta. (caso o robo tem 3 manilhas, joga a menor)
int jogarmaiorescondermanilha(){
  //Serial.println("Procurando maior não-manilha pra jogar...");
  int id = -1;
  if(carta1 != -1 && forca(carta1)>=forca(carta2) && forca(carta1)>=forca(carta3) && forca(carta1) < 11){
    id = carta1;
    jogar(1);
    return id;
  }
  if(carta2 != -1 && forca(carta2)>=forca(carta1) && forca(carta2)>=forca(carta3) && forca(carta2) < 11){
    id = carta2;
    jogar(2);
    return id;
  }
  if(carta3 != -1 && forca(carta3)>=forca(carta1) && forca(carta3)>=forca(carta2) && forca(carta3 < 11)){
    id = carta3;
    jogar(3);
    return id;
  }
  //Serial.println("Menor não-manilha nao encontrada. Jogando menor...");
  return jogarmenor();
}

//joga a menor carta na mao do robo. volta o id da carta
int jogarmenor(){
  int id = -1;
  //Serial.println("Procurando menor carta pra jogar...");
    if(carta1 != -1){
      if(carta2 == -1 && forca(carta1) <= forca(carta3)){
        id = carta1;
        jogar(1);
        return id;
      }
      if(carta3 == -1 && forca(carta1) <= forca(carta2)){
        id = carta1;
        jogar(1);
        return id;
      }
      if(carta2 == -1 && carta3 == -1){
        id = carta1;
        jogar(1);
        return id;
      }
      if(forca(carta1)<=forca(carta2) && forca(carta1)<=forca(carta3)){
        id = carta1;
        jogar(1);
        return id;
      }
    }
    if(carta2 != -1){
      if(carta1 == -1 && forca(carta2) <= forca(carta3)){
        id = carta2;
        jogar(2);
        return id;
      }
      if(carta3 == -1 && forca(carta2) <= forca(carta1)){
        id = carta2;
        jogar(2);
        return id;
      }
      if(carta1 == -1 && carta3 == -1){
        id = carta2;
        jogar(2);
        return id;
      }
      if(forca(carta2)<=forca(carta1) && forca(carta2)<=forca(carta3)){
        id = carta2;
        jogar(2);
        return id;
      }
    }
    if(carta3 != -1){
      if(carta1 == -1 && forca(carta3) <= forca(carta2)){
        id = carta3;
        jogar(3);
        return id;
      }
      if(carta2 == -1 && forca(carta3) <= forca(carta1)){
        id = carta3;
        jogar(3);
        return id;
      }
      if(carta1 == -1 && carta2 == -1){
        id = carta3;
        jogar(3);
        return id;
      }
      if(forca(carta3)<=forca(carta2) && forca(carta3)<=forca(carta1)){
        id = carta3;
        jogar(3);
        return id;
      }
    }
    //Serial.println("Menor carta não encontrada.");
}

//robo joga uma carta (rng, podendo jogar a maior, a maior nao manilha, ou uma aleatoria), e espera uma jogada do usuario. retorna 2 se ganhou, 1 se empatou, 0 se perdeu
//o parametro escolha pode ser colocado em 0 para nao usar rng nenhum, sempre jogando a maior
int comecarrobo(int escolha){
  Serial.println(F("Robo escolhendo carta..."));
  printarlcd(F("Minha vez."), F("Pensando..."));
  delay(pensamento);
  if(pensartrucorobo() == 1) truco(2);
  randomSeed(analogRead(0));
  int rng = random(0,10);
  if(escolha == 0) rng = 0;
  int cartarobo = -1;
  if(rng<2){
    cartarobo = jogarmaior();
  }else if(rng<7){
    cartarobo = jogarmaiorescondermanilha();
  }else if(rng<8 && carta1 != -1){
    cartarobo = jogar(1);
  }else if(rng<9 && carta2 != -1){
    cartarobo = jogar(2);
  }else if(carta3 != -1){
    cartarobo = jogar(3);
  }else{
    jogarmaior();
  }
  Serial.println(F("Esperando carta do jogador..."));
  printarlcd(F("Sua vez."), F("Esperando..."));
  cartajogador = lercarta();
  Serial.print(F("Você jogou: "));
  Serial.println(nomear(cartajogador));
  if(forca(cartarobo)>forca(cartajogador)) return 2;
  if(forca(cartarobo)==forca(cartajogador)) return 1;
  if(forca(cartarobo)<forca(cartajogador)) return 0;
}

//robo joga a carta mais fraca que ganha se possivel, se nao empata, se nao joga a mais fraca; retorna 2 se ganhou a rodada, 1 se empatou, 0 se perdeu
int responderrobo(){
  Serial.println(F("Esperando carta do jogador..."));
  printarlcd(F("Sua vez."), F("Esperando..."));
  cartajogador = lercarta();
  Serial.print(F("Você jogou: "));
  Serial.println(nomear(cartajogador));
  Serial.println(F("Robo escolhendo carta..."));
  printarlcd(F("Minha vez."), F("Pensando..."));
  delay(pensamento);
  if(pensartrucorobo() == 1) truco(2);
  //ver se da pra ganhar, se nao joga a mais fraca
  if(forca(carta1) < forca(cartajogador) && forca(carta2) < forca(cartajogador) && forca(carta3) < forca(cartajogador)){
    jogarmenor();
    return(0);
  }
  
  //jogar a carta suprema
  int storage_array[3];
  Vector<int> maiores(storage_array);
  if (forca(carta1)>forca(cartajogador)) maiores.push_back(carta1);
  if (forca(carta2)>forca(cartajogador)) maiores.push_back(carta2);
  if (forca(carta3)>forca(cartajogador)) maiores.push_back(carta3); 

  if(!maiores.empty()){
    int menor = maiores[0];
    for(auto c : maiores) if(forca(c)<forca(menor)) menor = c;
    if(carta1 == menor){
      jogar(1);
    }
    if(carta2 == menor){
      jogar(2);
    }
    if(carta3 == menor){
      jogar(3);
    }
    return(2);
  }
  //se nao der, empatar jogando a maior
  jogarmaior();
  return(1);
}

//joga uma rodada
void novarodada(){
  inicializarrodada();
  //variaveis para os resultados de cada mao. quemempatou eh 2 se robo, 1 se jogador
  int resultadomao1 = -1, resultadomao2 = -1, resultadomao3 = -1, quemempatou1 = -1, quemempatou2 = -1;

  // inicio da mão 1
  maoatual = 1;
  if(vez == 1){
    Serial.println(F("Rodada começa pelo JOGADOR."));
    printarlcd(F("Voce comeca"), F("essa rodada."));
    delay(pensamento);
    resultadomao1 = responderrobo();
    if(resultadomao1 == 1) quemempatou1 = 2;
  }else{
    Serial.println(F("Rodada começa pelo ROBO."));
    printarlcd(F("O robo comeca"), F("essa rodada."));
    delay(pensamento);
    resultadomao1 = comecarrobo(1);
    if(resultadomao1 == 1) quemempatou1 = 1;
  }

  // inicio da mao 2
  maoatual = 2;
  if(resultadomao1 == 2){
    resultadomao2 = comecarrobo(1);
    if(resultadomao2 == 1) quemempatou2 = 1;
  }
  if(resultadomao1 == 1 && quemempatou1 == 2){
    resultadomao2 = comecarrobo(0);
    if(resultadomao2 == 1) quemempatou2 = 1;
  }
  if(resultadomao1 == 1 && quemempatou1 == 1){
    resultadomao2 = responderrobo();
    if(resultadomao2 == 1) quemempatou1 = 2;
  }
  if(resultadomao1 == 0){
    resultadomao2 = responderrobo();
  }

  //verificar se a rodada já acabou até aqui
  if((resultadomao1 == 2 && resultadomao2 == 2)||(resultadomao1 == 1 && resultadomao2 == 2)||(resultadomao1 == 2 && resultadomao2 == 1)){
    Serial.println("Robo ganha a rodada.");
    probo += pontosemjogo;
    return;
  }
  if((resultadomao1 == 0 && resultadomao2 == 0)||(resultadomao1 == 1 && resultadomao2 == 0)||(resultadomao1 == 0 && resultadomao2 == 1)){
    Serial.println("Jogador ganha a rodada.");
    pjogador += pontosemjogo;
    return;
  }

  //se não acabou, continuar para a mão 3
  maoatual = 3;
  if(resultadomao2 == 2){
    resultadomao3 = comecarrobo(1);
  }
  if(resultadomao2 == 1 && quemempatou2 == 2){
    resultadomao3 = comecarrobo(0);
  }
  if(resultadomao2 == 1 && quemempatou2 == 1){
    resultadomao3 = responderrobo();
  }
  if(resultadomao2 == 0){
    resultadomao3 = responderrobo();
  }

  //verificar ganhador da rodada
  if((resultadomao1 == 1 && resultadomao2 == 1 && resultadomao3 == 2)||((resultadomao1 == 2 || resultadomao2 == 2) && resultadomao3 == 2)){
    Serial.println(F("Robo ganha a rodada."));
    printarlcd(F("Robo ganhou"), F("essa rodada."));
    delay(pensamento);
    probo += pontosemjogo;
    return;
  }else if(resultadomao1 == 1 && resultadomao2 == 1 && resultadomao3 == 1){
    Serial.println(F("Três empates seguidos. Isso nunca deveria acontecer. Ninguém ganha."));
    printarlcd(F("Tres empates."), F("Sem ganhador."));
    delay(pensamento);
    return;
  }else{
    Serial.println(F("Jogador ganha a rodada."));
    printarlcd(F("Voce ganhou"), F("essa rodada."));
    delay(pensamento);
    pjogador += pontosemjogo;
    return;
  }
}

//reinicializa pontos e rodada para começar novo jogo
void novojogo(){
  pjogador = 0;
  probo = 0;
  rodadaatual = 0;
  printarlcd(F("Comecando novo"), F("jogo..."));
  delay(pensamento);
}

//verifica o ganhador e printa no serial
void verificarganhador(){
  if(pjogador>=12){
    Serial.println(F("JOGADOR ganhou! Parabéns!"));
    printarlcd(F("Voce ganhou!"), F("Parabens!"));
  }else if(probo>=12){
    Serial.println(F("ROBO ganhou! ;)"));
    printarlcd(F("ROBO ganhou!"), F(";)"));
  }else{
   Serial.println(F("Erro: ganhador não encontrado."));
  }
  delay(5000);
  Serial.println(F("FIM DO JOGO. Iniciando novo jogo em 10 segundos..."));
  printarlcd(F("FIM DO JOGO."), F("Novo jogo em 10s"));
  delay(10000);
}

void loop() {
  
  novojogo();

  while(pjogador < 12 && probo < 12){
    novarodada();
  }
  
  verificarganhador();
};
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
 pedir ou aumentar truco 0 
 fugir truco             1 
 aceitar truco           2
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

MFRC522 mfrc522(SS_PIN, RST_PIN);  // instancia para leitor NFC
Servo servo1, servo2, servo3; // instancias para os servos
LCD_I2C lcd(0x27, 16, 2); // instancia para o display lcd
Buzzer buzzer(2); // instancia para o buzzer 

void setup() {
	Serial.begin(9600);		// Initialize serial communications with the PC
	SPI.begin();			// Init SPI bus
  Wire.begin();
  lcd.begin(&Wire);
  lcd.display();
  lcd.backlight();
  buzzer.begin(100);
	mfrc522.PCD_Init();		// Init MFRC522
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

//declaraçoes de variaveis globais
byte Card[16][4]; //matriz para armazenar os dados do ultimo NFC lido
int carta1, carta2, carta3, cartajogador, j1, j2, j3; //variaveis para armazenar as cartas
int pjogador, probo, pontosemjogo; //variaveis para armazenar os pontos
int maoatual, rodadaatual, vez = 2; //variaveis relacionadas ao estado de jogo
int tempo=30; //velocidade que as letras aparecem no display (delay em ms)

//int EstadoTruco = 0, EstadoFugir = 0, EstadoAumentar = 0, EstadoAceitar = 0; <- coisas relacionadas a truco, ñ implementado

// inicializa a matriz com zeros
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

// mostra os dados armazenados na matriz
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

//mexe servo n e tira cartan da mao do robo. retorna o id da carta jogada
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
  if(numcarta == 40) return(18); // zap
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
    case 0: Serial.println(F("Setup. Armazenando carta em outra variável..."));
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

// reseta as cartas para começar nova rodada
void resetarcartas(){
  carta1 = 0;
  carta2 = 0;
  carta3 = 0;
  cartajogador = 0;
  j1 = 0;
  j2 = 0;
  j3 = 0;
}

// troca a vez, reseta as cartas, incrementa a rodada, reseta pontos em jogo e le as 3 cartas do robo
void inicializarrodada(){
  vez = vez%2 +1;
  resetarcartas();
  maoatual = 0;
  rodadaatual++;
  pontosemjogo = 1;
  
  Serial.print(F("Começando rodada "));
  Serial.println(rodadaatual, DEC);
  Serial.print(F("Pontuacao atual - Jogador "));
  Serial.print(pjogador, DEC);
  Serial.print(F(" X Robo "));
  Serial.println(probo, DEC);

  printarlcd("Rodada atual: " + String(rodadaatual), "Voce " + String(pjogador) +"x"+ String(probo) + " Robo");
  delay(2000);

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
  delay(2000);
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
  delay(2000);
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
  delay(2000);
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
    delay(2000);
    resultadomao1 = responderrobo();
    if(resultadomao1 == 1) quemempatou1 = 2;
  }else{
    Serial.println(F("Rodada começa pelo ROBO."));
    printarlcd(F("O robo comeca"), F("essa rodada."));
    delay(2000);
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
    delay(2000);
    probo += pontosemjogo;
    return;
  }else if(resultadomao1 == 1 && resultadomao2 == 1 && resultadomao3 == 1){
    Serial.println(F("Três empates seguidos. Isso nunca deveria acontecer. Ninguém ganha."));
    printarlcd(F("Tres empates."), F("Sem ganhador."));
    delay(2000);
    return;
  }else{
    Serial.println(F("Jogador ganha a rodada."));
    printarlcd(F("Voce ganhou"), F("essa rodada."));
    delay(2000);
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
  delay(2000);
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
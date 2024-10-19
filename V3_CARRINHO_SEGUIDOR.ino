/*
  PROJETO: CARRINHO SEGUIDOR DE FAIXA
  VERSÃO: 6.0
  DATA: 21/11/2023
  PROGRAMADOR: CLEYSON
*/

//BIBLIOTECAS
#include <Ultrasonic.h>
#include <MFRC522.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>

//PINAGENS
//MOTOR ESQUERDO
#define in1 8
#define in2 9
#define vmotore 10

//MOTOR DIREITO
#define in3 12
#define in4 13
#define vmotord 11

// Sensores de faixa 
#define ldrd 5
#define ldre 6

// Sensores Ultrassonico
Ultrasonic ultrassom(7,3); // define o nome do sensor(ultrassonico) PINO TRIGER E ECHO RESPECTIVAMENTE

//LED
#define led_verde 4
#define led_vermelho A0

//DEFINIÇÃO DOS PINOS PARA O RFID
#define SS_PIN 10  // Pino SDA RFID
#define RST_PIN 9  // Pino RST do RFID

//VARIÁVEIS
int leituraldrd = 0;
int leituraldre = 0;
int liga=0;
float distancia;
unsigned long tempodistancia =0;
float duration, distanceCm, SOUND_VELOCITY;

//Variáveis para os sensores RFID e armazenamento dos valores das tags
MFRC522 rfid(SS_PIN, RST_PIN);
String pilhaUUIDs[5]; //Array com tamanho para a pilha
int topoDaPilha = -1; //Variavel que representa o topo da pilha

//Inicialização do LCD I2C (endereço pode ser 0x27 ou 0x3F dependendo do modelo testar com ambos)
LiquidCrystal_I2C lcd(0x27, 16, 2); // Endereço do LCD, 16 colunas e 2 linhas

//FUNCTION CAPAZ DE EMPILHAR OS VALORES NA PILHA
void push(String uid) {
  if (topoDaPilha < 4) {  
    topoDaPilha++;
    pilhaUIDs[topoDaPilha] = uid;  
    Serial.print("UID empilhado: ");
    Serial.println(uid);
  } else {
    Serial.println("A pilha está cheia, não é possível empilhar mais UIDs.");
  }
}

//FUNCTION RESPONSAVEL POR DESEMPILHAR UUID ANTES DE APRESENTAR SEU VALOR
String pop() {
  if (topoDaPilha >= 0) {  
    String uid = pilhaUIDs[topoDaPilha];  
    topoDaPilha--; 
    Serial.print("UID desempilhado: ");
    Serial.println(uid);
    return uid;
  } else {
    Serial.println("A pilha está vazia, não é possível desempilhar.");
    return "";
  }
}

//FUNCTION PARA LER O CARTAO RFID
void lerRfid() 
{
  //Verificação se não há um cartão para leitura
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  String uuidStr = "";
  Serial.print("UID da tag: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);

    uuidStr += String(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  push(uuidStr);
  rfid.PICC_HaltA();  
}

//FUNCTION PARA EXIBIR NO LCD OS VALORES LIDOS DESEMPILHADOS
void exibirNoLCD() {
  lcd.clear();  // Limpa o LCD antes de exibir
  String uids[5];  // Array para armazenar os UIDs a serem exibidos
  int numUIDs = 0;  // Contador de UIDs armazenados

  // Coleta todos os UIDs antes de exibir
  while (topoDaPilha >= 0 && numUIDs < 5) {
    uids[numUIDs++] = pop(); 
  }

  // Exibe os UIDs coletados
  for (int i = 0; i < numUIDs; i++) {
    lcd.setCursor(0, 0);
    lcd.print("UID desempilhado:");
    lcd.setCursor(0, 1);
    lcd.print(uids[i]);  // Exibe o UID
    delay(2000);         // Exibe cada UID por 2 segundos
    lcd.clear();         // Limpa o LCD antes de mostrar o próximo UID
  }
}

void setup()
{
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(vmotore, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(vmotord, OUTPUT);
  pinMode(ldrd, INPUT);
  pinMode(ldre, INPUT);  
  pinMode(led_verde, OUTPUT);
  pinMode(led_vermelho, OUTPUT);

  Serial.begin(9600);

  //Inicializa RFID
  SPI.begin();
  rfid.PCD_Init();

  // Inicializa o LCD
  lcd.init();
  lcd.backlight();  // Ativa a luz de fundo do LCD

  delay(5000);
}

void loop()
{
  digitalWrite(led_verde, HIGH);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  analogWrite(vmotore, 83);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(vmotord, 83);  

  while (digitalRead(ldrd) == LOW || digitalRead(ldre) == LOW) {
    digitalWrite (led_vermelho, LOW);    
    digitalWrite(led_verde, HIGH);

    distanceCm = ultrassom.Ranging(CM);

    if (digitalRead(ldrd) == LOW && digitalRead(ldre) == LOW) {      
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      analogWrite(vmotore, 0);
      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
      analogWrite(vmotord, 83); 
    } else if (digitalRead(ldrd) == LOW) { // Ajustes suaves nos motores
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      analogWrite(vmotore, 0);  // Motor direito gira a meia velocidade

      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
      analogWrite(vmotord, 83);
    } else if (digitalRead(ldre) == LOW) {
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      analogWrite(vmotord, 83);

      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
      analogWrite(vmotord, 0);  // Motor esquerdo gira a meia velocidade
    }
  }    

  distanceCm = ultrassom.Ranging(CM);

  if (distanceCm <= 6) {
    digitalWrite(led_verde, LOW);
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(vmotore, 83);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
    analogWrite(vmotord, 83);

    //Chamando a function para ler o cartão RFID
    lerRfid();

    for (int i=0; i<20; i++)
    {
      digitalWrite (led_vermelho, HIGH);
      delay(100);
      digitalWrite (led_vermelho, LOW);
      delay(100);
    }

    //Chamando function responsavel por exibir no LCD os valores(UUIDS) lidos se todos já tiverem armazenados
    if (topo == 4) {
      exibirNoLCD();
      while(1);
    }
  }
  delay(50);
}














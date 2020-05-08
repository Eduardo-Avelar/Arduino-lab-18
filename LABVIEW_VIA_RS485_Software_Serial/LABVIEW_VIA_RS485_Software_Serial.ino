/***************************************************************************
           Arduino Lab 18 - Leirura de dados de um inversor de
          Frequência e de um controlador de temperatura atraves
                    da rede RS-485 Modbus RTU
****************************************************************************
  -- IDE do Arduino Versão 1.8.3
  -- Autor: Eduardo Avelar
  -- Blog: easytromlabs.com
  -- email: contato@easytromlabs.com

  -- Março, 2018
****************************************************************************/
#include <ModbusMaster.h>
#include <SoftwareSerial.h>

#define MAX485_DE      2
#define MAX485_RE_NEG  3

// Instancia Objeto ModbusMaster
ModbusMaster node;

// Instancia Objeto SoftwareSerial
SoftwareSerial mySerial(10, 11); // RX, TX

uint8_t deviceID = 1;
uint8_t resultMain;
String c, texto;
String ID, REGISTER, VALUE;

void preTransmission() {
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission() {
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

void setup() {
  // Atribui pinos como saída
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);

  // inicializa modo de recebimento
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  // Atribui velocidade de comunicação (Baud Rate)
  Serial.begin(9600);
  mySerial.begin(9600);

  // Callbacks - Permite configurar o transeiver RS485 corretamente
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  // Envia caracteres vazios para reiniciar a interface em LabVIEW
  Serial.print("ID2MSGN");
  Serial.println(" ");
  Serial.print("ID1MSGN");
  Serial.println(" ");
  Serial.print("ID1OUTC");
  Serial.println(" ");
  Serial.print("ID1OUTF");
  Serial.println(" ");
  Serial.print("ID1OUTV");
  Serial.println(" ");
  Serial.print("ID1OUTD");
  Serial.println(" ");
  Serial.print("ID1OUTP");
  Serial.println(" ");
  Serial.print("ID1STAT");
  Serial.println(" ");
  Serial.print("ID2TEMP");
  Serial.println(" ");
  Serial.print("ID2STAT");
  Serial.println(" ");
}
void loop() {
  while (Serial.available() > 0) {

    // Slave ID: ID 0-2 > ID1
    // Slave Register: 3-6 > 0005
    // Slave Register Value: 7-11 > 10000

    c = (char)Serial.read();

    if (c == ":") {
      break;
    }
    else {
      texto += c; // texto = texto + c;
    }
    ID = texto.substring(2, 3);
    REGISTER = texto.substring(3, 7);
    VALUE = texto.substring(7, 11);
    deviceID = 0;
  }

  switch (deviceID) {

    // Device ID imaginário - Serve apenas para enviar comandos ao Inversor via RS485
    case 0:
      node.begin(getInt(ID), mySerial);
      resultMain = node.writeSingleRegister(getInt(REGISTER) - 1, getInt(VALUE));
      deviceID = 1;
      texto = "";
      ID = "";
      REGISTER = "";
      VALUE = "";
      break;

    // Rotina que realiza a leitura dos dados pertinenetes ao inversor de frequencia
    case 1:
      // Modbus ID 1 - Inversor de Frequencia
      node.begin(deviceID, mySerial);
      delay(20);
      resultMain = node.readHoldingRegisters(0x0009 - 1, 6); // Realiza a leitura do registrador 9 ate 14 (0x0E)
      delay(20);

      if (resultMain == node.ku8MBSuccess) {
        Serial.print("ID1OUTC");
        Serial.println(node.getResponseBuffer(0x00)); // Apanha o dado alocado na posição 0 do buffer
        Serial.print("ID1OUTF");
        Serial.println(node.getResponseBuffer(0x01)); // Apanha o dado alocado na posição 1 do buffer
        Serial.print("ID1OUTV");
        Serial.println(node.getResponseBuffer(0x02));
        Serial.print("ID1OUTD");
        Serial.println(node.getResponseBuffer(0x03));
        Serial.print("ID1OUTP");
        Serial.println(node.getResponseBuffer(0x04));
        Serial.print("ID1STAT");
        Serial.println(node.getResponseBuffer(0x05));
      } else {
        Serial.print("ID1MSGN");
        Serial.println("Falha de comunicacao");
      }
      deviceID = 2;
      break;

    // Rotina que realiza a leitura dos dados pertinenetes ao controlador de temperatura
    case 2:
      // Modbus ID 2 - Controlador de temperatura
      node.begin(deviceID, mySerial);
      delay(20);
      resultMain = node.readHoldingRegisters(0x0001 - 1, 3);
      delay(20);

      if (resultMain == node.ku8MBSuccess) {
        Serial.print("ID2TEMP");
        Serial.println(node.getResponseBuffer(0x00));
        Serial.print("ID2POTE");
        Serial.println(node.getResponseBuffer(0x03));
      } else {
        Serial.print("ID2MSGN");
        Serial.println("Falha de comunicacao");
      }
      delay(15);
      deviceID = 1;
      break;
  }
  delay(45);
}

// Função que realiza a conversão de um caracter em ASCII para numero inteiro
int getInt(String texto) {
  int temp = texto.length() + 1;
  char buffer[temp];
  texto.toCharArray(buffer, temp);
  int num = atoi(buffer);
  return num;
}

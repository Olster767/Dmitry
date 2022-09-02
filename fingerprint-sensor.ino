#include <Adafruit_Fingerprint.h>
#include<ModbusRtu.h>      
Modbus bus;
unsigned long timer1;
bool attachedFinger = false;
int id;
int outMessage = -1;

// Инициализация Software Serial для датчика отпечатков пальцев
SoftwareSerial mySerial(2, 3); // mySerial(TX, RX)

// Инициализация объекта для датчика отпечатков пальцев
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Инициализация массива в который будет записываться номер "id"
int sentById [1] = {0};

void setup()
{
  bus = Modbus(1,Serial,5);
  bus.begin(9600);
  Serial.begin(9600);
  delay(100);
//  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
}

void loop()                     // run over and over again
{
  if (attachedFinger == false){
    outMessage = bus.getOutCnt();
  }
  // Пока палец не тронул сенсор датчик готов с сканированию
  if(!attachedFinger){
    // Получаем id отпечатка
    id = getFingerprintIDez();
    delay(50);            //don't ned to run this at full speed.
    }

  // Если палец тронул сенсор и прошла 1 секунда, то датчик готов к сканированию следующего отпечатка
  // Слишком частый скан отпечатков приводит к задержке ответа на OPC сервере
  if (attachedFinger == true && millis() - timer1 > 1000){
    attachedFinger = false;
    }


  sentById[0] = id;
  bus.poll(sentById, 1);

  // Эта часть кода отвечает за то чтобы не приходил двойной ответ, чтобы не было такого что сотрудник пришел и тут же ушел
  if (outMessage != bus.getOutCnt() && attachedFinger == true){
    delay(1000);
    id = -1;
    outMessage = bus.getOutCnt();
    }
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  // При сканировании пальца обновляется таймер, датчик не производит сканирование
  attachedFinger = true;
  timer1 = millis();

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
//  Serial.print("Found ID #"); Serial.print(finger.fingerID);
//  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

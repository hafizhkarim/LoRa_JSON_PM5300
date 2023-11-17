#include <LoRa.h>
#include <SD.h>

// define pin
#define ss 5
#define rst 15
#define dio0 4
#define cardSelect 21
#define Buzz 22
#define lampu 2

String LoRaData;                       //menyiapkan variabel untuk menyimpan bacaan dari LoRa
String ID_alat;                        //variabel untuk menyimpan ID alat
File myFile;                           //inisiasi nama perintah File
String namaFile = "/EnergyMeter.txt";  //inisiasi nama file yang akan ditulis


void setup() {
  Serial.begin(115200);  //baudrate alat untuk terhubung melalui komunikasi serial
  pinMode(Buzz, OUTPUT);
  pinMode(lampu, OUTPUT);

  // mengatur LoRa
  LoRa.setPins(ss, rst, dio0);
  LoRa.setGain(6);
  LoRa.setTxPower(20);
  while (!LoRa.begin(915E6)) {  //915 adalah frekuensi LoRa dalam MHz
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xA6);  // samakan ke reciever / transmitter untuk enkripsi data
  Serial.println("LoRa Initializing OK!");

  //memulai SD card
  SD.begin(cardSelect);
  if (!SD.begin(cardSelect)) {  //membaca apakah ada SD card atau tidak
    Serial.println("Gagal Memuat Kartu SD");
    delay(1000);
    ESP.restart();
  } else {
    Serial.println("Kartu SD Terbaca");
  }
}

void loop() {
  //membaca jika ada data masuk dari LoRa
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
      int index = LoRaData.indexOf(";");
      ID_alat = LoRaData.substring(0, index);
      String kirim = ID_alat + " diterima";
      LoRa.beginPacket();
      LoRa.print(kirim);  // mengirim gabungan data JSON ke LoRa
      LoRa.endPacket();
      Serial.print("LoRa: ");
      Serial.println(LoRaData);
      tone(Buzz, 3000, 100);
    }

    //membuka file yang akan diisi
    File myFile = SD.open(namaFile, FILE_APPEND);

    if (!myFile) {                             // cek apakah file ada atau tidak
      myFile = SD.open(namaFile, FILE_WRITE);  // membuat file baru
      // Serial.println("File_Write");  //debug
    } else {
      // Serial.println("File_Append"); //debug
    }
    //Mencatat data ke SD
    if (myFile) {
      myFile.println();
      myFile.print(LoRaData);
      myFile.close();
      Serial.print("Berhasil menulis data ke ");
      Serial.println(namaFile);
      //indikator berhasil menulis ke file
      digitalWrite(lampu, HIGH);
      delay(100);
      digitalWrite(lampu, LOW);
    } else {
      Serial.println("gagal menulis file");
      delay(1000);
      ESP.restart();
    }
  }
  if (Serial.available()) {
    String bukaFile = Serial.readString();
    Serial.println();
    Serial.println(bukaFile);
    File myFile = SD.open(bukaFile, FILE_READ);
    if (myFile) {
      // baca seluruh data di dalam file
      while (myFile.available()) {
        Serial.write(myFile.read());
      }
      myFile.close();
    } else {
      // jika file tidak terbaca
      Serial.println("gagal membuka file");
    }
  }
}

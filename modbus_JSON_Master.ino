//Memanggil Library yang akan dipakai
#include <ArduinoJson.h>
#include <LoRa.h>
#include <SPI.h>

// define pin modul LoRa
#define ss 5
#define rst 15
#define dio0 4

// define ID alat yang akan digunakan sebagai kode unik tiap alat
String ID_alat[] = { "alatX", "alatY", "alatZ" };

// define register yang akan diminta
int reg[] = { 3028, 3110 };

// siapkan variable yang akan digunakan
DynamicJsonDocument kirim(1024);  // variabel JSON
DynamicJsonDocument doc(1024);
double data[100];  // array temp untuk menyimpan data

// fungsi membaca data sejumlah data reg dan menggabungkannya dengan format JSON
void isiJSON() {
  for (int i = 0; reg[i]; i++) {
    kirim["reg"][i] = reg[i];
  }
}

void bacaJSON() {
  for (int i = 0; doc["data"][i]; i++) {
    data[i] = doc["data"][i];
  }
}

void setup() {
  Serial.begin(115200);
  LoRa.setPins(ss, rst, dio0);
  // mengatur kekuatan pancaran LoRa
  LoRa.setGain(6);
  LoRa.setTxPower(20);

  while (!LoRa.begin(915E6)) {  //915 adalah frekuensi LoRa dalam MHz
    Serial.println(".");
    delay(500);  //ulang hingga LoRa tersambung
  }
  LoRa.setSyncWord(0xF3);  // samakan ke reciever / transmitter untuk enkripsi data
  Serial.println("LoRa Initializing OK!");
  isiJSON();
  Serial.println("JSON reg add success");
}

void loop() {

  for (int i = 0; ID_alat[i];i++) {
    kirim["kode"] = ID_alat[i];

    for (int count = 0; count < 10; count++) { //mengulang 10 kali percobaan pengiriman ke 1 alat
      LoRa.beginPacket();
      serializeJson(kirim, LoRa);  // mengirim gabungan data JSON ke LoRa
      LoRa.endPacket();

      //membaca jika ada data masuk dari LoRa
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
        while (LoRa.available()) {
          String LoRaData = LoRa.readString();
          Serial.print("LoRa: ");
          Serial.println(LoRaData);
          deserializeJson(doc, LoRaData);
          String kode = doc["kode"];
          Serial.print("kode = ");
          Serial.println(kode);
          if (kode == ID_alat[i]) {  //mengecek apakah kode yang diterima sesuai dengan ID alat
            bacaJSON();
            break; // keluar dari loop pengiriman untuk memulai pengiriman ke alat selanjutnya

            // mengeluarkan data ke Serial
            Serial.println("Data = ");
            for (int i = 0; doc["reg"][i]; i++) {  // mengambil sejumlah data sesuai jumlah register yang diterima
              int reg = doc["reg"][i];
              Serial.print(reg);
              Serial.print(" data ");
              Serial.println(data[i]);
            }
          }
          serializeJson(doc, Serial);  // mengirim gabungan data JSON ke Serial
          LoRa.beginPacket();
          serializeJson(doc, LoRa);  // mengirim gabungan data JSON ke LoRa
          LoRa.endPacket();
          Serial.println();
        }
      } else {
        delay(1000); // menunggu lalu mengulang pegiriman
      }
    }
  }
}

//Memanggil Library yang akan dipakai
#include <ArduinoJson.h>
#include <ModbusMaster.h>
#include <LoRa.h>
#include <SPI.h>

// define pin modul LoRa
#define ss 5
#define rst 15
#define dio0 4

// define ID alat yang akan digunakan sebagai kode unik tiap alat
#define ID_alat "alatX"

// define parameter modbus
#define baud 19200
#define ID_meter 1

// siapkan variable yang akan digunakan
DynamicJsonDocument doc(1024);  // variabel JSON
double data[100];               // array temp untuk menyimpan data

ModbusMaster node;  //define kode perintah modbus

float HexTofloat(uint32_t x) {
  return (*(float*)&x);
}

uint32_t FloatTohex(float x) {
  return (*(uint32_t*)&x);
}

// fungsi membaca data Power Meter dengan keluaran Float
float Read_Meter_float(char addr, uint16_t REG) {
  float i = 0;
  uint8_t result, j;
  uint16_t data[2];
  uint32_t value = 0;
  node.begin(ID_meter, Serial2);
  result = node.readHoldingRegisters(REG, 2);  ///< Modbus function 0x03 Read Holding Registers
  delay(100);
  if (result == node.ku8MBSuccess) {
    for (j = 0; j < 2; j++) {
      data[j] = (node.getResponseBuffer(j));
    }
    //menampilkan data dalam HEX
    Serial.print(data[1], HEX);
    Serial.println(data[0], HEX);
    //mengubah data menjadi Float
    value = data[0];
    value = value << 16;
    value = value + data[1];
    i = HexTofloat(value);
    Serial.println("Connect modbus Ok.");  //debug modbus connect
    return i;
  } else {
    Serial.print("Connect modbus fail. REG >>> ");
    Serial.println(REG);  // Debug gagal connect modbus
    delay(100);
    return 0;
  }
}

// fungsi membaca data dari power meter
double bacaData(int Reg_addr) {
  double c = Read_Meter_float(ID_meter, Reg_addr - 1);  // -1 karena kebutuhan alat (AKAN BERBEDA UNTUK TIAP ALAT)
  return c;
}

// fungsi membaca data sejumlah data reg dan menggabungkannya dengan format JSON
void ambilDataJSON() {
  for (int i = 0; doc["reg"][i]; i++) {
    data[i] = bacaData(doc["reg"][i]);
    doc["data"][i] = data[i];
  }
}


void setup() {
  Serial.begin(115200);
  Serial2.begin(baud, SERIAL_8E1);  // Serial 8E1 menunjukkan parity even 8 data bit
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
}

void loop() {

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
      if (kode == ID_alat) {  //mengecek apakah kode yang dikirim sesuai dengan ID alat
        ambilDataJSON();
        // mengeluarkan data ke Serial
        // Serial.println("Data = "); 
        // for (int i = 0; doc["reg"][i]; i++) {  // mengambil sejumlah data sesuai jumlah register yang diterima
        //   int reg = doc["reg"][i];
        //   Serial.print(reg);
        //   Serial.print(" data ");
        //   Serial.println(data[i]);
        // }

        serializeJson(doc, Serial);  // mengirim gabungan data JSON ke Serial
        //if(data[0]>0.1){
        LoRa.beginPacket();
        serializeJson(doc, LoRa);  // mengirim gabungan data JSON ke LoRa
        LoRa.endPacket();
        //}
        Serial.println();
      }
    }
  }
}

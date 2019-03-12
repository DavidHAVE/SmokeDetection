/*
   Arduino Uno        LCD I2C
   5V                   VCC
   GND                  GND
   A4                   SDA
   A5                   SCL

   Arduino Uno          MQ2
   5V                   VCC
   GND                  GND
   A0                   A0

   Arduino Uno       SIM Module
   5v                   VCC (3.7V - 4.2V)
   GND                  GND
   D7                   TX
   D8                   RX

   Arduino Uno        Buzzer
   GND                  GND
   D5                    +
*/

//Library Untuk LCD dengan komunikasi I2C
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//Library Untuk SIM800L
#include <gprs.h>
#include <SoftwareSerial.h> //Library untuk software serial


//Sensor
int buzzerPin = 5; //Buzzer Pada Pin Digital 4
int mq2Pin = A0; //Sensor MQ2 pada pin digital Analog 0
//Nilai pemicu sensor yang dikatakan mendeteksi asap dan menyalakan buzzer
int sensorThres = 700;

//Konfgurasi LCD pada komunikasi I2C
LiquidCrystal_I2C lcd(0x27, 16, 2); //0x27 adalah alamat lcd dari komunikasi I2C, 16 adalah jumlah kolom, dan 2 adalah jumlah baris lcd

GPRS gprs; //Membuat objek baru dari GPRS yang berguna untuk mengirim sms

boolean isAlarm = false; //Variabel boolean untuk menandakan apakah terdeteksi asap
boolean isBuzzer = false; //Varoabel boolean untuk menandakan apakah buzzer sudah berbunyi
long previousMillis; //Untuk menyimpan nilai mili sekon


//Fungsi yang dijalankan pertama kali dan sekali saat Arduino dinyalakan, berfungsi untuk persiapan sistem
void setup() {
  pinMode(buzzerPin, OUTPUT); //Konfigurasi pin buzzer menjadi perangkat output
  pinMode(mq2Pin, INPUT); //Konfigurasi pin MQ2 menjadi perangkat input

  lcd.begin(); //memulai lcd
  lcd.backlight(); //menyalakan backlight
  Serial.begin(9600); //Memulai komunikasi serial dengan kecepatan 9600 baud rate, berguna untuk menampilkan tulisan pada serial monitor
  Serial.println("===DETEKSI GAS==="); //Menampilkan tulisan ke serial monitor
  lcd.setCursor(0, 0); //Meletakkan kursor lcd pada kolom 0 baris 0
  lcd.print("DETEKSI GAS"); //Menampilkan tulisan ke lcd
  //Animasi pembuka pada LCD
  for (int i = 0; i < 16; i++) {
    lcd.scrollDisplayRight(); //scroll kanan
    delay(300); //delay 2,5 detik
  }
  lcd.clear(); //Menghapus semua konten pada lcd

  gprs.preInit(); //Memulai gprs
  delay(1000); //Delay 1 detik
  while (0 != gprs.init()) { //Jika gprs tidak bisa dimulai maka
    delay(1000);
    //Terdapat pesan error
    Serial.print("init error\r\n"); //pesan di Serial Monitor jika proses init module GPRS Gagal
  }
  Serial.println("Init succes..."); //pesan di Serial Monitor jika proses init module GPRS Sukses
  delay(1000);
}

//Fungsi untuk mengirim pesan
void sendMessage() {
  gprs.sendSMS("089697779303", "Terdeteksi Asap!"); //Mengirim pesan
}


//Fungsi yang dieksekusi setelah setup yang dijalankan secara berulang-ulang
void loop() {
  long currentMillis = millis(); //stopwatch dalam mili sekon

  if (currentMillis == 0) { //Jika waktu saat ini 0 detik maka
    previousMillis = 0; //Memasukkan nilai 0 kedalam variabel previousMillis
  }

  if (isAlarm) { //Bila isAlarm bernilai true maka
    Serial.print("current :");
    Serial.println(currentMillis - previousMillis);

    if ((currentMillis - previousMillis >= 60000)) { //Menunggu sampai 1 menit
      Serial.println("Mengirim Pesan"); //Menampilkan ke Serial Monitor
      sendMessage(); //Mengirim pesan

      previousMillis = currentMillis; //Mengisi nilai waktu saat ini dalam mili sekon keadalam variabel previousMillis
    }
  } else {
    previousMillis = currentMillis; //Mengisi nilai waktu saat ini dalam mili sekon keadalam variabel previousMillis
  }

  //Sensor akan mengeluarkan nilai dari 0-1023, semakin tinggi nilai semakin pekat asap
  int mq2Value = analogRead(mq2Pin); //Membaca nilai dari pin analog MQ2 lalu disimpan kedalam variabel mq2Value
  Serial.print("Nilai MQ2: "); //Menampilkan Nilai Sensor ke Serial Monitor
  Serial.println(mq2Value);

  // Cek Apakah Nilai Sensor Sudah diatas Nilai Pemicu
  if (mq2Value > sensorThres) { //Bila ada asap
    lcd.setCursor(0, 0); //Kursor lcd pada posisi kolom 0 dan baris 0
    lcd.print("Rokok Harap"); //Menampilkan di lcd
    lcd.setCursor(0, 1); //Kursor lcd pada posisi kolom 0 dan baris 1
    lcd.print("Dimatikan"); //Menampilkan di lcd

    digitalWrite(buzzerPin, HIGH); //Mengubah nilai pin digital pada buzzer menjadi HIGH menjadikan buzzer berbunyi
    isAlarm = true; //Variabel boolean isAlarm menjadi true
    //   tone(buzzer, 1000, 200);
  } else {
    lcd.clear(); //Menghapus semua konten pada lcd
    digitalWrite(buzzerPin, LOW);  //Mengubah nilai pin digital pada buzzer menjadi LOW menjadikan buzzer mati
    isAlarm = false; //Variabel boolean isAlarm menjadi false
    //    noTone(buzzer);
  }
}

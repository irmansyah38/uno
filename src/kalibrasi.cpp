// #include <Arduino.h>
// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>
// #include "HX711.h"

// #define DOUT A0
// #define CLK A1

// LiquidCrystal_I2C lcd(0x27, 16, 2); // Alamat I2C dan ukuran LCD Anda
// HX711 scale(DOUT, CLK);

// float calibration_factor = 650;
// int GRAM;

// void setup()
// {
//   Serial.begin(9600);
//   lcd.init();      // Inisialisasi LCD
//   lcd.backlight(); // Nyalakan backlight
//   lcd.setCursor(0, 0);
//   lcd.print("Scale Calibration");
//   lcd.setCursor(0, 1);
//   lcd.print("Factor: ");
//   lcd.print(calibration_factor);
//   delay(2000);
//   lcd.clear(); // Bersihkan tampilan LCD
//   Serial.println("tekan a,s,d,f untuk menaikan calibration_factor ke 10,100,1000,10000");
//   Serial.println("tekan z,x,c,v untuk menurunkan calibration_factor ke 10,100,1000,10000");
//   Serial.println("Tekan T untuk Tare");
//   scale.set_scale();
//   scale.tare();
//   long zero_factor = scale.read_average();
//   Serial.print("Zero factor: ");
//   Serial.println(zero_factor);
//   delay(1000);
// }

// void loop()
// {
//   scale.set_scale(calibration_factor);
//   GRAM = scale.get_units(), 4;
//   Serial.print("Reading: ");
//   Serial.print(GRAM);
//   Serial.print(" Gram");
//   Serial.print(" calibration_factor: ");
//   Serial.print(calibration_factor);
//   Serial.println();

//   lcd.setCursor(0, 0);
//   lcd.print("Weight: ");
//   lcd.print(GRAM);
//   lcd.print(" g");

//   lcd.setCursor(0, 1);
//   lcd.print("Calibration: ");
//   lcd.print(calibration_factor);

//   if (Serial.available())
//   {
//     char temp = Serial.read();
//     if (temp == '+' || temp == 'a')
//       calibration_factor += 0.1;
//     else if (temp == '-' || temp == 'z')
//       calibration_factor -= 0.1;
//     else if (temp == 's')
//       calibration_factor += 10;
//     else if (temp == 'x')
//       calibration_factor -= 10;
//     else if (temp == 'd')
//       calibration_factor += 100;
//     else if (temp == 'c')
//       calibration_factor -= 100;
//     else if (temp == 'f')
//       calibration_factor += 1000;
//     else if (temp == 'v')
//       calibration_factor -= 1000;
//     else if (temp == 't')
//     {
//       scale.tare();
//       lcd.clear(); // Bersihkan tampilan LCD setelah tare
//     }
//   }
// }

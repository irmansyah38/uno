#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Fuzzy.h>
#include <SoftwareSerial.h>
#include <HX711.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// pin
const int phPin = A2;
#define dtPin A0
#define sckPin A1
const uint8_t suhuPin = 4;
const uint8_t asamPin = 3;
const uint8_t basaPin = 5;
const uint8_t aquascapePin = 7;
const uint8_t mySerialRxPin = 8;
const uint8_t mySerialTxPin = 10;

// calibration factor for scale
float calibrationFactor = 183.60;

// value sensor
float phValue;
float suhuValue;
uint8_t outputValue;
bool giveFluid = false;
bool wait = true;
bool asamOrBasa; // true untuk asam dan false untuk basa
bool emptyAsam = false;
bool emptyBasa = false;
const long interval = 1000;
unsigned long previousMillis = 0;
String data;
bool change = false;

/*                                 make object                                                     */
HX711 scaleSensor(dtPin, sckPin);                      // scale
OneWire onewire(suhuPin);                              // suhu
DallasTemperature suhuSensor(&onewire);                // suhu
SoftwareSerial mySerial(mySerialRxPin, mySerialTxPin); // serial komunikasi tambahan
LiquidCrystal_I2C lcd(0x27, 20, 4);                    // lcd
Fuzzy *fuzzy = new Fuzzy();                            // objecct fuzzy

/*                               Functions                                   */

void displayFloatValue(int coloum, int row, float number)
{
  lcd.setCursor(coloum, row);
  lcd.print(number);
}

void displaySentance(int coloum, int row, String sentance)
{
  lcd.setCursor(coloum, row);
  lcd.print(sentance);
}

void displaySensors()
{
  displaySentance(0, 0, "Suhu    :");
  displayFloatValue(9, 0, suhuValue);

  displaySentance(0, 1, "PH      :");
  displayFloatValue(9, 1, phValue);
}

// function for fuzzy setup
void fuzzySetup()
{
  // Fuzzy Input
  FuzzyInput *inputph = new FuzzyInput(1);
  FuzzySet *SangatAsam = new FuzzySet(0, 0, 2.9, 3);
  inputph->addFuzzySet(SangatAsam);
  FuzzySet *Asam = new FuzzySet(2, 3, 5, 5.5);
  inputph->addFuzzySet(Asam);
  FuzzySet *MendekatiAsam = new FuzzySet(5, 5.1, 6.7, 6.9);
  inputph->addFuzzySet(MendekatiAsam);
  FuzzySet *Normal = new FuzzySet(6.8, 6.8, 7.3, 7.3);
  inputph->addFuzzySet(Normal);
  FuzzySet *MendekatiBasa = new FuzzySet(7.2, 7.4, 8.9, 9);
  inputph->addFuzzySet(MendekatiBasa);
  FuzzySet *Basa = new FuzzySet(8.5, 9, 11, 12);
  inputph->addFuzzySet(Basa);
  FuzzySet *SangatBasa = new FuzzySet(11, 11.1, 14, 14);
  inputph->addFuzzySet(SangatBasa);
  fuzzy->addFuzzyInput(inputph);

  // Fuzzy Output
  FuzzyOutput *timbangan = new FuzzyOutput(1);
  FuzzySet *None = new FuzzySet(0, 0, 0, 0);
  timbangan->addFuzzySet(None);
  FuzzySet *Sedikit = new FuzzySet(0.1, 0.6, 1.5, 2);
  timbangan->addFuzzySet(Sedikit);
  FuzzySet *Biasa = new FuzzySet(2, 3, 7, 8);
  timbangan->addFuzzySet(Biasa);
  FuzzySet *Banyak = new FuzzySet(8, 9, 10, 10);
  timbangan->addFuzzySet(Banyak);
  fuzzy->addFuzzyOutput(timbangan);

  // Fuzzy Rule1
  FuzzyRuleAntecedent *rule1 = new FuzzyRuleAntecedent();
  rule1->joinSingle(Normal);
  FuzzyRuleConsequent *consequence1 = new FuzzyRuleConsequent();
  consequence1->addOutput(None);
  FuzzyRule *FuzzyRule1 = new FuzzyRule(1, rule1, consequence1);
  fuzzy->addFuzzyRule(FuzzyRule1);

  // Fuzzy Rule2
  FuzzyRuleAntecedent *rule2 = new FuzzyRuleAntecedent();
  rule2->joinSingle(MendekatiAsam);
  FuzzyRuleConsequent *consequence2 = new FuzzyRuleConsequent();
  consequence2->addOutput(Sedikit);
  FuzzyRule *FuzzyRule2 = new FuzzyRule(2, rule2, consequence2);
  fuzzy->addFuzzyRule(FuzzyRule2);

  // Fuzzy Rule3
  FuzzyRuleAntecedent *rule3 = new FuzzyRuleAntecedent();
  rule3->joinSingle(MendekatiBasa);
  FuzzyRuleConsequent *consequence3 = new FuzzyRuleConsequent();
  consequence3->addOutput(Sedikit);
  FuzzyRule *FuzzyRule3 = new FuzzyRule(3, rule3, consequence3);
  fuzzy->addFuzzyRule(FuzzyRule3);

  // Fuzzy Rule4
  FuzzyRuleAntecedent *rule4 = new FuzzyRuleAntecedent();
  rule4->joinSingle(Asam);
  FuzzyRuleConsequent *consequence4 = new FuzzyRuleConsequent();
  consequence4->addOutput(Biasa);
  FuzzyRule *FuzzyRule4 = new FuzzyRule(4, rule4, consequence4);
  fuzzy->addFuzzyRule(FuzzyRule4);

  // Fuzzy Rule5
  FuzzyRuleAntecedent *rule5 = new FuzzyRuleAntecedent();
  rule5->joinSingle(Basa);
  FuzzyRuleConsequent *consequence5 = new FuzzyRuleConsequent();
  consequence5->addOutput(Biasa);
  FuzzyRule *FuzzyRule5 = new FuzzyRule(5, rule5, consequence5);
  fuzzy->addFuzzyRule(FuzzyRule5);

  // Fuzzy Rule6
  FuzzyRuleAntecedent *rule6 = new FuzzyRuleAntecedent();
  rule6->joinSingle(SangatAsam);
  FuzzyRuleConsequent *consequence6 = new FuzzyRuleConsequent();
  consequence6->addOutput(Banyak);
  FuzzyRule *FuzzyRule6 = new FuzzyRule(6, rule6, consequence6);
  fuzzy->addFuzzyRule(FuzzyRule6);

  // Fuzzy Rule7
  FuzzyRuleAntecedent *rule7 = new FuzzyRuleAntecedent();
  rule7->joinSingle(SangatBasa);
  FuzzyRuleConsequent *consequence7 = new FuzzyRuleConsequent();
  consequence7->addOutput(Banyak);
  FuzzyRule *FuzzyRule7 = new FuzzyRule(7, rule7, consequence7);
  fuzzy->addFuzzyRule(FuzzyRule7);
}

// fungsi fuzzy
void executeFuzzy()
{
  if (phValue > 7.3)
  {
    asamOrBasa = true;
  }

  if (phValue < 6.8)
  {
    asamOrBasa = false;
  }

  // Set input for fuzzy logic
  fuzzy->setInput(1, phValue);
  fuzzy->fuzzify();
  // Defuzzify and get output
  outputValue = fuzzy->defuzzify(1);
  giveFluid = true;
  lcd.clear();
}

void givePHFluid(bool kondisi)
{
  lcd.clear();
  if (kondisi)
  {
    displaySentance(0, 0, "Asam    :");
    digitalWrite(asamPin, LOW);
  }
  else
  {
    displaySentance(0, 0, "Basa    :");
    digitalWrite(basaPin, LOW);
  }

  displayFloatValue(9, 0, outputValue);

  displaySentance(0, 1, "scale   :");
  bool condition = true;
  float scaleValue;
  scaleSensor.power_up();
  scaleSensor.tare();
  while (condition)
  {

    scaleSensor.set_scale(calibrationFactor);
    scaleValue = scaleSensor.get_units(), 4;
    if (scaleValue < 0)
    {
      displayFloatValue(9, 1, 0);
    }
    else
    {
      displayFloatValue(9, 1, scaleValue);
    }

    unsigned long currentMillis = millis();
    if (scaleValue >= outputValue)
    {
      previousMillis = currentMillis;
      if (kondisi)
      {
        emptyAsam = false;
      }
      else
      {
        emptyBasa = false;
      }

      condition = false;
    }

    if (currentMillis - previousMillis >= interval * 20)
    {
      previousMillis = currentMillis;
      if (kondisi)
      {
        emptyAsam = true;
      }
      else
      {
        emptyBasa = true;
      }
      condition = false;
    }
  }

  scaleSensor.power_down();

  if (kondisi)
  {
    digitalWrite(asamPin, HIGH);
  }
  else
  {
    digitalWrite(basaPin, HIGH);
  }
  delay(2000);
  lcd.clear();
  displaySentance(0, 1, "  tunggu 15 detik  ");

  condition = true;
  digitalWrite(aquascapePin, LOW);

  delay(interval * 15);

  digitalWrite(aquascapePin, HIGH);
  wait = true;
  giveFluid = false;
  lcd.clear();
  displaySensors();
}

// fungsi pembacaan sensor pH
void executePh()
{
  int nilaiPengukuranPh = analogRead(phPin);
  double TeganganPh = 5 / 1024.0 * nilaiPengukuranPh;
  // Po = 7.00 + ((teganganPh7 - TeganganPh) / PhStep);
  phValue = 7.00 + ((2.5 - TeganganPh) / 0.33);

  displaySentance(9, 1, "         ");

  if (phValue < 0 || phValue > 14)
  {
    displaySentance(9, 1, "error");
  }
  else
  {
    displayFloatValue(9, 1, phValue);
  }
}

void executeSuhu()
{
  suhuSensor.requestTemperatures();
  suhuValue = suhuSensor.getTempCByIndex(0);
  displaySentance(9, 0, "         ");

  if (suhuValue < -10)
  {
    displaySentance(9, 0, "error");
  }
  else
  {
    displayFloatValue(9, 0, suhuValue);
  }
}

void sendToEsp()
{
  String jsonString = "{\"suhu\":";
  jsonString += suhuValue;
  jsonString += ",\"ph\":";
  jsonString += phValue;
  jsonString += ",\"emptyAsam\":";
  jsonString += emptyAsam;
  jsonString += ",\"emptyBasa\":";
  jsonString += emptyBasa;
  jsonString += "}";
  Serial.println(jsonString);
  delay(500);
}

void receivedFromEsp()
{
  if (mySerial.available())
  {
    String dt = mySerial.readStringUntil('\n');
    delay(500);

    if (dt != data)
    {
      data = dt;
      change = true;
    }
  }
}

void isConnectedEsp()
{
  change = false;
  displaySentance(0, 2, "                    ");
  if (data == "s")
  {
    displaySentance(0, 2, "sudah terkoneksi");
  }
  else if (data.length() >= 7)
  {
    displaySentance(0, 2, data + ":90");
  }
  else if (data == "b")
  {
    displaySentance(0, 2, "belum terkoneksi");
  }
  else
  {
    return;
  }
}

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);

  // pin pump
  pinMode(asamPin, OUTPUT);
  pinMode(basaPin, OUTPUT);
  pinMode(aquascapePin, OUTPUT);
  digitalWrite(asamPin, HIGH);
  digitalWrite(basaPin, HIGH);
  digitalWrite(aquascapePin, HIGH);

  pinMode(phPin, INPUT); // ph sensor
  suhuSensor.begin();    // suhu sensor
  fuzzySetup();          // fuzzy

  scaleSensor.set_scale();
  scaleSensor.tare();

  // lcd
  lcd.init();
  lcd.backlight();
  displaySentance(0, 1, "     Kelompok5     ");
  displaySentance(0, 2, "   Electric Five   ");
  delay(3000);
  lcd.clear();
  displaySensors();
}

void loop()
{
  sendToEsp();
  receivedFromEsp();
  if (change)
  {
    isConnectedEsp();
  }
  if (!giveFluid)
  {
    executeSuhu();
    executePh();
    delay(1000);

    if (!wait)
    {
      if (phValue >= 0 && phValue <= 14)
      {
        executeFuzzy();
      }
    }
  }

  if (giveFluid)
  {
    givePHFluid(asamOrBasa);
  }

  if (wait)
  {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval * 120)
    {
      previousMillis = currentMillis;
      wait = false;
    }
  }
}
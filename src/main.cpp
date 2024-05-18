#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Fuzzy.h>
#include "PumpClass.h"
#include "TemperatureClass.h"
#include "ScaleClass.h"
#include "PHClass.h"
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <arduino-timer.h>

// pin ph
int phPin = A0;

// make object sensors
DS18B20Sensor suhu(5);
ScaleClass scaleFluid(A1, A2, 1013.60);
PumpClass basaFluidToScale(2, "Basa", false);
PumpClass asamFluidToScale(3, "Asam", false);
PumpClass fluidToAquascape(4, ".", false);

// make object for softwareSerial
SoftwareSerial mySerial(8, 9);

//

// make variable for value sensors
float suhuValue = 0;
float phValue = 0;
String ip;
bool receivedBool = true;

// make object for json
StaticJsonDocument<200> doc;

// make object lcd
LiquidCrystal_I2C lcd(0x27, 20, 4);

// fuzzy
Fuzzy *fuzzy = new Fuzzy();

// timer
auto timer = timer_create_default();

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

void displaySensorsValue(int coloum, int row, float value)
{
    lcd.setCursor(coloum, row);
    lcd.print("         "); // Clear the previous value
    lcd.setCursor(coloum, row);
    lcd.print(value);
}

void displaySentance(int coloum, int row, String sentance)
{
    lcd.setCursor(coloum, row);
    lcd.print("         "); // Clear the previous value
    lcd.setCursor(coloum, row);
    lcd.print(sentance);
}

// make function for postion display sensors
void displaySensors()
{
    // display suhu
    displaySentance(0, 0, "Suhu    :");
    displaySensorsValue(9, 0, suhuValue);

    // display Ph
    displaySentance(0, 1, "PH      :");
    displaySensorsValue(9, 1, phValue);
}

void givePhFluid(int amountPhFluid, PumpClass &fluidPump)
{
}

// Fungsi untuk menghitung nilai pH dari tegangan
float ph(float voltage)
{
    return 7 + ((2.6 - voltage) / 0.35);
}
int buf[10];

// make function for suhu sensor
bool executeSuhu(void *)
{
    suhuValue = suhu.getTemperature();
    displaySensorsValue(9, 0, suhuValue);
    return true;
}

// make function for ph sensor
bool executePh(void *)
{
    for (int i = 0; i < 10; i++)
    {
        buf[i] = analogRead(phPin);
        delay(300);
    }
    float avgValue = 0;
    for (int i = 0; i < 10; i++)
        avgValue += buf[i];
    float pHVol = (float)avgValue * 5.0 / 1023 / 10;
    phValue = (-8.0251 * pHVol + 34.091);
    displaySensorsValue(9, 1, phValue);
    return true;
}

// fungsi fuzzy
void executeFuzzy()
{

    // Set input for fuzzy logic
    fuzzy->setInput(1, phValue);
    fuzzy->fuzzify();
    // Defuzzify and get output
    float output = fuzzy->defuzzify(1);
    if (phValue < 6.8)
    {
        givePhFluid(output, basaFluidToScale);
    }
    else if (phValue > 7.3)
    {
        givePhFluid(output, asamFluidToScale);
    }
    return true;
}

// kirim data ke esp
bool sendDataToEsp(void *)
{
    doc["suhu"] = suhuValue;
    doc["ph"] = phValue;
    doc["emptyAsam"] = basaFluidToScale.empty;
    doc["emptyBasa"] = asamFluidToScale.empty;

    serializeJson(doc, Serial);
    Serial.print("\n");

    doc.clear();
    return true;
}

// dari esp ke mega
bool received(void *)
{
    if (receivedBool)
    {
        if (mySerial.available())
        {
            String msg = mySerial.readStringUntil('\n');

            if (msg == "sudah")
            {
                lcd.clear();
                displaySentance(0, 1, "      ^_____^      ");
                displaySentance(0, 2, "Terhubung telegram");
                delay(3000);
                lcd.clear();
                displaySensors();
                receivedBool = false;
                return false;
            }
            else if (msg.length() > 5 && ip.length() == 0)
            {
                msg += ":8080";
                ip = msg;
            }
        }
    }

    return true;
}

void setup()
{
    Serial.begin(9600);
    mySerial.begin(9600);
    // for initialize sensors
    suhu.begin();                  // Manggil suhu
    pinMode(phPin, INPUT);         // PH jadi input
    scaleFluid.initialize();       // Nilai timbangan
    asamFluidToScale.initialize(); // pompa asam nyala
    basaFluidToScale.initialize(); // pompa basa nyala
    fluidToAquascape.initialize(); // pompa ke aquascape nyala

    // fuzzy setup
    fuzzySetup();

    // lcd
    lcd.init();
    lcd.backlight();

    // openng
    displaySentance(0, 1, " Proyek Sistem IoT ");
    displaySentance(0, 2, "  Electric -- Five  ");
    delay(2000);
    lcd.clear();

    // menampilkan sensor
    displaySensors();

    timer.every(1000, executeSuhu);
    timer.every(1000, executePh);
    timer.every(100, sendDataToEsp);
    timer.every(100, received);
}

void loop()
{
    timer.tick();

    if (phValue < 0 || phValue > 14)
    {
        displaySentance(9, 1, "Error");
    }
    else if (phValue > 6.8 || phValue < 7.3)
    {
        displaySentance(0, 2, "kondisi :");
        displaySentance(9, 2, "Normal");
    }
    else
    {
        executeFuzzy();
    }
}

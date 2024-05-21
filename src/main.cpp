#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Fuzzy.h>
#include <ArduinoJson.h>
// #include <SoftwareSerial.h>
#include <HX711.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// pin
const uint8_t phPin = A0;
int dtPin = A1;
int sckPin = A2;
const uint8_t suhuPin = 1;
const uint8_t asamPin = 3;
const uint8_t basaPin = 5;
const uint8_t aquascapePin = 7;
const uint8_t mySerialRxPin = 8;
const uint8_t mySerialTxPin = 10;

// calibration factor for scale
float calibrationFactor = 1000;

// value sensor
float phValue;
float suhuValue;
uint8_t outputValue;
bool giveFluid = false;
bool wait = false;
bool pushScale = false;
bool pushAquascape = false;
bool asamOrBasa;
bool emptyAsam = false;
bool emptyBasa = false;
const long interval = 1000;
unsigned long previousMillis = 0;
String ip;

/*                                 make object                                                     */
HX711 scaleSensor;                      // scale
OneWire onewire(suhuPin);               // suhu
DallasTemperature suhuSensor(&onewire); // suhu
// SoftwareSerial mySerial(8, 9);          // serial komunikasi tambahan
StaticJsonDocument<200> doc;        // object json
LiquidCrystal_I2C lcd(0x27, 20, 4); // lcd
Fuzzy *fuzzy = new Fuzzy();         // objecct fuzzy

/*                               Functions                                   */

void displayFloatValue(int coloum, int row, float number)
{
    lcd.setCursor(coloum, row);
    lcd.print(number);
}

void deleteDisplay(int coloum, int row, bool full)
{
    lcd.setCursor(coloum, row);
    if (full)
    {
        lcd.print("                   ");
    }
    else
    {
        lcd.print("          ");
    }
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
    if (phValue < 0 || phValue > 14)
    {
        return;
    }

    if (phValue < 6.8)
    {
        asamOrBasa = false;
    }
    else
    {
        asamOrBasa = true;
    }

    // Set input for fuzzy logic
    fuzzy->setInput(1, phValue);
    fuzzy->fuzzify();
    // Defuzzify and get output
    outputValue = fuzzy->defuzzify(1);
    pushScale = true;
    giveFluid = true;
    deleteDisplay(0, 0, true);
    deleteDisplay(0, 1, true);
}

void fluidToScale(bool asamOrBasa)
{
    scaleSensor.power_up();

    unsigned long currentMillis = millis();

    if (asamOrBasa)
    {
        digitalWrite(asamPin, HIGH);
        displaySentance(0, 0, "Asam    :");
        displayFloatValue(9, 0, outputValue);
    }
    else
    {
        digitalWrite(basaPin, HIGH);
        displaySentance(0, 0, "Basa    :");
        displayFloatValue(9, 0, outputValue);
    }

    displaySentance(0, 1, "scale   :");
    float scaleValue;
    if (scaleSensor.is_ready())
    {
        scaleValue = scaleSensor.get_units(10);
        displayFloatValue(9, 1, scaleValue);
    }
    else
    {
        displaySentance(9, 1, "error");
    }

    if (scaleValue >= outputValue)
    {
        delay(2000);
        if (asamOrBasa)
        {
            digitalWrite(asamPin, LOW);
            emptyAsam = false;
        }
        else
        {
            digitalWrite(basaPin, LOW);
            emptyBasa = false;
        }
        deleteDisplay(0, 0, true);
        deleteDisplay(0, 0, true);
        previousMillis = currentMillis;
        outputValue = 0;
        pushScale = false;
        pushAquascape = true;
        scaleSensor.power_down();
        return;
    }

    if (currentMillis - previousMillis >= interval * 15)
    {
        if (asamOrBasa)
        {
            digitalWrite(asamPin, LOW);
            emptyAsam = true;
        }
        else
        {
            digitalWrite(basaPin, LOW);
            emptyBasa = true;
        }
        deleteDisplay(0, 0, true);
        deleteDisplay(0, 0, true);
        previousMillis = currentMillis;
        outputValue = 0;
        pushScale = false;
        pushAquascape = true;
        // scaleSensor.power_down();
    }
}

void fluidToAquascape()
{
    displaySentance(0, 1, "  tunggu 10 detik  ");
    unsigned long currentMillis = millis();
    digitalWrite(aquascapePin, HIGH);
    if (currentMillis - previousMillis >= interval * 10)
    {
        deleteDisplay(0, 1, true);
        digitalWrite(aquascapePin, LOW);
        previousMillis = currentMillis;
        giveFluid = false;
        pushAquascape = false;
        wait = true;
        displaySensors();
    }
}

// Fungsi untuk menghitung nilai pH dari tegangan
float ph(float voltage)
{
    return 7 + ((2.6 - voltage) / 0.35);
}
int buf[10];

// make function for ph sensor
void executePh()
{
    for (int i = 0; i < 10; i++)
    {
        buf[i] = analogRead(phPin);
        delay(300);
    }
    float avgValue = 0;
    for (int i = 0; i < 10; i++)
        avgValue += buf[i];
    phValue = (float)avgValue * 5.0 / 1023 / 10;

    if (phValue < 0 && phValue > 14)
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
    deleteDisplay(9, 0, false);

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
    if (Serial.available() == 0)
    {
        doc["suhu"] = suhuValue;
        doc["ph"] = phValue;
        doc["emptyAsam"] = emptyAsam;
        doc["emptyBasa"] = emptyBasa;

        String output;
        serializeJson(doc, output);
        Serial.println(output);

        doc.clear();
    }
}

// void receivedFromEsp()
// {
//     if (mySerial.available())
//     {
//         String data = mySerial.readStringUntil('\n');

//         ip = data;
//     }
// }

void setup()
{
    Serial.begin(9600);
    // mySerial.begin(9600);

    // pin pump
    pinMode(asamPin, OUTPUT);
    pinMode(basaPin, OUTPUT);
    pinMode(aquascapePin, OUTPUT);

    pinMode(phPin, INPUT); // ph sensor
    suhuSensor.begin();    // suhu sensor
    fuzzySetup();          // fuzzy

    lcd.init();
    lcd.backlight();
    // lcd
    scaleSensor.begin(dtPin, sckPin, calibrationFactor);

    displaySentance(0, 1, "     Kelompok5     ");
    displaySentance(0, 2, "   Electric Five   ");
    delay(3000);
    deleteDisplay(0, 1, true);
    deleteDisplay(0, 2, true);
    displaySensors();
}

void loop()
{
    if (!giveFluid)
    {
        executeSuhu();
        executePh();
        sendToEsp();
        delay(1000);
        if (!wait)
        {
            executeFuzzy();
        }
    }
    else
    {
        if (pushScale)
        {
            fluidToScale(asamOrBasa);
        }

        if (pushAquascape && !pushScale)
        {
            fluidToAquascape();
        }
    }

    if (wait)
    {
        unsigned long currentMillis = millis();

        if (currentMillis - previousMillis >= interval * 60)
        {
            previousMillis = currentMillis;
            wait = false;
        }
    }
}

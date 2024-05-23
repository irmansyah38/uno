#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Fuzzy.h>
#include <SoftwareSerial.h>
#include <HX711.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// pin
const int phPin = A0;
#define dtPin A2
#define sckPin A4
const uint8_t suhuPin = 4;
const uint8_t asamPin = 3;
const uint8_t basaPin = 5;
const uint8_t aquascapePin = 7;
const uint8_t mySerialRxPin = 8;
const uint8_t mySerialTxPin = 10;

// calibration factor for scale
float calibrationFactor = 861.70;

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
String received;
int scaleValue;

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
    deleteDisplay(9, 1, false);
    scaleSensor.set_scale(calibrationFactor);
    scaleValue = scaleSensor.get_units(10), 4;
    displayFloatValue(9, 1, scaleValue);

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
        scaleValue = 0;
        previousMillis = currentMillis;
        outputValue = 0;
        pushScale = false;
        pushAquascape = true;
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
        scaleValue = 0;
        previousMillis = currentMillis;
        outputValue = 0;
        pushScale = false;
        pushAquascape = true;
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
    float phValue = (3.5 * voltage) + 0.5;
    return phValue;
}

// fungsi pembacaan sensor pH
void executePh()
{
    int phRaw = analogRead(phPin);
    float phVoltage = phRaw * (5.0 / 1023.0);
    phValue = ph(phVoltage);
    deleteDisplay(9, 1, false);

    if (phValue < 0 || phValue > 14)
    {
        displaySentance(9, 1, "error!");
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
}

void receivedFromEsp()
{

    if (mySerial.available())
    {
        String data = mySerial.readStringUntil('\n');
        received = data;
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
    deleteDisplay(0, 1, true);
    deleteDisplay(0, 2, true);
    displaySensors();
}

void displayIP()
{
    deleteDisplay(0, 3, true);
    deleteDisplay(0, 2, true);
    if (received == "belum")
    {
        displaySentance(0, 3, "tidak konek wifi");
    }
    else if (received == "sudah")
    {
        displaySentance(0, 3, "Sudah terhubung");
    }
    else if (received.length() > 6)
    {
        displaySentance(0, 2, "Masuk link di bawah");
        received += ":8080";
        displaySentance(0, 3, received);
    }
}

void loop()
{
    sendToEsp();
    receivedFromEsp();
    displayIP();
    if (!giveFluid)
    {
        executeSuhu();
        executePh();
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

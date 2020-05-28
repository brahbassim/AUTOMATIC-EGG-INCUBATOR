/* *****************************
 * ********* Libraries *********
 * *****************************/
#include <Arduino.h>
#include <Wire.h>
#include <dht.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

/* *****************************
 * ***** Incubating params *****
 * *****************************/
float inc_hum_point = 0.00;
int inc_hum_delay = 0;
float inc_temp_point = 0.00;
float inc_temp_delay = 0.00;


/* *****************************
 * ****** Haching params *******
 * *****************************/
float hach_hum_point = 0.00;
int hach_hum_delay = 0;
float hach_temp_point = 0.00;
float hach_temp_delay = 0.00;

/* *****************************
 * ****** Global params ********
 * *****************************/
bool hum_reached = false;
bool temp_reached = false;
float current_temp_one = 0.00;
float current_temp_two = 0.00;
float current_temp_tree = 0.00;
float current_temp = 0.00;
float current_hum = 0.00;
int current_mode = 0;
int current_kind = 0;
int potRawValue, oldPotRawValue;
float max_temp = 39.00;
float min_temp = 35.00;
float max_hum = 90.00;
float min_hum = 30.00;
String current_bird = "";
unsigned long previous_oxygenation_duration_time = 0;
unsigned long previous_oxygenation_duration_time_delay = (unsigned long) 1000 * 60 * 60 * 6;
unsigned long previous_oxygenation_cycle_time = 0;
unsigned long previous_oxygenation_cycle_time_delay = (unsigned long) 1000 * 10;

/* *****************************
 * ********* Triggers **********
 * *****************************/
int humidify = 8;
int humidify_fan = 1;
int humidify_on = 3;
int heating = 7;
int heating_fan = 6;
int heating_on = 2;
int cooling = 5;
int alarm = 4;  
//int oxygenation = 11;

/* *****************************
 * ********* Sensors ***********
 * *****************************/
int mode = 11;
int dht22 = 12;
int ds18b20 = 13;
int kind = A3;

/* *****************************
 * ********** Icons ************
 * *****************************/
byte temp_icon[8] = {B00100,B01010,B01010,B01110,B01110,B11111,B11111,B01110};
byte hum_icon[8] = {B00100,B00100,B01010,B01010,B10001,B10001,B10001,B01110};
byte degre_icon [8] = {B00001100,B00010010,B00010010,B00001100,B00000000,B00000000,B00000000,B00000000};

/* *****************************
 * ****** Scope functions ******
 * *****************************/
void handleCooling();
void handleDehydrating();
void handleHeating();
void handleHumidity();
void readKind();
void handleKind();
void setPin(int pin, int state);
bool getPin(int pin);
void initTemperature();
void initHumidity();
bool inRange(int val, int min, int max);
void printResult();
void fresh();
void check_alarm();

/* *****************************
 * ********** Objects **********
 * *****************************/
LiquidCrystal_I2C lcd(0x27,20,4);
OneWire oneWire(ds18b20);
DallasTemperature sensors(&oneWire);
DeviceAddress sensorDeviceAddress;
dht DHT;

/* *****************************
 * ******* Setup program *******
 * *****************************/
void setup() {
    // Set pins mode
    pinMode(mode, INPUT);
    pinMode(kind, INPUT);
    pinMode(heating, OUTPUT);
    pinMode(humidify, OUTPUT);
    pinMode(heating_fan, OUTPUT);
    pinMode(humidify_fan, OUTPUT);
    pinMode(cooling, OUTPUT);
    pinMode(heating_on,OUTPUT);
    pinMode(humidify_on,OUTPUT);
    pinMode(alarm,OUTPUT);
    //pinMode(oxygenation,OUTPUT);
    
    // Initialize the lcd
    lcd.init();

    // Initialize the temp sensor
    sensors.begin();
    sensors.getAddress(sensorDeviceAddress, 0);
    sensors.setResolution(sensorDeviceAddress, 12);

    // Turn on backlight
    lcd.backlight();

    // Loading
    lcd.setCursor(0,0);
    lcd.print(F("---INITIALISATION---"));
    for(int i = 0; i< 20; i++){
        lcd.setCursor(i,1);
        lcd.print(".");
        delay(200);
    }

    lcd.setCursor(0,2);
    lcd.print(F("  DEVARTS CREATIVE"));
    lcd.setCursor(0,3);
    lcd.print(F("    V1.0 / 2019     "));  
    delay(5000);
    lcd.clear();

    // Create characters
    lcd.createChar(1, temp_icon);
    lcd.createChar(2, hum_icon);
    lcd.createChar(3, degre_icon);

    fresh();
}

/* *****************************
 * ******* Main program ********
 * *****************************/
void loop() {
    readKind();

    // Initialise temperature sensor
    initTemperature();

    // Initialise humidity sensor
    initHumidity();

    // Call alarm if error occures
    check_alarm();

    // Print all params on screen
    printResult();

    // Handle the heating
    handleHeating();

    // Handle the humidity
    handleHumidity();

    // Handle the cooling
    handleCooling();

    // Handle dehydrating
    handleDehydrating();
}

/* *****************************
 * ***** Set pin state *****
 * *****************************/
void setPin(int pin, int state){
    digitalWrite(pin,state);
}

/* *****************************
 * *** Get pin state ***
 * *****************************/
bool getPin(int pin){
    return digitalRead(pin);
}

/* *****************************
 * ***** Init temperature ******
 * *****************************/
void initTemperature(){
    sensors.requestTemperatures();
    current_temp = sensors.getTempCByIndex(0);
}

/* *****************************
 * ******* Init humidity *******
 * *****************************/
void initHumidity(){
    DHT.read22(dht22);
    current_hum = DHT.humidity;
}

/* *****************************
 * ******** Check Alarm ********
 * *****************************/
void check_alarm(){
    if(current_temp >= max_temp || current_temp <= min_temp || current_hum >= max_hum || current_hum <= min_hum){
        digitalWrite(alarm, HIGH);
    }else{
        digitalWrite(alarm, LOW);
    }
    
}

/* *****************************
 * ******* Print result ********
 * *****************************/
void printResult(){
    // Write current bird on lcd screen 4 line
    lcd.setCursor(0,0);
    lcd.print(F("ESPECE: "));
    lcd.print(current_bird);
    lcd.setCursor(17,0);
    lcd.print(F("   "));
    lcd.setCursor(17,0);
    lcd.print(current_kind);

    // Write current temp on lcd screen 1 line
    lcd.setCursor(0,1);
    lcd.write(1);
    lcd.setCursor(2,1);
    lcd.print(current_temp);
    lcd.print(F(" "));
    lcd.write(3);
    lcd.print(F("C"));

    // Write current hum on lcd screen 1 line
    lcd.setCursor(11,1);
    lcd.write(2);
    lcd.setCursor(13,1);
    lcd.print(current_hum);
    lcd.print(F(" "));
    lcd.print(F("%"));

    // Write current mode on lcd screen 3 line
    if(current_mode == 0){
        lcd.setCursor(0,2);
        lcd.print(F("MODE: INCUBATION"));
    }else{
        lcd.setCursor (0,2);
        lcd.print(F("MODE: ECLOSION"));
    }

    // Write point temp and hum on lcd screen 2 line
    if (current_mode == 0){
        lcd.setCursor(0,3);
        lcd.write(1);
        lcd.setCursor(2,3);
        lcd.print(inc_temp_point);
        lcd.print(F(" "));
        lcd.write(3);
        lcd.print(F("C"));

        lcd.setCursor(11,3);
        lcd.write(2);
        lcd.setCursor(13,3);
        lcd.print(inc_hum_point);
        lcd.print(F(" "));
        lcd.print(F("%"));
    }else{
        lcd.setCursor(0,3);
        lcd.write(1);
        lcd.setCursor(2,3);
        lcd.print(hach_temp_point);
        lcd.print(F(" "));
        lcd.write(3);
        lcd.print(F("C"));

        lcd.setCursor(11,3);
        lcd.write(2);
        lcd.setCursor(13,3);
        lcd.print(hach_hum_point);
        lcd.print(F(" "));
        lcd.print(F("%"));
    }
}

/* *****************************
 * **** Handle cooling *****
 * *****************************/
void handleCooling(){
    if(current_temp >= 38.10){
        setPin(cooling,1);
    }else{
        setPin(cooling,0);
    }  
}

/* *****************************
 * **** Handle dehydrating *****
 * *****************************/
void handleDehydrating(){
    // If in incubating mode
    if(current_mode == 0){
        if(current_hum >= inc_hum_point + 5){
            setPin(cooling,1);
        }else{
            setPin(cooling,0);
        } 
    }

    // If in haching mode
    if(current_mode == 1){
        if(current_hum >= hach_hum_point + 10){
            setPin(cooling,1);
        }else{
            setPin(cooling,0);
        }  
    }
}

/* *****************************
 * **** Handle heating *****
 * *****************************/
void handleHeating(){
    // If in incubating mode
    if(current_mode == 0){
        if(current_temp != -127){
            if(current_temp >= inc_temp_point){
            setPin(heating_on,0);
            setPin(heating,0);
            setPin(heating_fan,0);
            temp_reached = true;
        }
        if(current_temp < inc_temp_point){
            if(temp_reached){
                if(current_temp <= inc_temp_point - inc_temp_delay){
                    setPin(heating_on,1);
                    setPin(heating,1);
                    setPin(heating_fan,1);
                    temp_reached = false;
                }
            }else{
                setPin(heating_on,1);
                setPin(heating,1);
                setPin(heating_fan,1);
            }
        }
        }  
    }

    // If in haching mode
    if(current_mode == 1){
        if(current_temp != -127){
            if(current_temp >= hach_temp_point){
                setPin(heating_on,0);
                setPin(heating,0);
                setPin(heating_fan,0);
                temp_reached = true;
            }
            if(current_temp < hach_temp_point){
                if(temp_reached){
                    if(current_temp <= hach_temp_point - hach_temp_delay){
                        setPin(heating_on,1);
                        setPin(heating,1);
                        setPin(heating_fan,1);
                        temp_reached = false;
                    }
                }else{
                    setPin(heating_on,1);
                    setPin(heating,1);
                    setPin(heating_fan,1);
                }
            }
        }  
    } 
}

/* *****************************
 * ****** Handle humidity ******
 * *****************************/
void handleHumidity(){ 
    // If in incubation mode
    if(current_mode == 0){
        if(current_hum >= inc_hum_point){
            setPin(humidify_on,0);
            setPin(humidify_fan,0);
            setPin(humidify,0);
            hum_reached = true;
        }
        if(current_hum < inc_hum_point){
            if(hum_reached){
                if(current_hum <= inc_hum_point - inc_hum_delay){
                    setPin(humidify_on,1);
                    setPin(humidify_fan,1);
                    setPin(humidify,1);
                    hum_reached = false;
                }
            }else{
                setPin(humidify_on,1);
                setPin(humidify_fan,1);
                setPin(humidify,1);
            }
        }  
    }

    // If in haching mode
    if(current_mode == 1){
        if(current_hum >= hach_hum_point){
            setPin(humidify_on,0);
            setPin(humidify_fan,0);
            setPin(humidify,0);
            hum_reached = true;
        }
        if(current_hum < hach_hum_point){
            if(hum_reached){
                if(current_hum <= hach_hum_point - hach_hum_delay){
                    setPin(humidify_on,1);
                    setPin(humidify_fan,1);
                    setPin(humidify,1);
                    hum_reached = false;
                }
            }else{
                setPin(humidify_on,1);
                setPin(humidify_fan,1);
                setPin(humidify,1);
            }
        }  
    }
}

/* *****************************
 * **** Handle kind params *****
 * *****************************/
bool inRange(int val, int min, int max){
  return ((min < val) && (val < max));
}

/* *****************************
 * ***** Handle bird kind ******
 * *****************************/
void handleKind(){
    // POULLE
    if (inRange(current_kind,0,10)) {
        current_bird = "POULE";
        inc_hum_point = 45.00;
        inc_hum_delay = 5;
        inc_temp_point = 37.70;
        inc_temp_delay = 0.15;
        hach_hum_point = 75.00;
        hach_hum_delay = 5;
        hach_temp_point = 37.60;
        hach_temp_delay = 0.15;
    }

    // PINTADE
    if (inRange(current_kind,10,20)) {
        current_bird = "PINTADE";
        inc_hum_point = 50.00;
        inc_hum_delay = 5;
        inc_temp_point = 37.70;
        inc_temp_delay = 0.15;
        hach_hum_point = 80.00;
        hach_hum_delay = 5;
        hach_temp_point = 37.60;
        hach_temp_delay = 0.15;
    }

    // DINDE
    if (inRange(current_kind,20,30)) {
        current_bird = "DINDE";
        inc_hum_point = 50.00;
        inc_hum_delay = 5;
        inc_temp_point = 37.70;
        inc_temp_delay = 0.15;
        hach_hum_point = 75.00;
        hach_hum_delay = 5;
        hach_temp_point = 37.70;
        hach_temp_delay = 0.15;
    }

    // PAONNE
    if (inRange(current_kind,30,40)) {
        current_bird = "PAONNE";
        inc_hum_point = 50.00;
        inc_hum_delay = 5;
        inc_temp_point = 37.60;
        inc_temp_delay = 0.10;
        hach_hum_point = 75.00;
        hach_hum_delay = 5;
        hach_temp_point = 37.60;
        hach_temp_delay = 0.10;
    }

    // CANE
    if (inRange(current_kind,40,50)) {
        current_bird = "CANE";
        inc_hum_point = 55.00;
        inc_hum_delay = 5;
        inc_temp_point = 37.60;
        inc_temp_delay = 0.10;
        hach_hum_point = 85.00;
        hach_hum_delay = 10;
        hach_temp_point = 37.60;
        hach_temp_delay = 0.10;
    }

    // OIE
    if (inRange(current_kind,50,60)) {
        current_bird = "OIE";
        inc_hum_point = 55.00;
        inc_hum_delay = 5;
        inc_temp_point = 37.60;
        inc_temp_delay = 0.10;
        hach_hum_point = 85.00;
        hach_hum_delay = 10;
        hach_temp_point = 37.60;
        hach_temp_delay = 0.10;
    }

     // CAILLE
    if (inRange(current_kind,60,70)) {
        current_bird = "CAILLE";
        inc_hum_point = 50.00;
        inc_hum_delay = 5;
        inc_temp_point = 37.90;
        inc_temp_delay = 0.20;
        hach_hum_point = 75.00;
        hach_hum_delay = 5;
        hach_temp_point = 37.80;
        hach_temp_delay = 0.15;
    }

     // PERDRIX
    if (inRange(current_kind,70,80)) {
        current_bird = "PERDRIX";
        inc_hum_point = 50.00;
        inc_hum_delay = 5;
        inc_temp_point = 37.80;
        inc_temp_delay = 0.20;
        hach_hum_point = 75.00;
        hach_hum_delay = 5;
        hach_temp_point = 37.60;
        hach_temp_delay = 0.10;
    }

    // FAISANE
    if (inRange(current_kind,80,90)) {
        current_bird = "FAISANE";
        inc_hum_point = 50.00;
        inc_hum_delay = 5;
        inc_temp_point = 37.80;
        inc_temp_delay = 0.20;
        hach_hum_point = 70.00;
        hach_hum_delay = 5;
        hach_temp_point = 37.60;
        hach_temp_delay = 0.20;
    }

    // POULE
    if (inRange(current_kind,80,90)) {
        current_bird = "POULE";
        inc_hum_point = 50.00;
        inc_hum_delay = 5;
        inc_temp_point = 37.70;
        inc_temp_delay = 0.15;
        hach_hum_point = 75.00;
        hach_hum_delay = 5;
        hach_temp_point = 37.60;
        hach_temp_delay = 0.15;
    }

    // POULE
    if (inRange(current_kind,90,100)) {
        current_bird = "POULE";
        inc_hum_point = 50.00;
        inc_hum_delay = 5;
        inc_temp_point = 37.70;
        inc_temp_delay = 0.15;
        hach_hum_point = 75.00;
        hach_hum_delay = 5;
        hach_temp_point = 37.60;
        hach_temp_delay = 0.15;
    }
}

/* *****************************
 * ***** Fresh all params ******
 * *****************************/
void fresh(){
    readKind();
    // Load incubating and haching params
    handleKind();
    if(inc_hum_point == 0.00){
        current_bird = "POULE";
        inc_hum_point = 50.00;
        inc_hum_delay = 5;
        inc_temp_point = 37.70;
        inc_temp_delay = 0.15;
        hach_hum_point = 75.00;
        hach_hum_delay = 5;
        hach_temp_point = 37.60;
        hach_temp_delay = 0.15;
    }

    // Get current mode
    if(digitalRead(mode) == LOW){
        current_mode = 0;
    }else{
        current_mode = 1;
    }
}

/* *****************************
 * ********* Read kind *********
 * *****************************/
void readKind(){
    potRawValue = analogRead(kind);
    potRawValue = analogRead(kind);
    potRawValue = constrain(potRawValue, 8, 1015);
    if (potRawValue < (oldPotRawValue - 4) || potRawValue > (oldPotRawValue + 4)) {
        oldPotRawValue = potRawValue;
        current_kind = map(oldPotRawValue, 8, 1008, 0, 100);
    }
}
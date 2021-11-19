#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C  //talvez seja 0x3C
#define SEGUNDOS_COLETA 30000 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);

//config termistor
float R1 = 10000;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07; //valores constantes para calculo

//config variaveis
float ph, temperatura;
int turbidez;
const int TAMANHO = 100;
unsigned long tempo = 0;

//config pin
int turbidezPin = A0; // Potenciômetro simulado sensor de turbidez
int phPin = A1; // Potenciômetro simulado sensor de pH
int temperaturaPin = A2; // Termistor
int ledPin = 11; // LED

void setup() {
    //configura pinos de INPUT e OUTPUT do arduinos
    pinMode(turbidezPin, INPUT);
    pinMode(phPin, INPUT);
    pinMode(temperaturaPin, INPUT);
    pinMode(ledPin,OUTPUT);
    
    digitalWrite(ledPin, LOW);    
    
    //inicializa comunicação serial
    Serial.begin(9600);

    //config do display
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.clearDisplay();
    display.setTextSize(1); 
    display.setTextColor(SSD1306_WHITE);
}

void loop() {
    secondsDelay(SEGUNDOS_COLETA);
    ledAlarm();  
}

//função que faz leitura da temperatura e retorna o valor em graus celcius
float readTemp(int ThermistorPin){
  int vo = analogRead(ThermistorPin);
  float R2 = R1 * (1023.0 / (float)vo - 1.0); //calculo R2, demonstração no arquivo pdf da aula
  float logR2 = log(R2);
  float T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));// Equação de Steinhart–Hart 
  float Tc = T - 273.15; //temp em Graus celcius
  return Tc;
}

//função que faz a leitura do potenciômetro e retorna o valor dentro da escala de pH
float readPh(int phPin) {
    int phRead = analogRead(phPin);
    //float phpwm = map(phRead, 0,1023,0,14);  
    float phpwm = (float)phRead *14.0 / 1023.0; 
    return phpwm;
}

//função que faz a leitura do potenciômetro e retorna o valor dentro da escala de turbidez
int readTurbidez(int turbidezPin) {
    int turbidezRead = analogRead(turbidezPin);
    int turbidezpwm = map(turbidezRead, 0,1023,0,1500);  
    return turbidezpwm;
}

void printDisplay(float temp,float ph, int turbidez ){
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Temperatura: "+ (String)temp+(char)247+"C");
    display.print("pH: ");
    display.println(ph,1);
    display.print("Turbidez: "+ (String)turbidez+ " NTu");
    display.display();
}

void sendJson(float temp,float ph, int turbidez ){
    StaticJsonDocument<TAMANHO> json; //cria o objeto Json
    
    //formato de leitura no node-red
    json["temperatura"] = temp;
    json["ph"] = ph;
    json["turbidez"] = turbidez;

    serializeJson(json, Serial);
    Serial.println();
}

void ledAlarm(){
    if (Serial.available() > 0) {
        char dado = Serial.read();
        dado == '1'? digitalWrite(ledPin, HIGH) : digitalWrite(ledPin, LOW);
    }
}

//Delay com millis para coletar a cada 30 segundos as informações dos sensores e enviar via MQTT
void secondsDelay(int seconds){
    if (millis() - tempo >= seconds){
        tempo = millis(); 
        
        temperatura = readTemp(temperaturaPin);
        ph = readPh(phPin);
        turbidez = readTurbidez(turbidezPin);
        printDisplay(temperatura, ph, turbidez);
        sendJson(temperatura, ph, turbidez);
    }

}
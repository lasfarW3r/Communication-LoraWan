#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// LoRa pins
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
#define BAND 866E6

// OLED pins
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// Animation settings
struct Goutte {
  int x;
  int y;
  int vitesse;
};
const int NB_GOUTTES = 30;
Goutte gouttes[NB_GOUTTES];

// LoRa data
String LoRaData;
bool animationActive = false;

void initialiserGouttes() {
  for (int i = 0; i < NB_GOUTTES; i++) {
    gouttes[i].x = random(0, SCREEN_WIDTH);
    gouttes[i].y = random(-SCREEN_HEIGHT, 0);
    gouttes[i].vitesse = random(1, 4);
  }
}

void dessinerTexte() {
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  const char* texte = "YW3R";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(texte, 0, 0, &x1, &y1, &w, &h);
  int x = (SCREEN_WIDTH - w) / 2;
  int y = (SCREEN_HEIGHT - h) / 2;
  display.setCursor(x, y);
  display.print(texte);
}

void dessinerGouttes() {
  for (int i = 0; i < NB_GOUTTES; i++) {
    display.drawPixel(gouttes[i].x, gouttes[i].y, SSD1306_WHITE);
    gouttes[i].y += gouttes[i].vitesse;
    if (gouttes[i].y >= SCREEN_HEIGHT) {
      gouttes[i].x = random(0, SCREEN_WIDTH);
      gouttes[i].y = random(-SCREEN_HEIGHT, 0);
      gouttes[i].vitesse = random(1, 4);
    }
  }
}

void playAnimation() {
  initialiserGouttes();
  unsigned long start = millis();
  while (millis() - start < 5000) {  // Play animation for 5 seconds
    display.clearDisplay();
    dessinerTexte();
    dessinerGouttes();
    display.display();
    delay(30);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("LORA RECEIVER");
  display.display();

  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("LoRa Initialized OK!");
  display.setCursor(0, 10);
  display.println("LoRa Initialized OK!");
  display.display();
}

void loop() {
  if (animationActive) return;  // Don't interrupt animation

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.println("\rReceived packet:");

    while (LoRa.available()) {
      LoRaData = LoRa.readString();
      Serial.println(LoRaData);
    }

    display.clearDisplay();
    display.setCursor(30, 0);
    display.print("Yasir Lasfar");
    display.setCursor(0, 20);
    display.print("Received packet:");
    display.setCursor(0, 30);
    display.print(LoRaData);
    display.display();

    // Extract counter from message
    if (LoRaData.startsWith("YW3R ")) {
      int receivedCounter = LoRaData.substring(5).toInt();

      if (receivedCounter == 10) {
        animationActive = true;
        playAnimation();
        delay(30000);  // Additional delay after animation
        animationActive = false;
      }
    }
  }
}

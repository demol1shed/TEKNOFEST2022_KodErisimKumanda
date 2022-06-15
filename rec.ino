#include <ekran.h>
#include <nRF24.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <joystick.h>
#include <string.h>

#define CE 48
#define CSN 49

#define JOY_X A0
#define JOY_Y 0
#define JOY_X2 A1
#define SWT 17
#define SWT2 19

nRF24 radyoModulu;
RF24 radyo(CE, CSN);

U8GLIB_SSD1306_128X64 ekran;
U8GLIB_SSD1306_128X64 ekran1;
U8GLIB_SSD1306_128X64 ekran2;
OledEkran ekranModulu(20, 20);

Joystick joystick(JOY_X, JOY_X2);

int joystickMaplenmisDegerler[4];
bool roleSwitchDurumu;
bool hareketSwitchDurumu;
bool veriDurumu;

void setup(){
  Serial.begin(9600);
  radyo = radyoModulu.nRF24VericiKurulum(radyo, RF24_PA_HIGH, 9600, RF24_250KBPS);
  ekranModulu.PinKurulum();
  ekran.setRot180();
  ekran1.setRot180();
  CizimLoop();
}

void loop(){
  unsigned long x = millis();
  bool tempSwitchDegeri = roleSwitchDurumu; 
  bool tempSwitchDegeri2 = hareketSwitchDurumu; 
  _JoystickOku();
  if(DebugKontrol()){ 
    DebugPrint();
  }else{
    Gonder();
  }
  // Sadece switchleri kontrol ediyor
  // Neden hep true????????????
  if(EkranDegerKontrol(tempSwitchDegeri, roleSwitchDurumu) || EkranDegerKontrol(tempSwitchDegeri2, hareketSwitchDurumu)){ 
    CizimLoop(); 
  }
  Serial.print(millis() - x);
  Serial.println(" ms, ana loop suresi");
}

// Onceki degeri parametre olarak alıp karşılaştırıp returnle
bool EkranDegerKontrol(bool tempdeger, bool deger){
  if(tempdeger != deger){
    return true;
  }
  return false;
}

bool DebugKontrol(){
  if(digitalRead(SWT)){
    return 1;
  }else{
    return 0;
  }
}

void DebugPrint(){
  for(int i = 0; i < sizeof(joystickMaplenmisDegerler) / sizeof(joystickMaplenmisDegerler[0]); i++){
    Serial.print(joystickMaplenmisDegerler[i]);
    Serial.print('\t');
  }
  Serial.println(" ");
}

/**
 * @deprecated debug kontrol switchi, fonksiyon ismi değiştirilmeli
 * 
 * @return const char* 
 */
const char* RoleDurumu(){
  switch (roleSwitchDurumu){
  case 0:
    return " ";
  
  case 1:
    return "Debug modu aktif";
  }
}

const char* HareketDurumu(){
  switch (hareketSwitchDurumu){
  case 0:
    return "Hareket Modu";
  
  case 1:
    return "Kriko Modu";
  }
}

/**
 * @brief joystick vektorleri ters!!
 * 
 */
void _JoystickOku(){
  joystick.JoystickOku(1);
  joystickMaplenmisDegerler[1] = map(joystick.veriVektorleri[0],0 , 1023, -255, 0) * -1;
  joystickMaplenmisDegerler[0] = map(joystick.veriVektorleri[1],0 , 1023, 0, 255);
  // kontrol et
  roleSwitchDurumu = digitalRead(SWT); 
  hareketSwitchDurumu = digitalRead(SWT2);
  joystickMaplenmisDegerler[2] = roleSwitchDurumu;
  joystickMaplenmisDegerler[3] = hareketSwitchDurumu;
}

void Gonder(){
  unsigned long _x = millis();  
  radyoModulu.nRF24VeriGonder(radyo, joystickMaplenmisDegerler, 4);
  Serial.print(millis() - _x);
  Serial.println(" ms, nrf gonderim suresi");
}

#pragma region EkranFonksiyonları
void EkranSec(int ekran=3){
  switch (ekran){
  case 0:
    ekranModulu.EkranKurulum(LOW, LOW, LOW);
    return;
  
  case 1:
    ekranModulu.EkranKurulum(HIGH, LOW, LOW);
    return;

  case 2:
    ekranModulu.EkranKurulum(LOW, HIGH, LOW);
    return;
  }
}

/**
 * @note 3(+ 1) maksimum ekrana yazılabilecek mesaj sayısı.
 * 
 * @param mesajlar 
 */
void EkranaCiz(const char* mesajlar[]){
  ekran.setFont(u8g_font_7x13);
  for(int i = 1; i < 3 + 1; i++){
    ekran.drawStr(10, 20 * i, mesajlar[i - 1]);
  }
}
void EkranaCiz1(const char* mesajlar[]){
  ekran1.setFont(u8g_font_7x13);       
  for(int i = 1; i < 3 + 1; i++){
    ekran1.drawStr(20, 20 * i, mesajlar[i - 1]);
  }
}
void EkranaCiz2(const char* mesajlar[]){
  ekran2.setFont(u8g_font_7x13);
  for(int i = 1; i < 3 + 1; i++){
    ekran2.drawStr(20, 20 * i, mesajlar[i - 1]);
  }
}

inline void StrHazirla(char (&buffer)[10 + sizeof(char)], int deger){
  sprintf(buffer, "%d", deger);
}

void CizimLoop(){
  unsigned long _x = millis();  
  EkranSec(0);
  ekran.firstPage();
  do {
    const char* _arr[3] = {" ", RoleDurumu(), " "};
    EkranaCiz(_arr);
  } while(ekran.nextPage());

  EkranSec(1);
  ekran1.firstPage();
  do{
    const char* _arr[3] = {" ", HareketDurumu(), " "};
    EkranaCiz1(_arr);
  } while(ekran1.nextPage());

  EkranSec(2);
  ekran2.firstPage();
  do{
    /*char numCharBuf[10 + sizeof(char)];
    char numCharBuf2[10 + sizeof(char)];

    unsigned long cur = micros();
    StrHazirla(numCharBuf, joystickMaplenmisDegerler[0]);
    StrHazirla(numCharBuf2, joystickMaplenmisDegerler[1]);
    strcat(numCharBuf, ", ");
    strcat(numCharBuf, numCharBuf2);
    unsigned long time = micros() - cur;
    
    char timeBuf[10 + sizeof(char)];
    StrHazirla(timeBuf, time);*/
    /*const char* _arr[3] = {" ", " ", " "};

    EkranaCiz2(_arr);*/
  } while(ekran2.nextPage());
  Serial.print(millis() - _x);
  Serial.println(" ms, ekran fonksiyon suresi");  
}
#pragma endregion
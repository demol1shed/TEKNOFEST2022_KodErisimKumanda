#include <ekran.h>
#include <nRF24.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <joystick.h>
#include <string.h>

// Nrf24 pinler
#define CE 48
#define CSN 49

// Joystick pinleri
#define JOY_X A0  // sol joystick
#define JOY_Y 0
#define JOY_X2 A1 // sağ joystick
#define SWT 17    // switch 1
#define SWT2 19   // switch 2

// nRF24 modülü
nRF24 radyoModulu;
// Özel radyo modülümüz
RF24 radyo(CE, CSN);

#pragma region ekranlar 
U8GLIB_SSD1306_128X64 ekran;
U8GLIB_SSD1306_128X64 ekran1;
U8GLIB_SSD1306_128X64 ekran2;
#pragma endregion
// Ekranlar için özel sınıf
OledEkran ekranModulu(20, 20);

// Özel joystick modülü
Joystick joystick(JOY_X, JOY_X2);

// Radyodan gelen değerlerin sıralandığı dizi
int joystickMaplenmisDegerler[4];
// Ekrana yansıtılacak switchin durumu.
/**
 * @deprecated ismi değiştirilecek röle kullanılmıyor
 * @brief kumandanın debug modunu kontrol ediyor
 */
bool roleSwitchDurumu;
// Kumandanın hareket ve kriko modunda olduğunu kararlaştıran değişken
bool hareketSwitchDurumu;

void setup(){
  Serial.begin(9600);
  // nRF24'ü hazır hale getirir.
  radyo = radyoModulu.nRF24VericiKurulum(radyo, RF24_PA_HIGH, 9600, RF24_250KBPS);
  // Ekran pinlerini hazır hale getirir.
  ekranModulu.PinKurulum();
  // Ekran 0 ve 1'i 180 derece döndürür.
  ekran.setRot180();
  ekran1.setRot180();
  // Ekranları hazırladıktan sonra ekrana başlangıç için hazır bilgleri yaz
  CizimLoop();
}

void loop(){
  unsigned long x = millis();
  // Switchlerin eski değerlerini değişkene kaydet
  bool tempSwitchDegeri = roleSwitchDurumu; 
  bool tempSwitchDegeri2 = hareketSwitchDurumu; 
  // Joystick değerlerini oku, yeni switch değerlerini al.
  _JoystickOku();
  if(DebugKontrol()){ 
    DebugPrint();
  }else{
    Gonder();
  }
  // Kaydedilen değerleri şimdiki değerlerle karşılaştır.
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

/**
 * @brief Kumandanın debug modunda olup olmadığını karşılaştırır.
 * 
 * @return true 
 * @return false 
 */
bool DebugKontrol(){
  if(digitalRead(SWT)){
    return 1;
  }else{
    return 0;
  }
}

/**
 * @brief Debug modunda verileri yansıtır.
 * 
 */
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
  // joystick değerlerini oku
  joystick.JoystickOku(1);
  // Joystickteki değerleri maple ve kaydet.
  joystickMaplenmisDegerler[1] = map(joystick.veriVektorleri[0],0 , 1023, -255, 0) * -1;
  joystickMaplenmisDegerler[0] = map(joystick.veriVektorleri[1],0 , 1023, 0, 255);
  roleSwitchDurumu = digitalRead(SWT); 
  hareketSwitchDurumu = digitalRead(SWT2);
  // Switch değerlerini kaydet.
  joystickMaplenmisDegerler[2] = roleSwitchDurumu;
  joystickMaplenmisDegerler[3] = hareketSwitchDurumu;
}

/**
 * @brief nRF modülünün verileri göndermesini sağlar.
 * 
 */
void Gonder(){
  unsigned long _x = millis();  
  // nRF modülünün veriyi göndermesini sağlar.
  radyoModulu.nRF24VeriGonder(radyo, joystickMaplenmisDegerler, 4);
  Serial.print(millis() - _x);
  Serial.println(" ms, nrf gonderim suresi");
}

#pragma region EkranFonksiyonları
void EkranSec(int ekran=3){
  // CD4056'nin pinlerini kullanarak ekran secer.
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
  // Fontu ayarlar.
  ekran.setFont(u8g_font_7x13);
  // Ekrana yazilabilecek satir sayisi kadar mesajlari yaz.
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

/**
 * @brief int degeri char'a donusturur.
 * 
 * @param deger 
 */
inline void StrHazirla(char (&buffer)[10 + sizeof(char)], int deger){
  sprintf(buffer, "%d", deger);
}

void CizimLoop(){
  unsigned long _x = millis();  
  // Ekrani secer.
  EkranSec(0);
  // Ekrani basina al.
  ekran.firstPage();
  do {
    // Returnlenen degeri diziye yazar.
    const char* _arr[3] = {" ", RoleDurumu(), " "};
    // Dizideki veriyi ekrana yansitir.
    EkranaCiz(_arr);
  } while(ekran.nextPage());

  // Ekrani secer.
  EkranSec(1);
  // Ekrani basina al.
  ekran1.firstPage();
  do{
    // Returnlenen degeri diziye yazar.
    const char* _arr[3] = {" ", HareketDurumu(), " "};
    // Dizideki veriyi ekrana yansitir.
    EkranaCiz1(_arr);
  } while(ekran1.nextPage());

  // Ekrani secer
  EkranSec(2);
  // Ekrani basina al/
  ekran2.firstPage();
  do{
    // int degeri yazmak icin kullandim yorumlari bosver.
    
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
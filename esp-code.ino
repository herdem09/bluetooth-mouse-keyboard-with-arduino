// BleMouse kütüphanesini projeye dahil ediyoruz
// Bu kütüphane ESP32'nin Bluetooth Low Energy (BLE) üzerinden kablosuz fare gibi çalışmasını sağlar
#include <BleMouse.h>

// BLE Mouse nesnesi oluşturuyoruz ve 3 parametre veriyoruz:
// 1. "ESP32 Mouse" - Bluetooth cihaz adı (bilgisayarda bu isimle görünür)
// 2. "DIY Project" - Üretici/imalatçı bilgisi
// 3. 100 - Pil seviyesi yüzdesi (sanal değer)
BleMouse bleMouse("ESP32 Mouse", "DIY Project", 100);

// ========== JOYSTİCK PİN TANIMLARI ==========
// Analog joystick'in X eksenini okumak için GPIO 34 pini kullanılıyor
#define JOY_X 34

// Analog joystick'in Y eksenini okumak için GPIO 35 pini kullanılıyor
#define JOY_Y 35

// Joystick'in ortasındaki basma butonu için GPIO 32 pini (orta tıklama işlevi yapacak)
#define JOY_SW 32

// ========== ROTARY ENCODER PİN TANIMLARI ==========
// Encoder'ın CLK (saat sinyali) pini - dönüş yönünü tespit etmek için kullanılır
#define ENC_CLK 25

// Encoder'ın DT (data/veri) pini - CLK ile birlikte dönüş yönünü belirler
#define ENC_DT 26

// Encoder'ın basma butonu pini
#define ENC_SW 27

// ========== FARE BUTON PİNLERİ ==========
// Sol fare butonu için GPIO 14 pini
#define BTN_LEFT 14

// Sağ fare butonu için GPIO 12 pini
#define BTN_RIGHT 12

// ========== JOYSTICK AYARLARI ==========
// Ölü bölge: Joystick tam ortada değilken bile küçük titreşimler olabilir
// Bu değerden küçük hareketler görmezden gelinir (300 birimlik tolerans)
const int joyDeadzone = 300;

// Joystick'in X eksenindeki merkez noktası
// Başlangıçta varsayılan 2048 (12-bit ADC'nin ortası: 0-4095 aralığının yarısı)
// setup() fonksiyonunda kalibrasyon yapılarak gerçek merkez bulunacak
int joyCenterX = 2048;

// Joystick'in Y eksenindeki merkez noktası
// X ile aynı mantıkla çalışır
int joyCenterY = 2048;

// Fare hızı/hassasiyet çarpanı
// Joystick ne kadar eğilirse fare o kadar hızlı hareket eder (maksimum 16 piksel/adım)
const int mouseSensitivity = 16;

// Son buton kontrolünün yapıldığı zamanı milisaniye cinsinden tutar
// Butonların çok sık okunmasını engellemek için kullanılır
unsigned long lastButtonCheck = 0;

// Buton kontrolleri arasındaki minimum bekleme süresi (50 milisaniye)
// Bu süre debouncing (sıçrama önleme) için gereklidir
const unsigned long buttonDelay = 50;

// ========== ENCODER DEĞİŞKENLERİ ==========
// Encoder'ın CLK pininin önceki durumunu saklar (HIGH veya LOW)
// Yeni durumla karşılaştırarak dönüş olup olmadığını anlarız
int lastClkState = HIGH;

// Encoder butonunun durumu (basılı mı değil mi)
// HIGH = basılı değil, LOW = basılı (INPUT_PULLUP kullanıldığı için)
bool encBtnState = HIGH;

// ========== SETUP FONKSİYONU ==========
// Arduino/ESP32'de program başladığında bir kez çalışan fonksiyon
void setup() {
  // Seri haberleşmeyi başlat (bilgisayarla USB üzerinden veri alışverişi için)
  // 115200 baud rate (saniyede 115200 bit hızında iletişim)
  Serial.begin(115200);
  
  // Debug mesajı: Program başladığını bildir
  Serial.println("🚀 ESP32 BLE Mouse başlatılıyor...");

  // BLE Mouse servisini başlat ve Bluetooth üzerinden yayına geç
  bleMouse.begin();

  // ========== PİN MODLARININ AYARLANMASI ==========
  // JOY_X pinini analog giriş olarak ayarla (0-4095 arası değer okuyacak)
  pinMode(JOY_X, INPUT);
  
  // JOY_Y pinini analog giriş olarak ayarla
  pinMode(JOY_Y, INPUT);
  
  // Joystick butonu için INPUT_PULLUP modu: pin varsayılan olarak HIGH
  // Butona basınca GND'ye bağlanıp LOW olur
  pinMode(JOY_SW, INPUT_PULLUP);
  
  // Encoder CLK pini için dahili pull-up direnci aktif
  pinMode(ENC_CLK, INPUT_PULLUP);
  
  // Encoder DT pini için dahili pull-up direnci aktif
  pinMode(ENC_DT, INPUT_PULLUP);
  
  // Encoder butonu için dahili pull-up direnci aktif
  pinMode(ENC_SW, INPUT_PULLUP);
  
  // Sol fare butonu için dahili pull-up direnci aktif
  pinMode(BTN_LEFT, INPUT_PULLUP);
  
  // Sağ fare butonu için dahili pull-up direnci aktif
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  // Encoder'ın şu anki CLK durumunu oku ve kaydet
  // Sonraki okumalarda değişiklik olup olmadığını anlamak için
  lastClkState = digitalRead(ENC_CLK);

  // ========== JOYSTİCK KALİBRASYONU ==========
  // Joystick'in gerçek merkez noktasını bulmak için kalibrasyon yapıyoruz
  Serial.println("🔧 Joystick merkezleniyor...");
  
  // X ve Y değerlerinin toplamını tutacak değişkenler
  long sumX = 0, sumY = 0;
  
  // 50 kez okuma yapıp ortalamasını alacağız (gürültüyü azaltmak için)
  for (int i = 0; i < 50; i++) {
    // X ekseninin analog değerini oku ve toplama ekle
    sumX += analogRead(JOY_X);
    
    // Y ekseninin analog değerini oku ve toplama ekle
    sumY += analogRead(JOY_Y);
    
    // Her okuma arasında 10 milisaniye bekle
    delay(10);
  }
  
  // 50 okumanın ortalamasını al ve merkez X değeri olarak kaydet
  joyCenterX = sumX / 50;
  
  // 50 okumanın ortalamasını al ve merkez Y değeri olarak kaydet
  joyCenterY = sumY / 50;

  // Bulunan merkez değerlerini seri monitöre yazdır
  Serial.print("📍 Merkez X: ");
  Serial.print(joyCenterX);
  Serial.print(" | Merkez Y: ");
  Serial.println(joyCenterY);

  // Tüm pinlerin hazır olduğunu bildir
  Serial.println("✓ Pinler hazır");
  
  // Encoder test modunun aktif olduğunu bildir
  Serial.println("🔧 Rotary Encoder Test Aktif!");
  
  // Bluetooth bağlantısı beklendiğini bildir
  Serial.println("Bluetooth bağlantısı bekleniyor...");
}

// ========== LOOP FONKSİYONU ==========
// Setup'tan sonra sürekli tekrar eden ana program döngüsü
void loop() {
  // BLE bağlantısının aktif olup olmadığını kontrol et
  if (bleMouse.isConnected()) {
    // Bağlantı varsa fare kontrollerini çalıştır
    
    // Joystick hareketlerini oku ve fare imlecini hareket ettir
    handleJoystick();
    
    // Encoder dönüşlerini oku ve scroll (kaydırma) yap
    handleEncoder();
    
    // Sol ve sağ fare butonlarını kontrol et
    handleMouseButtons();
    
    // Her döngüde 10 milisaniye bekle (CPU kullanımını azaltmak için)
    delay(10);
  } else {
    // Bağlantı yoksa farklı bir işlem yap
    
    // Son bildirim zamanını saklayan statik değişken (bellekte kalıcı)
    static unsigned long lastBlink = 0;
    
    // Son bildirimden bu yana 1000 milisaniye (1 saniye) geçtiyse
    if (millis() - lastBlink > 1000) {
      // Bağlantı bekleniyor mesajı yazdır
      Serial.println("⏳ Bluetooth bağlantısı bekleniyor...");
      
      // Son bildirim zamanını güncelle
      lastBlink = millis();
    }
    
    // Bağlantı yokken daha yavaş kontrol et (100 milisaniye bekle)
    delay(100);
  }
}

// ========== JOYSTİCK KONTROL FONKSİYONU ==========
// Joystick'in konumunu okuyup fare hareketine çeviren fonksiyon
void handleJoystick() {
  // X ekseninin analog değerini oku (0-4095 arası)
  int xVal = analogRead(JOY_X);
  
  // Y ekseninin analog değerini oku (0-4095 arası)
  int yVal = analogRead(JOY_Y);

  // ========== GÜRÜLTÜ FİLTRESİ ==========
  // X değeri merkeze çok yakınsa (±100 birim içinde) merkeze sabitle
  // Bu küçük titreşimleri ve analog gürültüyü yok sayar
  if (abs(xVal - joyCenterX) < 100) xVal = joyCenterX;
  
  // Y değeri merkeze çok yakınsa merkeze sabitle
  if (abs(yVal - joyCenterY) < 100) yVal = joyCenterY;

  // Fare imlecinin X ekseninde hareket miktarını tutacak değişken (başlangıçta 0)
  int moveX = 0;
  
  // Fare imlecinin Y ekseninde hareket miktarını tutacak değişken (başlangıçta 0)
  int moveY = 0;

  // ========== X EKSENİ HAREKETİ ==========
  // Mevcut X değerinin merkezden farkını hesapla (ne kadar eğilmiş)
  int deltaX = xVal - joyCenterX;
  
  // Eğilme miktarı ölü bölgeden büyükse (yani gerçekten hareket var)
  if (abs(deltaX) > joyDeadzone) {
    // Joystick'in maksimum hareket aralığını hesapla
    // Merkez 2048 ise, sağa 2047 sola 2048 gidebilir, büyük olanı al
    int maxRange = max(joyCenterX, 4095 - joyCenterX);
    
    // Joystick eğilmesini (joyDeadzone - maxRange arası) fare hızına (0 - mouseSensitivity arası) dönüştür
    // map() fonksiyonu bir aralıktaki değeri başka bir aralığa orantılı olarak çevirir
    moveX = map(abs(deltaX), joyDeadzone, maxRange, 0, mouseSensitivity);
    
    // Eğer eğilme negatif yöndeyse (sola) hareketi de negatif yap
    if (deltaX < 0) moveX = -moveX;
  }

  // ========== Y EKSENİ HAREKETİ ==========
  // Mevcut Y değerinin merkezden farkını hesapla
  int deltaY = yVal - joyCenterY;
  
  // Eğilme miktarı ölü bölgeden büyükse
  if (abs(deltaY) > joyDeadzone) {
    // Maksimum hareket aralığını hesapla
    int maxRange = max(joyCenterY, 4095 - joyCenterY);
    
    // Joystick eğilmesini fare hızına dönüştür
    moveY = map(abs(deltaY), joyDeadzone, maxRange, 0, mouseSensitivity);
    
    // Eğer eğilme negatif yöndeyse hareketi de negatif yap
    if (deltaY < 0) moveY = -moveY;
  }

  // Eğer X veya Y ekseninde hareket varsa (ikisi de 0 değilse)
  if (moveX != 0 || moveY != 0) {
    // BLE üzerinden fare hareket komutunu gönder
    // bleMouse.move(x_hareketi, y_hareketi)
    bleMouse.move(moveX, moveY);
  }
}

// ========== ROTARY ENCODER FONKSİYONU ==========
// Encoder'ın dönüşünü algılayıp scroll (kaydırma) yapan fonksiyon
void handleEncoder() {
  // CLK pininin şu anki durumunu oku (HIGH veya LOW)
  int clkState = digitalRead(ENC_CLK);
  
  // DT pininin şu anki durumunu oku
  int dtState = digitalRead(ENC_DT);

  // CLK pininin durumu değiştiyse (encoder dönüyor demektir)
  if (clkState != lastClkState) {
    // DT ve CLK farklı durumda ise saat yönünde dönüyor (sağa)
    if (dtState != clkState) {
      // Scroll yukarı komutu gönder
      // bleMouse.move(x, y, scroll) - x=0, y=0, scroll=+1
      bleMouse.move(0, 0, 1);
    } else {
      // DT ve CLK aynı durumdaysa saat yönünün tersine dönüyor (sola)
      // Scroll aşağı komutu gönder
      bleMouse.move(0, 0, -1);
    }
  }
  
  // Bir sonraki karşılaştırma için mevcut durumu kaydet
  lastClkState = clkState;
}

// ========== FARE BUTONLARI FONKSİYONU ==========
// Sol ve sağ fare butonlarını kontrol eden fonksiyon
void handleMouseButtons() {
  // Son buton kontrolünden beri buttonDelay (50ms) geçmediyse fonksiyondan çık
  // Bu debouncing için: butonun çok hızlı okunmasını engeller
  if (millis() - lastButtonCheck < buttonDelay) return;
  
  // Son kontrol zamanını güncelle
  lastButtonCheck = millis();

  // Sol butonun önceki basma durumunu tutan statik değişken
  // static olduğu için fonksiyon bitince değeri kaybolmaz
  static bool leftPressed = false;
  
  // Sağ butonun önceki basma durumunu tutan statik değişken
  static bool rightPressed = false;

  // Sol buton pinini oku - INPUT_PULLUP kullanıldığı için:
  // Basılı değilse HIGH (1), basılıysa LOW (0)
  // ! (NOT) operatörü ile tersine çeviriyoruz: basılıysa true, değilse false
  bool leftBtn = !digitalRead(BTN_LEFT);
  
  // Sağ buton pinini oku ve aynı mantıkla tersine çevir
  bool rightBtn = !digitalRead(BTN_RIGHT);

  // ========== SOL BUTON KONTROLÜ ==========
  // Sol butona basıldı VE daha önce basılı değildi (yeni basma)
  if (leftBtn && !leftPressed) {
    // BLE üzerinden sol fare butonunu bas
    bleMouse.press(MOUSE_LEFT);
    
    // Durumu basılı olarak işaretle
    leftPressed = true;
    
    // Debug mesajı
    Serial.println("🖱️ Sol tıklama BAŞLADI");
  } 
  // Sol butona basılmıyor VE daha önce basılıydı (butonu bırakma)
  else if (!leftBtn && leftPressed) {
    // BLE üzerinden sol fare butonunu bırak
    bleMouse.release(MOUSE_LEFT);
    
    // Durumu basılı değil olarak işaretle
    leftPressed = false;
    
    // Debug mesajı
    Serial.println("🖱️ Sol tıklama BİTTİ");
  }

  // ========== SAĞ BUTON KONTROLÜ ==========
  // Sağ butona basıldı VE daha önce basılı değildi
  if (rightBtn && !rightPressed) {
    // BLE üzerinden sağ fare butonunu bas
    bleMouse.press(MOUSE_RIGHT);
    
    // Durumu basılı olarak işaretle
    rightPressed = true;
    
    // Debug mesajı
    Serial.println("🖱️ Sağ tıklama BAŞLADI");
  } 
  // Sağ butona basılmıyor VE daha önce basılıydı
  else if (!rightBtn && rightPressed) {
    // BLE üzerinden sağ fare butonunu bırak
    bleMouse.release(MOUSE_RIGHT);
    
    // Durumu basılı değil olarak işaretle
    rightPressed = false;
    
    // Debug mesajı
    Serial.println("🖱️ Sağ tıklama BİTTİ");
  } 
}

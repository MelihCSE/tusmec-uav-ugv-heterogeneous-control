# İHA ve İKA Entegrasyonu - Heterojen Sürü Koordinasyonu

## 1. Proje Mimarisi
* '/docs': Donanım bileşen listeleri, bağlantı şemaları ve değerlendirme raporu.
* 'params': ArduCopter ve ArduRover için yapılandırılmış paramtere dosyaları.
* '/src': MAVLink protokolü üzerinden haberleşen C tabanlı koordinatör yazılımı.
* '/simulation': ArduPilot SITL simülasyon ortamı yapılandırmaları ve test notları.

## 2. Donanım Bileşenleri ve Bağlantı Şemaları
* UAV - GRAPH

```mermaid
graph TD
  BATTERY --> POWER_MODULE
  POWER_MODULE --> |5V| FLIGHT_CONTROLLER
  POWER_MODULE --> |High-Power| ESC[ESC x4]
  FLIGHT_CONTROLLER --> |PWM_SIGNAL| ESC
  ESC --> MOTOR[x4]
  GPS <-->|UART & I2C| FLIGHT_CONTROLLER
  RC --> FLIGHT_CONTROLLER
  TELEMETER <-->|UART-MAVLink| FLIGHT_CONTROLLER
  TELEMETER <-.->|Radio Free - MAVLink| GCS
```

* UGV - GRAPH
```mermaid
graph TD
  BATTERY -->|5V| BEC
  BATTERY -->|Motor Power| MOTOR_DRIVER
  BEC -->|5V| MCU
  MCU -->|PWM| MOTOR_DRIVER
  MOTOR_DRIVER --> L_MOTOR
  MOTOR_DRIVER --> R_DRIVER
  L_MOTOR --> ENCODER
  R_MOTOR --> ENCODER
  ENCODER -->|DIGITAL I/O| MCU
  GPS <-->|UART & I2C| MCU
  TELEMETER <-->|UART - MAVLink| MCU
  TELEMETER <-.->|Radio Freq - MAVLink| GCS
```

## 3. Kurulum ve Çalıştırma Adımları 
1. **Simulasyonun başlatılması:** ...
2. **Koordinatör Yazılımının Derlenmesi:** ...

## 4. Uygulanan Algoritma ve Senaryo
* **Senaryo:** İHa belirlenen alanı tarar ve tespit ettiği hedef konumu koordinatöre iletir, koordinatör bu hedefe gitme görevini İKA'ya atar.

## 5. Simülasyon Test Videosu
*(Ekran kaydı bağlantısı buraya eklenecektir.)

## 6. Referanslar ve Literatür Taraması
Projenin mimari tasarımı, simülasyon kurgusu ve heterojen koordinasyon mantığı oluşturulurken aşağıdaki akademik ve teknik kaynaklardan faydalanılmıştır:

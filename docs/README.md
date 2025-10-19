# FsmOS Documentation

Bu dizin FsmOS kütüphanesinin Doxygen dokümantasyonunu içerir.

## Dokümantasyon Oluşturma

### Gereksinimler
- Doxygen 1.9.8 veya üzeri
- Linux/Unix ortamı

### Komutlar

```bash
# Dokümantasyon oluştur
doxygen Doxyfile

# Dokümantasyonu temizle
rm -rf docs/
```

### Çıktı
Dokümantasyon `docs/html/` dizininde HTML formatında oluşturulur.

Ana sayfa: `docs/html/index.html`

## Doxyfile Konfigürasyonu

Doxyfile şu özellikleri içerir:

### Proje Bilgileri
- **Proje Adı**: FsmOS
- **Versiyon**: 1.3.0
- **Açıklama**: A lightweight cooperative task scheduler for Arduino
- **Yazar**: Aykut Özdemir <aykutozdemirgyte@gmail.com>

### Çıktı Ayarları
- **HTML**: Aktif (ana çıktı formatı)
- **LaTeX**: Pasif
- **RTF**: Pasif
- **Man Pages**: Pasif
- **XML**: Pasif

### Dokümantasyon Ayarları
- **EXTRACT_ALL**: Tüm sembolleri dokümante et
- **EXTRACT_PRIVATE**: Private üyeleri dahil et
- **EXTRACT_STATIC**: Static üyeleri dahil et
- **HIDE_UNDOC_MEMBERS**: Dokümante edilmemiş üyeleri gizle
- **CASE_SENSE_NAMES**: Büyük/küçük harf duyarlılığı

### Grafik Ayarları
- **CLASS_GRAPH**: Sınıf diyagramları
- **COLLABORATION_GRAPH**: İşbirliği diyagramları
- **INCLUDE_GRAPH**: Include diyagramları
- **CALL_GRAPH**: Çağrı diyagramları (pasif)

### Giriş Dosyaları
- `lib/FsmOS/FsmOS.h` - Ana header dosyası
- `lib/FsmOS/FsmOS.cpp` - Ana implementation dosyası
- `lib/FsmOS/examples/` - Örnek kodlar
- `README.md` - Ana sayfa olarak kullanılır

## Dokümantasyon İçeriği

### Sınıflar
- **Task**: Temel görev sınıfı
- **Scheduler**: Görev zamanlayıcısı
- **SharedMsg**: Paylaşımlı mesaj sınıfı
- **MsgDataPool**: Mesaj veri havuzu
- **LinkedQueue**: Bağlantılı kuyruk
- **Mutex**: Karşılıklı dışlama
- **Semaphore**: Semafor

### Veri Yapıları
- **TaskNode**: Görev düğümü
- **TaskStats**: Görev istatistikleri
- **SystemMemoryInfo**: Sistem bellek bilgisi
- **ResetInfo**: Sıfırlama bilgisi
- **MemoryStats**: Bellek istatistikleri

### Enum'lar
- **Priority**: Görev öncelik seviyeleri
- **State**: Görev durumları
- **LogLevel**: Log seviyeleri
- **ResetCause**: Sıfırlama nedenleri

## Özellikler

### Dokümantasyon Kalitesi
- ✅ Tüm public sınıflar dokümante edilmiş
- ✅ Tüm public metodlar dokümante edilmiş
- ✅ Parametre açıklamaları mevcut
- ✅ Dönüş değeri açıklamaları mevcut
- ✅ Örnek kodlar dahil edilmiş
- ✅ Sınıf diyagramları oluşturulmuş

### Navigasyon
- **Ana Sayfa**: Proje genel bakış
- **Sınıflar**: Tüm sınıfların listesi
- **Dosyalar**: Kaynak dosyalar
- **Fonksiyonlar**: Alfabetik fonksiyon listesi
- **Modüller**: Grup dokümantasyonu
- **Örnekler**: Örnek kodlar

## Sorun Giderme

### Uyarılar
Dokümantasyon oluşturulurken bazı uyarılar görülebilir:
- Obsolete tag uyarıları (normal)
- Undocumented symbol uyarıları (kontrol edilmeli)

### Performans
- Dokümantasyon oluşturma süresi: ~5-10 saniye
- Çıktı boyutu: ~2-5 MB
- Dosya sayısı: ~200+ HTML dosyası

## Güncelleme

Kod değişikliklerinden sonra dokümantasyonu yeniden oluşturun:

```bash
# Temizle ve yeniden oluştur
rm -rf docs/
doxygen Doxyfile
```

## Lisans

Bu dokümantasyon FsmOS kütüphanesi ile aynı lisans altındadır.
Copyright 2025 Aykut Özdemir <aykutozdemirgyte@gmail.com>

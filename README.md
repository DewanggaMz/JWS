# JWS ESP32

Firmware jadwal waktu sholat berbasis ESP32 untuk panel P10 3×1
(resolusi 96×16), RTC DS3231, web server, LittleFS, dan modul relay
4-channel active-low.

## Fitur utama

- Perhitungan jadwal Imsak, Subuh, Terbit, Dhuha, Dzuhur, Ashar,
  Maghrib, dan Isya.
- Jadwal dihitung ulang ketika tanggal berganti dan setelah konfigurasi
  prayer diperbarui.
- Konfigurasi persisten dalam `/database.json` di LittleFS.
- Koneksi Wi-Fi mode AP atau STA.
- Web API JSON dengan CORS.
- Lima layout panel:
  - Layout 1: jam besar, jadwal sholat, dan running text.
  - Layout 2: running text layar penuh.
  - Layout 3: rangkaian slide teks.
  - Layout 4: jam, tanggal, pasaran, Hijriah, dan running text.
  - Layout 5: countdown sholat berikutnya di bagian atas dan pesan
    `MEMASUKI WAKTU <NAMA SHOLAT>` di bagian bawah. Pesan diulang dua kali.
- Scheduler relay normal dan skenario khusus Jumat.

Urutan layout yang aktif saat ini adalah Layout 1 → Layout 4 →
Layout 5 → Layout 2. Layout 3 tersedia, tetapi belum dimasukkan ke
rotasi pada `src/panel/panelSetup.cpp`.

## Hardware dan pin

### RTC DS3231

| Sinyal | GPIO |
|---|---:|
| SDA | 16 |
| SCL | 17 |

### Panel P10

| Sinyal | GPIO |
|---|---:|
| OE | 22 |
| A | 19 |
| B | 21 |
| CLK | 18 |
| SCLK/Latch | 2 |
| DATA | 23 |

### Relay active-low

| Channel | GPIO |
|---|---:|
| Relay 1 | 25 |
| Relay 2 | 26 |
| Relay 3 | 27 |
| Relay 4 | 32 |

`LOW` berarti relay ON dan `HIGH` berarti relay OFF. Semua relay
diinisialisasi OFF saat boot.

Pastikan modul relay kompatibel dengan logika ESP32 3,3 V dan ground
ESP32 terhubung dengan ground modul relay.

## Build dan upload

Project menggunakan PlatformIO dengan environment
`esp32doit-devkit-v1`.

```bash
pio run
pio run --target upload
pio run --target uploadfs
pio device monitor
```

Baud rate serial monitor adalah `115200`.

> **Perhatian:** `uploadfs` mengunggah seluruh folder `data` dan dapat
> menimpa `database.json` yang sudah dikonfigurasi melalui API.
> Ambil cadangan terlebih dahulu melalui `GET /database`.

## Database

File sumber berada di `data/database.json`. Saat berjalan, firmware
membaca dan menulis `/database.json` pada LittleFS.

Struktur utamanya:

```json
{
  "prayerTimesConfig": {
    "location": {
      "latitude": -8.24523,
      "longitude": 112.600482,
      "timezoneOffsetMinutes": 420
    },
    "calculation": {
      "method": "INDONESIA",
      "fajrAngle": 20,
      "ishaAngle": 18,
      "ishaIsInterval": false,
      "ishaMinutes": 0,
      "asrMethod": "SHAFII",
      "highLatitudeRule": "NONE",
      "imsakOffsetMinutes": 10,
      "duhaAngle": 4,
      "adjustments": {
        "fajr": 0,
        "sunrise": 0,
        "dhuhr": 0,
        "asr": 0,
        "maghrib": 0,
        "isha": 0
      }
    }
  },
  "hijriConfig": {
    "correct": 1
  },
  "wifiConfig": {
    "mode": "AP",
    "ssid": "JWS-ESP32",
    "password": "password-minimal-8-karakter"
  },
  "relayConfig": {
    "enabled": true,
    "prePrayerMinutes": 10,
    "fridayPrePrayerMinutes": 40,
    "relay12OnDelaySeconds": 2,
    "relay12OffDelayMinutes": 5,
    "tartilSubuh": true,
    "tartilDzuhur": true,
    "tartilJumat": true,
    "tartilAshar": true,
    "tartilMagrib": true,
    "tartilIsha": true
  },
  "panelMessages": {
    "layout1": {
      "bottom": "NAMA MASJID",
      "repeatCount": 3,
      "prayerDisplay": {
        "showImsak": true,
        "showSunrise": true,
        "showDhuha": true
      }
    },
    "layout2": {
      "running": "PESAN LAYOUT 2"
    },
    "layout3": {
      "slides": ["SLIDE 1", "SLIDE 2"]
    },
    "layout4": {
      "showPasaran": true,
      "showHijriDate": true,
      "repeatCount": 1,
      "running": "PESAN LAYOUT 4"
    }
  }
}
```

## Ringkasan endpoint

| Method | Path | Keterangan |
|---|---|---|
| GET | `/` | Frontend statis dari LittleFS |
| GET | `/konfigurasi_prayer` | Membaca konfigurasi perhitungan jadwal |
| GET | `/database` | Membaca seluruh isi `database.json` |
| POST | `/konfigurasi_prayer` | Memperbarui konfigurasi dan menghitung ulang jadwal |
| POST | `/time_adjust` | Mengatur tanggal dan waktu RTC |
| POST | `/api/messages/layout1` | Mengubah pesan Layout 1 |
| POST | `/api/messages/layout2` | Mengubah pesan Layout 2 |
| POST | `/api/messages/layout3` | Mengubah slide Layout 3 |
| POST | `/api/messages/layout4` | Mengubah pesan/opsi Layout 4 |
| POST | `/api/layout1/prayer-times` | Mengatur item jadwal opsional Layout 1 |
| POST | `/api/wifi/config` | Mengubah mode dan kredensial Wi-Fi |
| POST | `/api/relay/config` | Mengubah timing scheduler relay |
| POST | `/api/relay/prayer-states` | Mengaktifkan scheduler per waktu sholat |

Semua endpoint POST menerima `Content-Type: application/json`.
Payload konfigurasi dapat dikirim parsial kecuali disebutkan berbeda.

## Dokumentasi endpoint

### `GET /konfigurasi_prayer`

Mengembalikan konfigurasi perhitungan jadwal yang aktif.

Contoh respons:

```json
{
  "success": true,
  "prayerTimesConfig": {
    "location": {
      "latitude": -8.24523,
      "longitude": 112.600482,
      "timezoneOffsetMinutes": 420
    },
    "calculation": {
      "method": "INDONESIA"
    }
  }
}
```

### `GET /database`

Mengembalikan seluruh isi `/database.json` tanpa pembungkus respons.

> Endpoint ini juga mengembalikan password Wi-Fi dalam bentuk teks.
> API saat ini tidak memiliki autentikasi; batasi akses jaringan ke
> perangkat sebelum digunakan di lingkungan publik.

### `POST /konfigurasi_prayer`

Memperbarui field konfigurasi prayer yang dikirim, menyimpannya ke
database, lalu menjadwalkan kalkulasi ulang. Jadwal baru otomatis
diterapkan ke Layout 1, Layout 5, dan scheduler relay.

```json
{
  "location": {
    "latitude": -7.7956,
    "longitude": 110.3695,
    "timezoneOffsetMinutes": 420
  },
  "calculation": {
    "method": "INDONESIA",
    "fajrAngle": 20,
    "ishaAngle": 18,
    "asrMethod": "SHAFII",
    "highLatitudeRule": "NONE",
    "imsakOffsetMinutes": 10,
    "duhaAngle": 4,
    "adjustments": {
      "fajr": 0,
      "sunrise": 0,
      "dhuhr": 0,
      "asr": 0,
      "maghrib": 0,
      "isha": 0
    }
  }
}
```

Nilai yang didukung:

- `method`: `ISNA`, `EGYPT`, `MAKKAH`, `INDONESIA`, atau `CUSTOM`.
- `asrMethod`: `SHAFII` atau `HANAFI`.
- `highLatitudeRule`: `NONE`, `MIDDLE_OF_NIGHT`, `ONE_SEVENTH`,
  atau `ANGLE_BASED`.

Respons sukses memuat `"scheduleRefreshQueued": true`.

### `POST /time_adjust`

Mengatur RTC DS3231.

```json
{
  "day": 29,
  "month": 6,
  "year": 2026,
  "hour": 12,
  "minute": 30,
  "second": 0
}
```

Jika tanggal berubah, jadwal sholat dihitung ulang dan tampilan panel
ikut diperbarui.

### `POST /api/messages/layout1`

```json
{
  "bottom": "MASJID BAITUROHMAH",
  "repeatCount": 3
}
```

- `bottom`: string 1–512 karakter.
- `repeatCount`: angka 1–255.

### `POST /api/messages/layout2`

```json
{
  "running": "INFORMASI KEGIATAN MASJID"
}
```

`running` harus berupa string 1–512 karakter.

### `POST /api/messages/layout3`

```json
{
  "slides": [
    "10 MENIT",
    "LAGI",
    "MEMASUKI",
    "WAKTU SHOLAT"
  ]
}
```

`slides` harus berisi 1–12 string. Setiap string maksimal 128
karakter.

### `POST /api/messages/layout4`

```json
{
  "running": "JAGA KEBERSIHAN MASJID",
  "showPasaran": true,
  "showHijriDate": true,
  "repeatCount": 1
}
```

- `running`: string 1–512 karakter.
- `showPasaran`: boolean.
- `showHijriDate`: boolean.
- `repeatCount`: angka 1–255.

### `POST /api/layout1/prayer-times`

Mengatur jadwal tambahan pada bagian atas Layout 1.

```json
{
  "showImsak": true,
  "showSunrise": true,
  "showDhuha": false
}
```

Semua field berupa boolean dan dapat dikirim parsial. Jadwal wajib
Subuh, Dzuhur, Ashar, Maghrib, dan Isya tetap ditampilkan.

### `POST /api/wifi/config`

```json
{
  "mode": "STA",
  "ssid": "Nama-Router",
  "password": "password-router"
}
```

- `mode`: `AP` atau `STA`, tidak peka huruf besar/kecil.
- `ssid`: 1–32 karakter.
- `password`: 8–63 karakter.

Konfigurasi disimpan langsung, tetapi baru aktif setelah perangkat
direstart. Respons memuat `"restartRequired": true`.

Jika mode STA gagal terhubung selama 15 detik, koneksi dianggap gagal.

### `POST /api/relay/config`

Mengatur timing scheduler relay. Payload dapat dikirim parsial.

```json
{
  "enabled": true,
  "prePrayerMinutes": 10,
  "fridayPrePrayerMinutes": 40,
  "relay12OnDelaySeconds": 2,
  "relay12OffDelayMinutes": 5
}
```

- `enabled`: mengaktifkan atau mematikan seluruh scheduler.
- `prePrayerMinutes`: Relay 4 ON sebelum sholat, 1–180 menit.
- `fridayPrePrayerMinutes`: Relay 3 ON sebelum Dzuhur Jumat; harus
  lebih besar dari `prePrayerMinutes` dan maksimal 180 menit.
- `relay12OnDelaySeconds`: jeda sebelum Relay 1 dan 2 ON; minimal 0
  dan tidak boleh lebih besar dari `prePrayerMinutes × 60`.
- `relay12OffDelayMinutes`: Relay 1 dan 2 OFF setelah waktu sholat
  normal, 0–180 menit.

Perubahan langsung diterapkan tanpa restart.

### `POST /api/relay/prayer-states`

Mengaktifkan atau menonaktifkan seluruh urutan relay per waktu sholat.

```json
{
  "tartilSubuh": true,
  "tartilDzuhur": false,
  "tartilJumat": true,
  "tartilAshar": true,
  "tartilMagrib": true,
  "tartilIsha": true
}
```

Payload dapat dikirim parsial. Jika sebuah state `false`, Relay
1/2/3/4 tidak dijalankan untuk waktu tersebut.

`tartilDzuhur` hanya berlaku pada hari selain Jumat. Pada hari Jumat,
scheduler khusus membaca `tartilJumat`, sehingga kombinasi berikut
tetap menjalankan skenario Jumat:

```json
{
  "tartilDzuhur": false,
  "tartilJumat": true
}
```

## Urutan scheduler relay

### Hari biasa

Untuk Subuh, Dzuhur, Ashar, Magrib, dan Isya yang state-nya aktif:

1. Relay 4 ON pada `prePrayerMinutes` sebelum waktu sholat.
2. Relay 1 dan 2 ON setelah `relay12OnDelaySeconds`.
3. Relay 4 OFF tepat ketika masuk waktu sholat.
4. Relay 1 dan 2 OFF setelah `relay12OffDelayMinutes`.

### Hari Jumat

Jika `tartilJumat` aktif:

1. Relay 3 ON pada `fridayPrePrayerMinutes` sebelum Dzuhur.
2. Relay 1 dan 2 ON setelah `relay12OnDelaySeconds`.
3. Saat tersisa `prePrayerMinutes`, Relay 3 OFF dan Relay 4 ON.
4. Tepat waktu Dzuhur, semua relay OFF.

## Format respons dan error

Respons konfigurasi umumnya berbentuk:

```json
{
  "success": true,
  "message": "Konfigurasi berhasil diperbarui"
}
```

Payload tidak valid mengembalikan HTTP `400`. Kegagalan membaca file
atau menerapkan data internal dapat mengembalikan HTTP `500`.

## Struktur source

```text
data/                         Frontend dan database awal LittleFS
include/panel/                Deklarasi layout panel
include/services/             Deklarasi service
src/panel/                    Implementasi layout dan animasi
src/services/database/        Akses database JSON
src/services/panel_messages/  Konfigurasi pesan panel
src/services/prayer_times/    Konfigurasi kalkulasi jadwal
src/services/relay/           Scheduler dan driver relay
src/services/wifi_config/     Konfigurasi Wi-Fi
src/server_app.cpp            Registrasi endpoint HTTP
src/handlers.cpp              Handler endpoint
src/main.cpp                  Setup dan loop utama
```

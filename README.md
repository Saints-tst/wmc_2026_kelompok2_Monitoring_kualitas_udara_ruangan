# 🌬️ Sistem Monitoring Kualitas Udara Ruangan Berbasis ESP32 dan Sensor MQ-135 (Pendekatan Simulasi)

Proyek mata kuliah **Wireless/Mobile Computing** — simulasi sistem IoT untuk memantau kualitas udara ruangan menggunakan **ESP32** dan sensor **MQ-135** pada simulator **Wokwi**, dengan **Blynk** sebagai server, penyimpan data, dan dashboard.

![Platform](https://img.shields.io/badge/Platform-ESP32-blue)
![Simulator](https://img.shields.io/badge/Simulator-Wokwi-green)
![IoT](https://img.shields.io/badge/IoT-Blynk-brightgreen)
![Status](https://img.shields.io/badge/Status-Proposal%20%2F%20Simulasi-orange)

---

## 📌 Deskripsi

Ventilasi yang kurang baik membuat gas seperti CO₂, amonia, dan uap organik menumpuk di dalam ruangan (ruang kuliah, laboratorium, kantor), sehingga menimbulkan rasa pengap, sakit kepala, dan menurunnya konsentrasi. Nilai mentah sensor sulit dipahami pengguna awam, sehingga sistem ini mengubah pembacaan sensor menjadi **status udara (Baik / Sedang / Buruk)** dan **rata-rata per jam** agar tren kualitas udara mudah dipantau.

Seluruh pengerjaan dilakukan **secara simulasi tanpa perangkat fisik**:

- ESP32 dijalankan pada simulator daring **Wokwi**
- Keluaran sensor MQ-135 dibangkitkan secara virtual (potensiometer dan/atau data buatan berpola waktu)
- Data dikirim ke **Blynk Cloud** yang berperan sebagai server, penyimpan data, sekaligus dashboard

## 🎯 Tujuan

1. Membangun prototipe simulasi monitoring kualitas udara berbasis ESP32 dan MQ-135 di Wokwi yang terhubung ke Blynk.
2. Menerapkan klasifikasi status udara (baik, sedang, buruk) berdasarkan ambang batas.
3. Menghasilkan rata-rata kualitas udara per jam secara otomatis dan menampilkannya pada dashboard Blynk.
4. Mengukur ketepatan konversi pembacaan dan akurasi klasifikasi terhadap perhitungan acuan dan skenario data simulasi.

## 🏗️ Arsitektur Sistem

```
┌──────────────┐   ┌─────────────┐   ┌─────────────────────┐   ┌──────────┐   ┌──────────────────┐   ┌──────────────────┐
│ Sensor MQ-135│ → │    ESP32    │ → │ Konversi, klasifikasi│ → │  Wi-Fi   │ → │   Blynk Cloud    │ → │ Dashboard Blynk  │
│  (virtual)   │   │   (Wokwi)   │   │  & rata-rata per jam │   │ (virtual)│   │(server & database)│   │  (web / mobile)  │
└──────────────┘   └─────────────┘   └─────────────────────┘   └──────────┘   └──────────────────┘   └──────────────────┘
```

## ⚙️ Alur Kerja Program

1. **Inisialisasi** — ESP32 terhubung ke Wi-Fi virtual, lalu ke Blynk Cloud (template ID + auth token), dan sinkronisasi waktu via NTP. Pemanasan sensor disimulasikan dengan penundaan beberapa detik.
2. **Pembacaan** — Nilai ADC dibaca setiap **10 detik** dengan beberapa kali sampling dan filter sederhana.
3. **Konversi** — ADC → tegangan → resistansi sensor (Rs) → rasio Rs/R0 → ppm ekuivalen berdasarkan kurva datasheet.
4. **Klasifikasi** — Status ditentukan berdasarkan ambang batas (lihat tabel di bawah).
5. **Rata-rata per jam** — Setiap sampel masuk ke akumulator (jumlah & banyak sampel). Saat jam berganti (waktu NTP), rata-rata dihitung, diklasifikasikan, lalu akumulator di-reset (±360 sampel per jam).
6. **Pengiriman** — Nilai real-time, status, dan rata-rata per jam dikirim ke Blynk melalui virtual pin (datastream).
7. **Gangguan jaringan** — Jika Wi-Fi virtual terputus, data disimpan sementara di ESP32 dan dikirim ulang setelah koneksi pulih.

### Rumus Konversi

```
Rs  = RL × (Vc − Vout) / Vout
ppm = a × (Rs / R0)^b
```

> Parameter `a` dan `b` diambil dari kurva datasheet MQ-135 pada tahap implementasi. `R0` berupa konstanta karena tidak ada kalibrasi pada sensor nyata.

### Klasifikasi Status Udara

| Status | Rentang (ppm ekuivalen) | Keterangan |
|--------|-------------------------|------------|
| 🟢 Baik | < 1000 | Udara segar, ventilasi memadai |
| 🟡 Sedang | 1000 – 2000 | Mulai pengap, disarankan membuka ventilasi |
| 🔴 Buruk | > 2000 | Kualitas udara rendah, perlu tindakan segera |

*Ambang batas dapat disesuaikan setelah pengujian simulasi.*

## 🧪 Rancangan Simulasi

- **Simulator:** Wokwi dengan ESP32 DevKit (proyek dibagikan lewat tautan untuk kerja bersama).
- **Platform IoT:** Blynk Cloud & Blynk Console — satu template (datastream + dashboard) dan satu perangkat untuk memperoleh auth token.
- **Pengganti sensor MQ-135:** potensiometer pada pin ADC ESP32 (diputar untuk mengubah "kadar gas").
- **Data buatan berpola:** ruangan tertutup (naik bertahap), ventilasi terbuka (turun), dan ruangan ramai (tinggi & berfluktuasi).
- **Indikator status:** LED hijau/kuning/merah dan/atau buzzer virtual.

> **Catatan perangkat nyata:** keluaran MQ-135 (hingga 5 V) perlu diturunkan dengan pembagi tegangan sebelum masuk ADC ESP32 (maks. 3,3 V), dan sensor dicatu 5 V karena pemanasnya membutuhkan tegangan tersebut.

## 🗄️ Rancangan Data (Blynk)

Penyimpanan memakai **datastream Blynk Cloud** (bukan basis data relasional) yang otomatis menyimpan riwayat nilai beserta waktunya.

### Datastream

| Datastream | Virtual Pin | Tipe Data | Keterangan |
|------------|:-----------:|-----------|------------|
| `nilai_adc` | V0 | Integer | Nilai ADC dari simulasi (data mentah) |
| `ppm` | V1 | Double | Nilai ppm ekuivalen hasil konversi |
| `status` | V2 | String | Status sesaat: baik, sedang, atau buruk |
| `rata_ppm` | V3 | Double | Rata-rata ppm dalam satu jam |
| `status_rata` | V4 | String | Status berdasarkan rata-rata per jam |
| `jumlah_sampel` | V5 | Integer | Banyak sampel yang masuk dalam satu jam |

### Widget Dashboard

| Widget | Datastream | Keterangan |
|--------|------------|------------|
| Gauge | `ppm` (V1) | Nilai ppm real-time |
| Label | `status` (V2) | Status udara sesaat |
| SuperChart | `ppm` (V1) | Grafik tren ppm terhadap waktu |
| Label | `rata_ppm` (V3) | Rata-rata ppm per jam terakhir |
| Label | `status_rata` (V4) | Status berdasarkan rata-rata per jam |
| Label | `jumlah_sampel` (V5) | Banyak sampel dalam satu jam |

## 🧰 Alat dan Bahan

| Komponen | Fungsi |
|----------|--------|
| Wokwi | Menjalankan ESP32 DevKit virtual, Wi-Fi virtual, dan Serial Monitor |
| Komponen virtual Wokwi (potensiometer, LED, buzzer) | Pengganti sensor MQ-135 dan indikator status |
| Arduino IDE / PlatformIO / editor Wokwi | Penulisan & pengujian kode, termasuk library Blynk untuk ESP32 |
| Blynk (Cloud & Console) | Server, penyimpanan data, pengelolaan template dan perangkat |
| Dashboard Blynk (web & mobile) | Menampilkan nilai real-time, status, dan grafik |
| Spreadsheet (Excel / Google Sheets) | Perhitungan acuan untuk validasi |
| GitHub | Penyimpanan kode dan kerja tim |

## ✅ Target Pengujian

| No | Pengujian | Target |
|:--:|-----------|--------|
| 1 | Validasi parameter konversi | Keluaran ppm konsisten dan masuk akal pada seluruh rentang ADC |
| 2 | Akurasi konversi (dibanding spreadsheet, MAPE) | MAPE < 1% |
| 3 | Akurasi klasifikasi (confusion matrix, 3 skenario) | Akurasi ≥ 80% |
| 4 | Rata-rata per jam (Blynk vs perhitungan manual) | Selisih mendekati 0 |
| 5 | Pengiriman data (termasuk simulasi Wi-Fi terputus) | Keberhasilan ≥ 95% |
| 6 | Operasi kontinu (target awal 24 jam) | Berjalan tanpa restart manual |

*Angka target bersifat awal dan dapat disesuaikan dengan hasil uji pendahuluan.*

## 🚧 Batasan Masalah

- Seluruh pengerjaan berupa simulasi tanpa perangkat fisik; data bukan data lingkungan nyata.
- MQ-135 tidak selektif terhadap satu gas, sehingga hasil berupa estimasi (ppm ekuivalen).
- Tidak ada kalibrasi sensor nyata maupun perbandingan dengan alat ukur pembanding; `R0` ditetapkan sebagai konstanta.
- Pengujian pada satu skenario ruangan virtual.
- Klasifikasi rule-based dengan ambang batas tetap (bukan machine learning).
- Tidak ada kompensasi suhu dan kelembapan.
- Gangguan jaringan hanya ditangani dengan penyimpanan sementara di ESP32.
- Kapasitas dan lama penyimpanan riwayat mengikuti batasan paket Blynk yang digunakan.
- Hasil simulasi belum membuktikan kinerja pada perangkat dan ruangan sebenarnya.

## 📅 Rencana Kegiatan

| Kegiatan | Bln 1 | Bln 2 | Bln 3 | Bln 4 |
|----------|:----:|:----:|:----:|:----:|
| Studi literatur | ✓ | | | |
| Perancangan sistem dan datastream Blynk | ✓ | ✓ | | |
| Implementasi simulasi, konfigurasi Blynk, dan penetapan parameter konversi | | ✓ | ✓ | |
| Pengujian dan pengambilan data simulasi | | | ✓ | ✓ |
| Analisis dan penyusunan laporan | | | | ✓ |

## 🚀 Cara Menjalankan (Rencana)

1. Buat proyek **ESP32 DevKit** di [Wokwi](https://wokwi.com) lalu rangkai potensiometer, LED (hijau/kuning/merah), dan buzzer.
2. Buat template, datastream (V0–V5), dan dashboard di [Blynk Console](https://blynk.cloud), lalu daftarkan satu perangkat.
3. Salin `BLYNK_TEMPLATE_ID`, `BLYNK_TEMPLATE_NAME`, dan `BLYNK_AUTH_TOKEN` ke kode ESP32.
4. Jalankan simulasi di Wokwi dan amati dashboard Blynk.

> ⚠️ **Jangan commit auth token Blynk ke repositori publik.** Gunakan placeholder pada kode yang diunggah.

## 👥 Tim — Kelompok 2

| Nama | NIM | Peran |
|------|-----|-------|
| Maulidzhan Syam Ikraam | 23210095 | Rangkaian simulasi Wokwi, generator data sensor virtual, parameter kurva konversi |
| Muhammad Fayyadh Alghifari | 23210010 | Pemrograman ESP32 (pembacaan, konversi, klasifikasi, rata-rata per jam, pengiriman ke Blynk) Konfigurasi Blynk (template, datastream, perangkat, dashboard) |
| Rahmad Fadli | 23210129 | Dokumentasi Pengujian dan analisis data |
| Darmawansya | 23210080 | Penyusunan laporan |

## 🏫 Institusi

- **Mata Kuliah:** Wireless/Mobile Computing
- **Dosen Pengampu:** Ahmad Mujahid Abdurrahman, S.Kom, M.T
- **Program Studi:** Ilmu Komputer
- **Fakultas:** Sains Teknologi dan Ilmu Kesehatan
- **Universitas:** Universitas Bina Bangsa Getsempena, Banda Aceh
- **Tahun:** 2026

## 🔭 Pengembangan Lanjutan

- Kalibrasi sensor nyata dan perbandingan dengan alat ukur pembanding
- Penambahan sensor suhu dan kelembapan
- Peringatan otomatis (notifikasi)
- Implementasi pada perangkat ESP32 dan MQ-135 fisik

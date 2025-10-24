# 🧠 ESP32 BLE Mouse Project

This project was developed at **İTÜ MTAL** (Istanbul Technical University Vocational and Technical Anatolian High School) as part of our **Robotics Class**.
It represents both a **technical application** of our electronics knowledge and a **practical reflection** of the **Ahi (Ahilik) system** we apply in our school.

---

## ⚙️ Project Overview

This project turns an **ESP32** into a **Bluetooth Mouse** using the `BleMouse` library.
A joystick controls the cursor, a rotary encoder handles scrolling, and physical buttons perform left/right mouse clicks — all wirelessly via Bluetooth Low Energy (BLE).

It’s our **first project** of the year and part of our **quarterly presentations** in the robotics course. Each team member contributes to design, coding, and debugging.

---

## 🧿 The Ahi System at İTÜMTAL

İTÜMTAL applies the **Ahilik (Ahi) system**, an ancient Turkish guild tradition that emphasizes **mentorship, craftsmanship, ethics, and solidarity**.
In this system:

* **Experienced students (Hami)** teach and guide others (their “Apprentices”).
* **Knowledge flows from those who know to those who learn.**
* The focus is not only on technical skills but also on **sharing, patience, and responsibility.**

In this project, I’m acting as a **Hami**, teaching my friends how to use **Arduino and ESP32** for robotics applications.
Together, we build, learn, and present our work to others — keeping the Ahi spirit alive through modern technology.

---

## 💡 Technical Details

**Main Components:**

* **ESP32S NodeMCU** – Microcontroller with BLE capability
* **Joystick** – Controls cursor movement (X and Y axes)
* **Rotary Encoder** – Scroll wheel simulation
* **Push Buttons** – Left and right click inputs
* **BleMouse Library** – Enables BLE-based mouse functionality

**Features:**

* Smooth joystick-based cursor control
* Encoder-based scrolling (up/down)
* Debounced button presses for stable clicks
* Automatic joystick calibration at startup
* Debug messages via Serial Monitor

---

## 🔩 Libraries Used

```cpp
#include <BleMouse.h>
```

You can install the **ESP32 BLE Mouse** library via the Arduino Library Manager or from [T-vK’s GitHub repository](https://github.com/T-vK/ESP32-BLE-Mouse).

---

## 🚀 How to Use

1. Upload the code to your **ESP32** board.
2. Power the board and wait for the message:

   ```
   🚀 ESP32 BLE Mouse başlatılıyor...
   Bluetooth bağlantısı bekleniyor...
   ```
3. Pair your computer with **“ESP32 Mouse”** via Bluetooth.
4. Move the joystick to control the cursor.
5. Rotate the encoder to scroll.
6. Use buttons for left and right clicks.

---

## 👥 Team & Mentorship

This project was built collaboratively by students in the **Robotics Class at İTÜ MTAL**.
Under the **Ahi system**, I serve as a **Hami (mentor)** — teaching Arduino programming, circuit building, and debugging to my teammates.

> “Those who know should teach; those who learn should share.”
> — Ahi Principle applied to Robotics

---

## 🏆 Future Plans

This project is only the beginning.
In upcoming presentations, we plan to:

* Integrate gesture control and sensors,
* Create wireless input devices for accessibility,
* Develop smarter IoT-based control systems using ESP32.

---

## 📚 Author

**Hidayet Erdem**
Hami, Robotics Student at İTÜ MTAL
Guiding peers in Arduino, Electronics, and Robotics through the Ahi system.

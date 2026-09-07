# 🤖 DeskRobo — WiFi-Controlled Bipedal Desk Robot

A small biped robot controlled entirely over WiFi through a phone browser — no app install needed. It walks, turns, and stands using coordinated 4-servo gaits, avoids obstacles autonomously while walking forward, and is driven live through a touch-joystick web interface hosted directly on the robot itself.

---

## 🚗 Why this exists

Most hobby-robot controllers need a companion app or a separate remote. DeskRobo instead hosts its own WiFi access point and a full touch-joystick control page — so any phone or laptop can connect and drive it straight from a browser, with zero extra software.

---

## ⚙️ How it works

**1. Self-hosted WiFi control**
The ESP8266 creates its own access point (`DeskRobo_AP`) and serves a webpage with a **touch/mouse-driven joystick UI** (built with plain HTML/CSS/JS, no external libraries). Dragging the joystick calculates direction by angle (`atan2`) and sends the corresponding command (`FORWARD`, `BACKWARD`, `LEFT`, `RIGHT`, `STOP`) to the robot via a simple HTTP endpoint (`/cmd?dir=...`).

**2. Four-servo walking gait**
The robot walks using two hip servos (`legLeft`, `legRight`) and two ankle servos (`footLeft`, `footRight`), coordinated in a 6-step tilt-swing-flatten sequence per stride — shifting weight onto one foot, swinging the hips, then flattening and repeating on the other side. `walkBackward()` runs the same sequence in reverse.

**3. Turning via high-leg pivot**
Turning isn't just a simple lean — the robot lifts its legs to a wider stance (`turnLeftHighPivot` / `turnRightHighPivot`) before pivoting, giving a more stable and pronounced turn than a basic weight-shift would allow.

**4. Autonomous obstacle avoidance**
While walking forward, an ultrasonic sensor continuously checks the distance ahead. If an obstacle comes within **15cm**, the robot automatically stops and resets to home position — overriding the joystick command — so it won't walk itself into something even if the driver keeps pushing forward.

**5. Stand mode**
A dedicated `setStandMode()` sequence swings the hips outward and rotates the ankle servos to a fixed pose — useful as a resting/display stance, triggered with a single button on the web UI.

---

## 🔩 Hardware Used

| Component | Purpose |
|---|---|
| ESP8266 (NodeMCU-class board) | Main controller + WiFi host |
| 2× MG90S Servos | Hip joints |
| 2× SG90 Servos | Ankle joints |
| Ultrasonic Sensor (HC-SR04 type) | Obstacle detection while walking |

---

## 🕹️ Controls (Web UI)

| Control | Action |
|---|---|
| Joystick drag | Walk forward / backward / turn left / turn right |
| Release joystick | Stop |
| STAND MODE button | Move to fixed standing pose |
| EMERGENCY STOP button | Immediately halt all motion |

---

## 🧰 Tech Stack

- **Platform:** ESP8266 (Arduino framework)
- **Libraries:** `ESP8266WiFi.h`, `ESP8266WebServer.h`, `Servo.h`
- **Frontend:** Vanilla HTML/CSS/JS (served directly from the ESP8266, touch + mouse joystick)
- **Language:** C++ (Arduino) + embedded HTML/JS

---

## 🚀 Getting Started

1. Wire up the 4 servos and ultrasonic sensor as per the pin definitions in `DESK_ROBO.ino`.
2. Open the sketch in the Arduino IDE with ESP8266 board support installed.
3. Flash it to the ESP8266.
4. Connect your phone/laptop to the WiFi network `DeskRobo_AP` (password: `password123`).
5. Open a browser to the ESP8266's IP address and drive DeskRobo with the on-screen joystick.

---

## 🔭 Future Improvements

- Add live obstacle-distance readout to the web UI
- Replace fixed AP credentials with a configurable WiFi setup
- Add more gait patterns (side-step, diagonal walk)
- Add a battery-level indicator to the control page

---

## 👤 Author

**Prakhar Dixit**
Electronics & Communication Engineering (VLSI Design & Technology) — Zakir Husain College of Engineering & Technology, AMU
Recruitment & Workshop Coordinator and contributor, SAE ZHCET
[LinkedIn](https://www.linkedin.com/in/prakhar-dixit-a2bb95377)

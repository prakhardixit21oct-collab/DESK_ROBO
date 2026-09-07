#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

// Servo Objects
Servo legLeft;   // D1 (GPIO5)  - MG90S Hip
Servo legRight;  // D2 (GPIO4)  - MG90S Hip
Servo footLeft;  // D5 (GPIO14) - SG90 Ankle
Servo footRight; // D6 (GPIO12) - SG90 Ankle

// Ultrasonic Sensor Pins
const int TRIG_PIN = D7; // GPIO13
const int ECHO_PIN = D8; // GPIO15

String currentMode = "STOP";

ESP8266WebServer server(80);

// Function Declarations
long getDistance();
void setHomePos();
void setStandMode();
void walkForward();
void walkBackward();
void turnLeftHighPivot();
void turnRightHighPivot();

// HTML Web Controller Page with Touch Joystick
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background: #181825; color: #cdd6f4; margin: 0; padding: 20px; user-select: none; touch-action: none; }
    h1 { color: #89b4fa; margin-bottom: 10px; font-size: 24px; }
    #joystick-container { position: relative; width: 220px; height: 220px; background: #313244; border-radius: 50%; margin: 25px auto; border: 4px solid #45475a; box-shadow: 0 8px 20px rgba(0,0,0,0.5); }
    #joystick-knob { position: absolute; width: 80px; height: 80px; background: #89b4fa; border-radius: 50%; top: 70px; left: 70px; box-shadow: 0 4px 10px rgba(0,0,0,0.4); }
    .btn-container { margin-top: 20px; }
    .btn { display: inline-block; padding: 12px 24px; font-size: 16px; font-weight: bold; color: #11111b; border: none; border-radius: 12px; margin: 5px; text-decoration: none; cursor: pointer; }
    .btn-stand { background: #fab387; }
    .btn-stop { background: #f38ba8; }
    #status { font-size: 18px; font-weight: bold; color: #a6e3a1; margin-top: 10px; }
  </style>
</head>
<body>
  <h1>Desk Robo Joystick</h1>
  <div id="status">Status: STOP</div>
  
  <div id="joystick-container">
    <div id="joystick-knob"></div>
  </div>

  <div class="btn-container">
    <button class="btn btn-stand" onclick="sendCmd('STAND')">STAND MODE</button>
    <button class="btn btn-stop" onclick="sendCmd('STOP')">EMERGENCY STOP</button>
  </div>

  <script>
    const container = document.getElementById('joystick-container');
    const knob = document.getElementById('joystick-knob');
    const statusText = document.getElementById('status');
    let lastDir = "STOP";

    const maxRadius = 70;
    const centerX = 110;
    const centerY = 110;

    function handleMove(e) {
      const rect = container.getBoundingClientRect();
      const touch = e.touches ? e.touches[0] : e;
      let x = touch.clientX - rect.left - centerX;
      let y = touch.clientY - rect.top - centerY;

      let distance = Math.hypot(x, y);
      if (distance > maxRadius) {
        x = (x / distance) * maxRadius;
        y = (y / distance) * maxRadius;
      }

      knob.style.transform = `translate(${x}px, ${y}px)`;

      let dir = "STOP";
      if (distance > 25) {
        let angle = Math.atan2(y, x) * (180 / Math.PI);
        if (angle >= -45 && angle < 45) dir = "RIGHT";
        else if (angle >= 45 && angle < 135) dir = "BACKWARD";
        else if (angle >= -135 && angle < -45) dir = "FORWARD";
        else dir = "LEFT";
      }

      if (dir !== lastDir) {
        lastDir = dir;
        sendCmd(dir);
      }
    }

    function resetKnob() {
      knob.style.transform = `translate(0px, 0px)`;
      if (lastDir !== "STOP") {
        lastDir = "STOP";
        sendCmd("STOP");
      }
    }

    function sendCmd(cmd) {
      statusText.innerText = "Status: " + cmd;
      fetch('/cmd?dir=' + cmd);
    }

    container.addEventListener('touchstart', handleMove);
    container.addEventListener('touchmove', handleMove);
    container.addEventListener('touchend', resetKnob);
    container.addEventListener('mousedown', (e) => {
      handleMove(e);
      document.onmousemove = handleMove;
      document.onmouseup = () => {
        resetKnob();
        document.onmousemove = null;
        document.onmouseup = null;
      };
    });
  </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  
  legLeft.attach(D1);
  legRight.attach(D2);
  
  // Custom pulse limits (500us to 2500us) allow full 180° rotation for SG90 ankles
  footLeft.attach(D5, 500, 2500);
  footRight.attach(D6, 500, 2500);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  setHomePos();

  WiFi.softAP("DeskRobo_AP", "password123");
  Serial.println("AP Active");

  server.on("/", handleRoot);
  server.on("/cmd", []() {
    if (server.hasArg("dir")) {
      currentMode = server.arg("dir");
    }
    server.send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  server.handleClient();

  if (currentMode == "FORWARD") {
    long dist = getDistance();
    if (dist > 0 && dist < 15) { // Stop if obstacle is closer than 15cm
      setHomePos();
      currentMode = "STOP";
    } else {
      walkForward();
    }
  } 
  else if (currentMode == "BACKWARD") {
    walkBackward();
  } 
  else if (currentMode == "LEFT") {
    turnLeftHighPivot();
    currentMode = "FORWARD"; 
  } 
  else if (currentMode == "RIGHT") {
    turnRightHighPivot();
    currentMode = "FORWARD"; 
  }
  else if (currentMode == "STAND") {
    setStandMode();
    currentMode = "STOP"; // Lock stand mode position until next input
  }
}

// ---------------- POSITIONAL FUNCTIONS ----------------

void setHomePos() {
  footLeft.write(90);
  footRight.write(90);
  delay(150);
  legLeft.write(90);
  legRight.write(90);
}

// FIXED STAND MODE (Legs swing outward in opposite angle directions)
void setStandMode() {
  // Step 1: Swing Hip Servos outward (Left goes to 170°, Right goes to 10° for mirrored outward swing)
  legLeft.write(170);
  legRight.write(10);
  delay(350);

  // Step 2: Rotate Left Foot Servo (SG90) to full 180 degrees
  footLeft.write(180);
  delay(400);

  // Step 3: Rotate Right Foot Servo (SG90) to full 0 degrees
  footRight.write(0);
  delay(400);
}

// ---------------- GAITS ----------------

void walkForward() {
  // 1. Tilt onto Left Foot
  footLeft.write(128);
  footRight.write(128);
  delay(200);
  
  // 2. Swing Hips
  legLeft.write(120);
  legRight.write(120);
  delay(200);
  
  // 3. Flatten Feet
  footLeft.write(90);
  footRight.write(90);
  delay(180);
  
  // 4. Tilt onto Right Foot
  footLeft.write(52);
  footRight.write(52);
  delay(200);
  
  // 5. Swing Hips
  legLeft.write(60);
  legRight.write(60);
  delay(200);
  
  // 6. Flatten Feet
  footLeft.write(90);
  footRight.write(90);
  delay(180);
}

void walkBackward() {
  // Reverse sequence of walkForward
  footLeft.write(128);
  footRight.write(128);
  delay(200);
  
  legLeft.write(60);
  legRight.write(60);
  delay(200);
  
  footLeft.write(90);
  footRight.write(90);
  delay(180);
  
  footLeft.write(52);
  footRight.write(52);
  delay(200);
  
  legLeft.write(120);
  legRight.write(120);
  delay(200);
  
  footLeft.write(90);
  footRight.write(90);
  delay(180);
}

// --- HIGH-LEG PIVOT TURNS ---

void turnLeftHighPivot() {
  legLeft.write(140);
  legRight.write(40);
  delay(250);

  footLeft.write(125);
  footRight.write(125);
  legLeft.write(60);
  legRight.write(120);
  delay(300);

  setHomePos();
  delay(200);
}

void turnRightHighPivot() {
  legLeft.write(140);
  legRight.write(40);
  delay(250);

  footLeft.write(55);
  footRight.write(55);
  legLeft.write(120);
  legRight.write(60);
  delay(300);

  setHomePos();
  delay(200);
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}
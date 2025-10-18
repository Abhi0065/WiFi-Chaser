#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>

const char* ssid = "Abhishek";
const char* password = "@bhishek";

const int output1 = 27;
const int output2 = 26;
const int output3 = 25;
const int output4 = 33;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Animation state variables
int currentAnimation = -1; // -1 = off, 0-15 = animation modes
int animationSpeed = 500;
unsigned long lastUpdate = 0;
int animationStep = 0;
int chaserPosition = 0;

// HTML page stored in program memory
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8"/>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no"/>
  <title>LED Control</title>
  <style>
    * {
      -webkit-tap-highlight-color: transparent;
      touch-action: manipulation;
      user-select: none;
      padding: 0;
      margin: 0;
    }
    body {
      width: 100%;
      min-height: 100vh;
      font-family: Arial, sans-serif;
      background: linear-gradient(135deg, #1e1e1e 0%, #2d2d2d 100%);
      color: #f0f0f0;
      text-align: center;
      overflow-x: hidden;
      padding-bottom: 30px;
    }
    .container {
      max-width: 450px;
      width: 90%;
      margin: 20px auto;
      padding: 25px;
      background: rgba(51, 51, 51, 0.95);
      border-radius: 12px;
      box-shadow: 0 4px 20px rgba(0, 0, 0, 0.7);
    }
    h1 {
      margin-bottom: 20px;
      font-size: 28px;
      background: linear-gradient(45deg, #4ce70f, #31d513);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }
    .connection-status {
      padding: 10px;
      margin-bottom: 20px;
      border-radius: 8px;
      font-size: 13px;
      font-weight: bold;
      transition: all 0.3s;
    }
    .connected {
      color: #4adf10;
      border: 2px solid #4adf10;
    }
    .disconnected {
      background: rgba(219, 30, 30, 0.2);
      color: #db1e1e;
      border: 2px solid #db1e1e;
    }
    .led-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
      gap: 15px;
      margin: 20px 0;
    }
    .led-item {
      padding: 15px;
      border-radius: 10px;
      border: 2px solid #555;
      background: rgba(30, 30, 30, 0.6);
      transition: all 0.3s;
      cursor: pointer;
    }
    .led-item:hover {
      transform: translateY(-2px);
      box-shadow: 0 4px 15px rgba(78, 219, 22, 0.3);
    }
    .led-item.active {
      border-color: #0edd3a;
      box-shadow: 0 0 20px rgb(34 219 22 / 40%);
    }
    .led-title {
      font-size: 16px;
      font-weight: 600;
      margin-bottom: 5px;
      color: #fff;
    }
    .led-desc {
      font-size: 11px;
      color: #aaa;
      margin-bottom: 12px;
    }
    .toggle-switch {
      position: relative;
      width: 70px;
      height: 32px;
      margin: 15px auto;
      display: flex;
      justify-content: center;
      align-items: center;
    }
    .toggle-switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }
    .toggle-switch label {
      position: absolute;
      cursor: pointer;
      background-color: #db1e1e;
      border-radius: 20px;
      width: 100%;
      height: 100%;
      transition: background-color 0.3s;
      box-shadow: inset 0 2px 5px rgba(0,0,0,0.3);
    }
    .toggle-switch label:before {
      content: "";
      position: absolute;
      border-radius: 50%;
      background-color: white;
      width: 26px;
      height: 26px;
      transition: transform 0.3s;
      transform: translateX(3px);
      top: 3px;
      left: 0;
      box-shadow: 0 2px 5px rgba(0,0,0,0.3);
    }
    .toggle-switch input:checked + label {
      background-color: #4edb16;
    }
    .toggle-switch input:checked + label:before {
      transform: translateX(41px);
    }
    .status {
      font-size: 11px;
      letter-spacing: 1px;
      margin-top: 8px;
      font-weight: 500;
    }
    .speed-section {
      margin-top: 30px;
      padding: 20px;
      background: rgba(30, 30, 30, 0.6);
      border-radius: 10px;
      border: 2px solid #555;
    }
    .speed-title {
      font-size: 18px;
      font-weight: 600;
      margin-bottom: 15px;
      color: #fff;
    }
    input[type="range"] {
      -webkit-appearance: none;
      width: 90%;
      height: 12px;
      margin: 20px auto;
      background: linear-gradient(to right, #ff4444 0%, #ff8800 50%, #4edb16 100%);
      border-radius: 6px;
      outline: none;
      display: block;
      box-shadow: inset 0 2px 5px rgba(0,0,0,0.3);
    }
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 24px;
      height: 24px;
      background: linear-gradient(135deg, #fff, #ddd);
      border-radius: 50%;
      cursor: pointer;
      box-shadow: 0 3px 8px rgba(0,0,0,0.4);
      border: 2px solid #4edb16;
    }
    input[type="range"]::-webkit-slider-thumb:hover {
      transform: scale(1.1);
      box-shadow: 0 4px 12px rgba(78, 219, 22, 0.6);
    }
    .speed-display {
      font-size: 24px;
      font-weight: bold;
      color: #4edb16;
      margin-top: 10px;
    }
    footer {
      padding: 15px;
      font-size: 11px;
      color: #888;
      margin-top: 20px;
    }
    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.5; }
    }
    .led-item.active .led-title {
      animation: pulse 2s infinite;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🎨 LED Animation Control</h1>
    <div id="connection-status" class="connection-status disconnected">
      ● Disconnected
    </div>
    
    <div class="led-grid">
      <div class="led-item" id="led-container-0">
        <div class="led-title">All ON</div>
        <div class="led-desc">All LEDs steady on</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led0"/>
          <label for="led0"></label>
        </div>
        <div id="status0" class="status">OFF</div>
      </div>
      
      <div class="led-item" id="led-container-1">
        <div class="led-title">Chaser →</div>
        <div class="led-desc">Sequential left to right</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led1"/>
          <label for="led1"></label>
        </div>
        <div id="status1" class="status">OFF</div>
      </div>
      
      <div class="led-item" id="led-container-2">
        <div class="led-title">Chaser ←</div>
        <div class="led-desc">Sequential right to left</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led2"/>
          <label for="led2"></label>
        </div>
        <div id="status2" class="status">OFF</div>
      </div>
      
      <div class="led-item" id="led-container-3">
        <div class="led-title">Bounce</div>
        <div class="led-desc">Bouncing effect</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led3"/>
          <label for="led3"></label>
        </div>
        <div id="status3" class="status">OFF</div>
      </div>
      
      <div class="led-item" id="led-container-4">
        <div class="led-title">All Blink</div>
        <div class="led-desc">All together blink</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led4"/>
          <label for="led4"></label>
        </div>
        <div id="status4" class="status">OFF</div>
      </div>
      
      <div class="led-item" id="led-container-5">
        <div class="led-title">Pairs Alt</div>
        <div class="led-desc">Pairs alternating</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led5"/>
          <label for="led5"></label>
        </div>
        <div id="status5" class="status">OFF</div>
      </div>
      
      <div class="led-item" id="led-container-6">
        <div class="led-title">Wave</div>
        <div class="led-desc">Smooth wave pattern</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led6"/>
          <label for="led6"></label>
        </div>
        <div id="status6" class="status">OFF</div>
      </div>
      
      <div class="led-item" id="led-container-7">
        <div class="led-title">Theater</div>
        <div class="led-desc">Theater chase effect</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led7"/>
          <label for="led7"></label>
        </div>
        <div id="status7" class="status">OFF</div>
      </div>
      
      <div class="led-item" id="led-container-8">
        <div class="led-title">Split</div>
        <div class="led-desc">Inside to outside</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led8"/>
          <label for="led8"></label>
        </div>
        <div id="status8" class="status">OFF</div>
      </div>
      
      <div class="led-item" id="led-container-9">
        <div class="led-title">Double Chase</div>
        <div class="led-desc">Two chasers opposite</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led9"/>
          <label for="led9"></label>
        </div>
        <div id="status9" class="status">OFF</div>
      </div>
      
      <div class="led-item" id="led-container-10">
        <div class="led-title">Random</div>
        <div class="led-desc">Random flashing</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led10"/>
          <label for="led10"></label>
        </div>
        <div id="status10" class="status">OFF</div>
      </div>
      
      <div class="led-item" id="led-container-11">
        <div class="led-title">Binary Count</div>
        <div class="led-desc">Binary counter</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led11"/>
          <label for="led11"></label>
        </div>
        <div id="status11" class="status">OFF</div>
      </div>
      
      <div class="led-item" id="led-container-12">
        <div class="led-title">Pulse</div>
        <div class="led-desc">Breathing effect</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led12"/>
          <label for="led12"></label>
        </div>
        <div id="status12" class="status">OFF</div>
      </div>
      
      <div class="led-item" id="led-container-13">
        <div class="led-title">Knight Rider</div>
        <div class="led-desc">KITT scanner effect</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led13"/>
          <label for="led13"></label>
        </div>
        <div id="status13" class="status">OFF</div>
      </div>
      
      <div class="led-item" id="led-container-14">
        <div class="led-title">Strobe</div>
        <div class="led-desc">Fast strobe flash</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led14"/>
          <label for="led14"></label>
        </div>
        <div id="status14" class="status">OFF</div>
      </div>
      
      <div class="led-item" id="led-container-15">
        <div class="led-title">Cascade</div>
        <div class="led-desc">Waterfall effect</div>
        <div class="toggle-switch">
          <input type="checkbox" id="led15"/>
          <label for="led15"></label>
        </div>
        <div id="status15" class="status">OFF</div>
      </div>
    </div>
    
    <div class="speed-section">
      <div class="speed-title">⚡ Speed Control</div>
      <input type="range" min="1" max="11" value="6" id="speed"/>
      <div class="speed-display" id="status-speed">6</div>
    </div>
  </div>
  
  <footer>Made By Abhishek | ESP32 WebSocket Controller</footer>

  <script>
    let ws = null;
    const connectionStatus = document.getElementById('connection-status');
    
    function updateConnectionStatus(connected) {
      if (connected) {
        connectionStatus.textContent = '● Connected';
        connectionStatus.className = 'connection-status connected';
      } else {
        connectionStatus.textContent = '● Disconnected';
        connectionStatus.className = 'connection-status disconnected';
      }
    }
    
    function connectWebSocket() {
      ws = new WebSocket('ws://' + window.location.hostname + ':81');
      
      ws.onopen = () => {
        console.log('WebSocket connected');
        updateConnectionStatus(true);
        restorePreviousState();
      };
      
      ws.onclose = () => {
        console.log('WebSocket disconnected');
        updateConnectionStatus(false);
        setTimeout(connectWebSocket, 3000);
      };
      
      ws.onerror = (error) => {
        console.error('WebSocket error:', error);
        updateConnectionStatus(false);
      };
    }
    
    connectWebSocket();
    
    function sendCommand(command) {
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(command);
      }
    }
    
    function updateActiveState(ledNumber) {
      for (let i = 0; i <= 15; i++) {
        const container = document.getElementById('led-container-' + i);
        if (container) {
          container.classList.remove('active');
        }
      }
      if (ledNumber >= 0) {
        const container = document.getElementById('led-container-' + ledNumber);
        if (container) {
          container.classList.add('active');
        }
      }
    }
    
    function initLED(ledNumber) {
      const toggle = document.getElementById('led' + ledNumber);
      const status = document.getElementById('status' + ledNumber);
      let isOn = sessionStorage.getItem('ledState' + ledNumber) === 'true';
      
      if (isOn) {
        toggle.checked = true;
        status.innerText = 'ON';
        updateActiveState(ledNumber);
      }
      
      toggle.addEventListener('change', () => {
        if (toggle.checked) {
          for (let i = 0; i <= 15; i++) {
            const otherToggle = document.getElementById('led' + i);
            const otherStatus = document.getElementById('status' + i);
            if (i !== ledNumber) {
              otherToggle.checked = false;
              otherStatus.innerText = 'OFF';
              sessionStorage.setItem('ledState' + i, 'false');
            }
          }
          status.innerText = 'ON';
          sessionStorage.setItem('ledState' + ledNumber, 'true');
          updateActiveState(ledNumber);
          sendCommand('ani' + ledNumber);
        } else {
          status.innerText = 'OFF';
          sessionStorage.setItem('ledState' + ledNumber, 'false');
          updateActiveState(-1);
          sendCommand('off');
        }
      });
    }
    
    for (let i = 0; i <= 15; i++) {
      initLED(i);
    }
    
    const speed = document.getElementById('speed');
    const speedStatus = document.getElementById('status-speed');
    
    const prevSpeed = sessionStorage.getItem('speed');
    if (prevSpeed) {
      speed.value = prevSpeed;
      speedStatus.innerText = prevSpeed;
    }
    
    speed.addEventListener('input', () => {
      speedStatus.innerText = speed.value;
    });
    
    speed.addEventListener('change', () => {
      const speedValue = parseInt(speed.value);
      sessionStorage.setItem('speed', speedValue);
      sendCommand('speed' + speedValue);
    });
    
    function restorePreviousState() {
      for (let i = 0; i <= 15; i++) {
        const isOn = sessionStorage.getItem('ledState' + i) === 'true';
        if (isOn) {
          sendCommand('ani' + i);
          updateActiveState(i);
          break;
        }
      }
      const prevSpeed = sessionStorage.getItem('speed');
      if (prevSpeed) {
        sendCommand('speed' + prevSpeed);
      }
    }
  </script>
</body>
</html>
)rawliteral";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
      
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      break;
    }
    
    case WStype_TEXT: {
      String message = String((char*)payload);
      
      if (message.startsWith("off")) {
        currentAnimation = -1;
        digitalWrite(output1, HIGH);
        digitalWrite(output2, HIGH);
        digitalWrite(output3, HIGH);
        digitalWrite(output4, HIGH);
      }
      else if (message.startsWith("ani")) {
        int aniNum = message.substring(3).toInt();
        currentAnimation = aniNum;
        animationStep = 0;
        chaserPosition = 0;
      }
      else if (message.startsWith("speed")) {
        int speedLevel = message.substring(5).toInt();
        animationSpeed = 1100 - (speedLevel * 100);
        if (animationSpeed < 50) animationSpeed = 50;
      }
      break;
    }
  }
}

void runAnimation() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastUpdate >= animationSpeed) {
    lastUpdate = currentMillis;
    
    switch(currentAnimation) {
      case -1: // OFF
        digitalWrite(output1, HIGH);
        digitalWrite(output2, HIGH);
        digitalWrite(output3, HIGH);
        digitalWrite(output4, HIGH);
        break;
        
      case 0: // All ON
        digitalWrite(output1, LOW);
        digitalWrite(output2, LOW);
        digitalWrite(output3, LOW);
        digitalWrite(output4, LOW);
        break;
        
      case 1: // Chaser Right (1->2->3->4)
        digitalWrite(output1, chaserPosition == 0 ? LOW : HIGH);
        digitalWrite(output2, chaserPosition == 1 ? LOW : HIGH);
        digitalWrite(output3, chaserPosition == 2 ? LOW : HIGH);
        digitalWrite(output4, chaserPosition == 3 ? LOW : HIGH);
        chaserPosition = (chaserPosition + 1) % 4;
        break;
        
      case 2: // Chaser Left (4->3->2->1)
        digitalWrite(output1, chaserPosition == 3 ? LOW : HIGH);
        digitalWrite(output2, chaserPosition == 2 ? LOW : HIGH);
        digitalWrite(output3, chaserPosition == 1 ? LOW : HIGH);
        digitalWrite(output4, chaserPosition == 0 ? LOW : HIGH);
        chaserPosition = (chaserPosition + 1) % 4;
        break;
        
      case 3: // Bounce (1->2->3->4->3->2->1)
        {
          int bouncePattern[] = {0, 1, 2, 3, 2, 1};
          int pos = bouncePattern[animationStep % 6];
          digitalWrite(output1, pos == 0 ? LOW : HIGH);
          digitalWrite(output2, pos == 1 ? LOW : HIGH);
          digitalWrite(output3, pos == 2 ? LOW : HIGH);
          digitalWrite(output4, pos == 3 ? LOW : HIGH);
          animationStep++;
        }
        break;
        
      case 4: // All Blink
        if (animationStep % 2 == 0) {
          digitalWrite(output1, LOW);
          digitalWrite(output2, LOW);
          digitalWrite(output3, LOW);
          digitalWrite(output4, LOW);
        } else {
          digitalWrite(output1, HIGH);
          digitalWrite(output2, HIGH);
          digitalWrite(output3, HIGH);
          digitalWrite(output4, HIGH);
        }
        animationStep++;
        break;
        
      case 5: // Pairs Alternating (1,2 vs 3,4)
        if (animationStep % 2 == 0) {
          digitalWrite(output1, LOW);
          digitalWrite(output2, LOW);
          digitalWrite(output3, HIGH);
          digitalWrite(output4, HIGH);
        } else {
          digitalWrite(output1, HIGH);
          digitalWrite(output2, HIGH);
          digitalWrite(output3, LOW);
          digitalWrite(output4, LOW);
        }
        animationStep++;
        break;
        
      case 6: // Wave (smooth transition)
        {
          int wavePattern[] = {1,1,1,1, 1,1,1,0, 1,1,0,0, 1,0,0,0, 
                               0,0,0,0, 0,0,0,1, 0,0,1,1, 0,1,1,1};
          int idx = (animationStep % 8) * 4;
          digitalWrite(output1, wavePattern[idx] ? HIGH : LOW);
          digitalWrite(output2, wavePattern[idx+1] ? HIGH : LOW);
          digitalWrite(output3, wavePattern[idx+2] ? HIGH : LOW);
          digitalWrite(output4, wavePattern[idx+3] ? HIGH : LOW);
          animationStep++;
        }
        break;
        
      case 7: // Theater Chase (every 2nd LED)
        {
          int theater = animationStep % 3;
          digitalWrite(output1, (theater == 0 || theater == 2) ? LOW : HIGH);
          digitalWrite(output2, (theater == 1) ? LOW : HIGH);
          digitalWrite(output3, (theater == 0 || theater == 2) ? LOW : HIGH);
          digitalWrite(output4, (theater == 1) ? LOW : HIGH);
          animationStep++;
        }
        break;
        
      case 8: // Split (Inside to Outside)
        {
          int splitPattern[][4] = {
            {1,1,1,1}, {0,1,1,0}, {1,0,0,1}, {0,0,0,0}
          };
          int idx = animationStep % 4;
          digitalWrite(output1, splitPattern[idx][0] ? HIGH : LOW);
          digitalWrite(output2, splitPattern[idx][1] ? HIGH : LOW);
          digitalWrite(output3, splitPattern[idx][2] ? HIGH : LOW);
          digitalWrite(output4, splitPattern[idx][3] ? HIGH : LOW);
          animationStep++;
        }
        break;
        
      case 9: // Double Chase (opposite directions)
        {
          bool led1 = (chaserPosition == 0 || chaserPosition == 2);
          bool led4 = (chaserPosition == 0 || chaserPosition == 2);
          digitalWrite(output1, led1 ? LOW : HIGH);
          digitalWrite(output2, !led1 ? LOW : HIGH);
          digitalWrite(output3, !led4 ? LOW : HIGH);
          digitalWrite(output4, led4 ? LOW : HIGH);
          chaserPosition = (chaserPosition + 1) % 4;
        }
        break;
        
      case 10: // Random
        digitalWrite(output1, random(0, 2) ? LOW : HIGH);
        digitalWrite(output2, random(0, 2) ? LOW : HIGH);
        digitalWrite(output3, random(0, 2) ? LOW : HIGH);
        digitalWrite(output4, random(0, 2) ? LOW : HIGH);
        break;
        
      case 11: // Binary Counter (0-15)
        {
          int count = animationStep % 16;
          digitalWrite(output1, (count & 0x01) ? LOW : HIGH);
          digitalWrite(output2, (count & 0x02) ? LOW : HIGH);
          digitalWrite(output3, (count & 0x04) ? LOW : HIGH);
          digitalWrite(output4, (count & 0x08) ? LOW : HIGH);
          animationStep++;
        }
        break;
        
      case 12: // Pulse (breathing effect simulation)
        {
          int pulsePattern[] = {1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1};
          int idx = animationStep % 16;
          if (pulsePattern[idx]) {
            digitalWrite(output1, LOW);
            digitalWrite(output2, LOW);
            digitalWrite(output3, LOW);
            digitalWrite(output4, LOW);
          } else {
            digitalWrite(output1, HIGH);
            digitalWrite(output2, HIGH);
            digitalWrite(output3, HIGH);
            digitalWrite(output4, HIGH);
          }
          animationStep++;
        }
        break;
        
      case 13: // Knight Rider (KITT scanner)
        {
          int kitt[] = {0, 1, 2, 3, 2, 1};
          int pos = kitt[animationStep % 6];
          digitalWrite(output1, pos == 0 ? LOW : HIGH);
          digitalWrite(output2, pos == 1 ? LOW : HIGH);
          digitalWrite(output3, pos == 2 ? LOW : HIGH);
          digitalWrite(output4, pos == 3 ? LOW : HIGH);
          animationStep++;
        }
        break;
        
      case 14: // Strobe
        if (animationStep % 4 < 2) {
          digitalWrite(output1, LOW);
          digitalWrite(output2, LOW);
          digitalWrite(output3, LOW);
          digitalWrite(output4, LOW);
        } else {
          digitalWrite(output1, HIGH);
          digitalWrite(output2, HIGH);
          digitalWrite(output3, HIGH);
          digitalWrite(output4, HIGH);
        }
        animationStep++;
        break;
        
      case 15: // Cascade (waterfall)
        {
          int cascade[][4] = {
            {0,1,1,1}, {0,0,1,1}, {0,0,0,1}, {0,0,0,0},
            {1,0,0,0}, {1,1,0,0}, {1,1,1,0}, {1,1,1,1}
          };
          int idx = animationStep % 8;
          digitalWrite(output1, cascade[idx][0] ? HIGH : LOW);
          digitalWrite(output2, cascade[idx][1] ? HIGH : LOW);
          digitalWrite(output3, cascade[idx][2] ? HIGH : LOW);
          digitalWrite(output4, cascade[idx][3] ? HIGH : LOW);
          animationStep++;
        }
        break;
    }
  }
}

void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(output1, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(output3, OUTPUT);
  pinMode(output4, OUTPUT);
  digitalWrite(output1, HIGH);
  digitalWrite(output2, HIGH);
  digitalWrite(output3, HIGH);
  digitalWrite(output4, HIGH);

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.disconnect();
  delay(100);
  
  // Set WiFi mode
  WiFi.mode(WIFI_STA);
  delay(100);
  
  // Begin connection
  WiFi.begin(ssid, password);
  
  // Wait for connection with timeout
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    // Try reconnecting every 10 attempts
    if (attempts % 10 == 0) {
      Serial.println("\nRetrying connection...");
      WiFi.disconnect();
      delay(100);
      WiFi.begin(ssid, password);
    }
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n\nWiFi Connection FAILED!");
    delay(5000);
    ESP.restart();
  }
  
  Serial.println("\n\n✅ WiFi Connected Successfully!");

  IPAddress gatewayIp = WiFi.gatewayIP();
  
  Serial.print("Gateway IP: ");
  Serial.println(gatewayIp);
  Serial.print("Current IP (DHCP): ");
  Serial.println(WiFi.localIP());
  
  // Configure static IP
  IPAddress local_IP(192, 168, gatewayIp[2], 50);
  IPAddress gateway(192, 168, gatewayIp[2], gatewayIp[3]);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(8, 8, 8, 8);
  IPAddress secondaryDNS(8, 8, 4, 4);

  Serial.println("\nSetting Static IP...");
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Static IP configuration failed");
  } else {
    Serial.println("Static IP configured");
    delay(100);
  }

  Serial.println("\n=== Network Configuration ===");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Gateway: ");
  Serial.print(WiFi.gatewayIP());

  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  // Start HTTP server
  server.on("/", handleRoot);
  server.begin();
  Serial.print("http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  webSocket.loop();
  server.handleClient();
  runAnimation();
}
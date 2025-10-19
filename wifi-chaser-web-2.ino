#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>

const char* ssid = "Abhishek";
const char* password = "@bhishek";

const int output1 = 12;
const int output2 = 14;
const int output3 = 27;
const int output4 = 26;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

int currentAnimation = -1;
int selectedAnimation = 0;
bool isPowerOn = false;
int animationSpeed = 500;
unsigned long lastUpdate = 0;
int animationStep = 0;
int chaserPosition = 0;

uint8_t ledStates[4] = {HIGH, HIGH, HIGH, HIGH};

unsigned long lastStateBroadcast = 0;
const unsigned long STATE_BROADCAST_INTERVAL = 100;
bool stateChanged = false;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8"/>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no"/>
  <title>LED Animation Control</title>
  <style>
    * {
      -webkit-tap-highlight-color: transparent;
      touch-action: manipulation;
      user-select: none;
      padding: 0;
      margin: 0;
      box-sizing: border-box;
    }
    
    body {
      width: 100%;
      height: 100vh;
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: #05040f;
      color: #f0f0f0;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 10px;
      overflow: hidden;
    }
    
    .main-container {
      width: 100%;
      max-width: 550px;
      max-height: 100%;
      background: rgba(30, 30, 45, 0.95);
      border-radius: 20px;
      padding: 20px;
      box-shadow: 0 20px 60px rgba(0, 0, 0, 0.5), 
                  0 0 100px rgba(102, 126, 234, 0.1),
                  inset 0 1px 1px rgba(255, 255, 255, 0.1);
      backdrop-filter: blur(10px);
      border: 1px solid rgba(255, 255, 255, 0.1);
      animation: slideIn 0.6s ease-out;
      overflow-y: auto;
      overflow-x: hidden;
    }
    
    .main-container::-webkit-scrollbar {
      width: 6px;
    }
    
    .main-container::-webkit-scrollbar-track {
      background: rgba(255, 255, 255, 0.05);
      border-radius: 10px;
    }
    
    .main-container::-webkit-scrollbar-thumb {
      background: rgba(102, 126, 234, 0.5);
      border-radius: 10px;
    }
    
    @keyframes slideIn {
      from {
        opacity: 0;
        transform: translateY(30px);
      }
      to {
        opacity: 1;
        transform: translateY(0);
      }
    }
    
    .header {
      text-align: center;
      margin-bottom: 20px;
    }
    
    h1 {
      font-size: 26px;
      font-weight: 700;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      margin-bottom: 6px;
      letter-spacing: -0.5px;
    }
    
    .connection-status {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      padding: 6px 14px;
      border-radius: 20px;
      font-size: 11px;
      font-weight: 600;
      margin-top: 10px;
      transition: all 0.3s;
      border: 1.5px solid;
    }
    
    .connection-status::before {
      content: '';
      width: 8px;
      height: 8px;
      border-radius: 50%;
      animation: pulse 2s infinite;
    }
    
    .connected {
      background: rgba(16, 185, 129, 0.15);
      color: #10b981;
      border-color: #10b981;
    }
    
    .connected::before {
      background: #10b981;
    }
    
    .disconnected {
      background: rgba(239, 68, 68, 0.15);
      color: #ef4444;
      border-color: #ef4444;
    }
    
    .disconnected::before {
      background: #ef4444;
    }
    
    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.3; }
    }
    
    .control-section {
      background: rgba(20, 20, 35, 0.6);
      border-radius: 14px;
      padding: 18px;
      margin-bottom: 15px;
      border: 1px solid rgba(255, 255, 255, 0.05);
    }
    
    .section-title {
      font-size: 14px;
      font-weight: 600;
      color: #e0e0e0;
      margin-bottom: 14px;
      display: flex;
      align-items: center;
      gap: 8px;
    }
    
    .section-title::before {
      content: '';
      width: 3px;
      height: 16px;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      border-radius: 2px;
    }
    
    .select-wrapper {
      position: relative;
      margin-bottom: 14px;
    }
    
    select {
      width: 100%;
      padding: 12px 40px 12px 16px;
      background: rgba(30, 30, 45, 0.8);
      border: 2px solid rgba(255, 255, 255, 0.1);
      border-radius: 10px;
      color: #f0f0f0;
      font-size: 14px;
      font-weight: 500;
      cursor: pointer;
      transition: all 0.3s ease;
      appearance: none;
      outline: none;
    }
    
    select:hover {
      border-color: rgba(102, 126, 234, 0.5);
      background: rgba(30, 30, 45, 0.95);
    }
    
    select:focus {
      border-color: #667eea;
      box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.15);
    }
    
    .select-wrapper::after {
      content: '▼';
      position: absolute;
      right: 20px;
      top: 50%;
      transform: translateY(-50%);
      color: #667eea;
      pointer-events: none;
      font-size: 12px;
    }
    
    option {
      background: #1a1a2e;
      color: #f0f0f0;
      padding: 12px;
    }
    
    .animation-preview {
      background: linear-gradient(135deg, rgba(102, 126, 234, 0.1) 0%, rgba(118, 75, 162, 0.1) 100%);
      border: 2px solid rgba(102, 126, 234, 0.3);
      border-radius: 10px;
      padding: 14px;
      text-align: center;
      margin-bottom: 14px;
      min-height: 90px;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      gap: 10px;
    }
    
    .preview-title {
      font-size: 15px;
      font-weight: 600;
      color: #667eea;
    }
    
    .led-visual {
      display: flex;
      gap: 10px;
      justify-content: center;
      align-items: center;
    }
    
    .led-dot {
      width: 32px;
      height: 32px;
      border-radius: 50%;
      background: rgba(255, 255, 255, 0.1);
      border: 2px solid rgba(255, 255, 255, 0.2);
      transition: all 0.3s ease;
      position: relative;
      overflow: hidden;
    }
    
    .led-dot::before {
      content: '';
      position: absolute;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      width: 0;
      height: 0;
      background: radial-gradient(circle, rgba(102, 126, 234, 0.8) 0%, transparent 70%);
      border-radius: 50%;
      transition: all 0.3s ease;
    }
    
    .led-dot.active {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      border-color: #667eea;
      box-shadow: 0 0 20px rgba(102, 126, 234, 0.6),
                  0 0 40px rgba(102, 126, 234, 0.3),
                  inset 0 0 10px rgba(255, 255, 255, 0.3);
      animation: glow 1.5s infinite;
    }
    
    .led-dot.active::before {
      width: 100%;
      height: 100%;
    }
    
    @keyframes glow {
      0%, 100% {
        box-shadow: 0 0 20px rgba(102, 126, 234, 0.6),
                    0 0 40px rgba(102, 126, 234, 0.3),
                    inset 0 0 10px rgba(255, 255, 255, 0.3);
      }
      50% {
        box-shadow: 0 0 30px rgba(102, 126, 234, 0.8),
                    0 0 60px rgba(102, 126, 234, 0.5),
                    inset 0 0 15px rgba(255, 255, 255, 0.5);
      }
    }
    
    .toggle-control {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 12px 16px;
      background: rgba(30, 30, 45, 0.5);
      border-radius: 10px;
      border: 1px solid rgba(255, 255, 255, 0.05);
      transition: opacity 0.3s;
    }
    
    .toggle-control.disabled {
      opacity: 0.4;
      pointer-events: none;
    }
    
    .toggle-label {
      font-size: 14px;
      font-weight: 500;
      color: #d0d0d0;
    }
    
    .toggle-switch {
      position: relative;
      width: 54px;
      height: 28px;
    }
    
    .toggle-switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }
    
    .toggle-slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background: rgba(239, 68, 68, 0.3);
      border: 2px solid #ef4444;
      border-radius: 34px;
      transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1);
    }
    
    .toggle-slider:before {
      position: absolute;
      content: "";
      height: 20px;
      width: 20px;
      left: 3px;
      bottom: 2px;
      background: white;
      border-radius: 50%;
      transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1);
      box-shadow: 0 2px 6px rgba(0, 0, 0, 0.3);
    }
    
    .toggle-switch input:checked + .toggle-slider {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      border-color: #667eea;
    }
    
    .toggle-switch input:checked + .toggle-slider:before {
      transform: translateX(26px);
      box-shadow: 0 2px 8px rgba(102, 126, 234, 0.5);
    }
    
    .slider-control {
      padding: 14px;
    }
    
    .slider-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 14px;
    }
    
    .speed-value {
      font-size: 20px;
      font-weight: 700;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      min-width: 35px;
      text-align: right;
    }
    
    input[type="range"] {
      -webkit-appearance: none;
      width: 100%;
      height: 8px;
      background: linear-gradient(to right, 
                                  #ef4444 0%, 
                                  #f59e0b 25%, 
                                  #10b981 50%, 
                                  #3b82f6 75%, 
                                  #8b5cf6 100%);
      border-radius: 8px;
      outline: none;
      margin: 10px 0;
      box-shadow: inset 0 1px 3px rgba(0, 0, 0, 0.3);
    }
    
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 20px;
      height: 20px;
      background: linear-gradient(135deg, #ffffff 0%, #e0e0e0 100%);
      border: 2px solid #667eea;
      border-radius: 50%;
      cursor: pointer;
      box-shadow: 0 3px 10px rgba(102, 126, 234, 0.4),
                  0 0 0 3px rgba(102, 126, 234, 0.1);
      transition: all 0.3s ease;
    }
    
    input[type="range"]::-webkit-slider-thumb:hover {
      transform: scale(1.1);
      box-shadow: 0 4px 12px rgba(102, 126, 234, 0.6),
                  0 0 0 5px rgba(102, 126, 234, 0.15);
    }
    
    input[type="range"]::-webkit-slider-thumb:active {
      transform: scale(1.05);
    }
    
    input[type="range"]::-moz-range-thumb {
      width: 20px;
      height: 20px;
      background: linear-gradient(135deg, #ffffff 0%, #e0e0e0 100%);
      border: 2px solid #667eea;
      border-radius: 50%;
      cursor: pointer;
      box-shadow: 0 3px 10px rgba(102, 126, 234, 0.4);
      transition: all 0.3s ease;
    }
    
    .speed-labels {
      display: flex;
      justify-content: space-between;
      font-size: 10px;
      color: #888;
      margin-top: 6px;
    }
    
    footer {
      text-align: center;
      margin-top: 15px;
      padding-top: 15px;
      border-top: 1px solid rgba(255, 255, 255, 0.1);
      font-size: 11px;
      color: #888;
    }
    
    footer a {
      color: #667eea;
      text-decoration: none;
      font-weight: 600;
      transition: color 0.3s;
    }
    
    footer a:hover {
      color: #764ba2;
    }
    
    @media (max-width: 480px) {
      body {
        padding: 5px;
      }
      
      .main-container {
        padding: 15px;
        border-radius: 16px;
      }
      
      h1 {
        font-size: 22px;
      }
      
      .led-dot {
        width: 28px;
        height: 28px;
      }
    }
  </style>
</head>
<body>
  <div class="main-container">
    <div class="header">
      <h1>⚡ LED Control Hub</h1>
      <div id="connection-status" class="connection-status disconnected">
        Disconnected
      </div>
    </div>
    
    <div class="control-section">
      <div class="section-title">Animation Mode</div>
      
      <div class="select-wrapper">
        <select id="animation-select">
          <option value="0">💡 All ON</option>
          <option value="1">➡️ Chaser Right</option>
          <option value="2">⬅️ Chaser Left</option>
          <option value="3">↔️ Bounce</option>
          <option value="4">✨ All Blink</option>
          <option value="5">🔄 Pairs Alternating</option>
          <option value="6">🌊 Wave</option>
          <option value="7">🎭 Theater Chase</option>
          <option value="8">⚡ Split</option>
          <option value="9">🔁 Double Chase</option>
          <option value="10">🎲 Random</option>
          <option value="11">🔢 Binary Counter</option>
          <option value="12">💫 Pulse Breathing</option>
          <option value="13">🚗 Knight Rider</option>
          <option value="14">⚠️ Strobe</option>
          <option value="15">🌈 Cascade</option>
        </select>
      </div>
      
      <div class="animation-preview">
        <div class="preview-title" id="preview-title">All ON</div>
        <div class="led-visual">
          <div class="led-dot" id="led1"></div>
          <div class="led-dot" id="led2"></div>
          <div class="led-dot" id="led3"></div>
          <div class="led-dot" id="led4"></div>
        </div>
      </div>
      
      <div class="toggle-control" id="toggle-control">
        <span class="toggle-label">Power</span>
        <div class="toggle-switch">
          <input type="checkbox" id="power-toggle"/>
          <label class="toggle-slider" for="power-toggle"></label>
        </div>
      </div>
    </div>
    
    <div class="control-section">
      <div class="section-title">Speed Control</div>
      <div class="slider-control">
        <div class="slider-header">
          <span style="font-size: 14px; color: #d0d0d0;">Speed Level</span>
          <span class="speed-value" id="speed-value">6</span>
        </div>
        <input type="range" min="1" max="11" value="6" id="speed-slider"/>
        <div class="speed-labels">
          <span>Slow</span>
          <span>Fast</span>
        </div>
      </div>
    </div>
    
    <footer>
      Made with 💜 by <a href="#">Abhishek</a>
    </footer>
  </div>

  <script>
    let ws = null;
    let isUpdatingFromServer = false;
    let speedDebounceTimer = null;
    let previewInterval = null;
    
    const connectionStatus = document.getElementById('connection-status');
    const animationSelect = document.getElementById('animation-select');
    const powerToggle = document.getElementById('power-toggle');
    const speedSlider = document.getElementById('speed-slider');
    const speedValue = document.getElementById('speed-value');
    const previewTitle = document.getElementById('preview-title');
    const toggleControl = document.getElementById('toggle-control');
    const ledDots = [
      document.getElementById('led1'),
      document.getElementById('led2'),
      document.getElementById('led3'),
      document.getElementById('led4')
    ];
    
    const animationNames = {
      '0': 'All ON', '1': 'Chaser Right', '2': 'Chaser Left',
      '3': 'Bounce', '4': 'All Blink', '5': 'Pairs Alternating', '6': 'Wave',
      '7': 'Theater Chase', '8': 'Split', '9': 'Double Chase', '10': 'Random',
      '11': 'Binary Counter', '12': 'Pulse Breathing', '13': 'Knight Rider',
      '14': 'Strobe', '15': 'Cascade'
    };
    
    function updateConnectionStatus(connected) {
      connectionStatus.textContent = connected ? 'Connected' : 'Disconnected';
      connectionStatus.className = 'connection-status ' + (connected ? 'connected' : 'disconnected');
    }
    
    function connectWebSocket() {
      ws = new WebSocket('ws://' + window.location.hostname + ':81');
      
      ws.onopen = () => {
        updateConnectionStatus(true);
      };
      
      ws.onclose = () => {
        updateConnectionStatus(false);
        setTimeout(connectWebSocket, 3000);
      };
      
      ws.onerror = (error) => {
        console.error('WebSocket error:', error);
        updateConnectionStatus(false);
      };
      
      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          isUpdatingFromServer = true;
          
          if (data.animation !== undefined) {
            animationSelect.value = data.animation;
            previewTitle.textContent = animationNames[data.animation];
            
            if (data.power !== undefined) {
              powerToggle.checked = data.power;
            }
            
            if (data.power) {
              startPreviewAnimation(data.animation);
            } else {
              startPreviewAnimation('-1');
            }
          }
          
          if (data.speed !== undefined) {
            speedSlider.value = data.speed;
            speedValue.textContent = data.speed;
            const currentMode = animationSelect.value;
            if (powerToggle.checked && currentMode !== '0') {
              startPreviewAnimation(currentMode);
            }
          }
          
          setTimeout(() => { isUpdatingFromServer = false; }, 50);
        } catch (e) {
          console.error('Parse error:', e);
        }
      };
    }
    
    connectWebSocket();
    
    function sendCommand(command) {
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(command);
      }
    }
    
    function updateLEDPreview(pattern) {
      requestAnimationFrame(() => {
        ledDots.forEach((dot, index) => {
          if (pattern[index]) {
            dot.classList.add('active');
          } else {
            dot.classList.remove('active');
          }
        });
      });
    }
    
    function startPreviewAnimation(mode) {
      if (previewInterval) clearInterval(previewInterval);
      
      let step = 0;
      const speedLevel = parseInt(speedSlider.value);
      const animSpeed = 1100 - (speedLevel * 100);
      const finalSpeed = animSpeed < 50 ? 50 : animSpeed;
      
      const animations = {
        '-1': () => updateLEDPreview([0, 0, 0, 0]),
        '0': () => updateLEDPreview([1, 1, 1, 1]),
        '1': () => {
          const pos = step % 4;
          updateLEDPreview([pos === 0, pos === 1, pos === 2, pos === 3]);
          step++;
        },
        '2': () => {
          const pos = step % 4;
          updateLEDPreview([pos === 3, pos === 2, pos === 1, pos === 0]);
          step++;
        },
        '3': () => {
          const pattern = [0, 1, 2, 3, 2, 1];
          const pos = pattern[step % 6];
          updateLEDPreview([pos === 0, pos === 1, pos === 2, pos === 3]);
          step++;
        },
        '4': () => {
          updateLEDPreview(step % 2 === 0 ? [1, 1, 1, 1] : [0, 0, 0, 0]);
          step++;
        },
        '5': () => {
          updateLEDPreview(step % 2 === 0 ? [1, 1, 0, 0] : [0, 0, 1, 1]);
          step++;
        },
        '6': () => {
          const patterns = [
            [1, 1, 1, 1], [1, 1, 1, 0], [1, 1, 0, 0], [1, 0, 0, 0],
            [0, 0, 0, 0], [0, 0, 0, 1], [0, 0, 1, 1], [0, 1, 1, 1]
          ];
          updateLEDPreview(patterns[step % 8]);
          step++;
        },
        '7': () => {
          const theater = step % 3;
          updateLEDPreview([
            theater === 0 || theater === 2,
            theater === 1,
            theater === 0 || theater === 2,
            theater === 1
          ]);
          step++;
        },
        '8': () => {
          const patterns = [[0, 0, 0, 0], [1, 0, 0, 1], [0, 1, 1, 0], [1, 1, 1, 1]];
          updateLEDPreview(patterns[step % 4]);
          step++;
        },
        '9': () => {
          updateLEDPreview([
            step % 2 === 0,
            step % 2 !== 0,
            step % 2 !== 0,
            step % 2 === 0
          ]);
          step++;
        },
        '10': () => {
          updateLEDPreview([
            Math.random() > 0.5,
            Math.random() > 0.5,
            Math.random() > 0.5,
            Math.random() > 0.5
          ]);
        },
        '11': () => {
          const count = step % 16;
          updateLEDPreview([
            !!(count & 0x01),
            !!(count & 0x02),
            !!(count & 0x04),
            !!(count & 0x08)
          ]);
          step++;
        },
        '12': () => {
          const pulsePattern = [1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1];
          const on = pulsePattern[step % 16];
          updateLEDPreview([on, on, on, on]);
          step++;
        },
        '13': () => {
          const pattern = [0, 1, 2, 3, 2, 1];
          const pos = pattern[step % 6];
          updateLEDPreview([pos === 0, pos === 1, pos === 2, pos === 3]);
          step++;
        },
        '14': () => {
          const on = step % 4 < 2;
          updateLEDPreview([on, on, on, on]);
          step++;
        },
        '15': () => {
          const patterns = [
            [1, 0, 0, 0], [1, 1, 0, 0], [1, 1, 1, 0], [1, 1, 1, 1],
            [0, 1, 1, 1], [0, 0, 1, 1], [0, 0, 0, 1], [0, 0, 0, 0]
          ];
          updateLEDPreview(patterns[step % 8]);
          step++;
        }
      };
      
      if (animations[mode]) {
        animations[mode]();
        if (mode !== '-1' && mode !== '0') {
          previewInterval = setInterval(() => animations[mode](), finalSpeed);
        }
      }
    }
    
    animationSelect.addEventListener('change', () => {
      if (isUpdatingFromServer) return;
      
      const value = animationSelect.value;
      previewTitle.textContent = animationNames[value];
      
      sendCommand('select' + value);
      
      if (powerToggle.checked) {
        sendCommand('ani' + value);
        startPreviewAnimation(value);
      } else {
        startPreviewAnimation('-1');
      }
    });
    
    powerToggle.addEventListener('change', () => {
      if (isUpdatingFromServer) return;
      
      const selectedAni = animationSelect.value;
      
      if (powerToggle.checked) {
        sendCommand('poweron');
        sendCommand('ani' + selectedAni);
        startPreviewAnimation(selectedAni);
      } else {
        sendCommand('poweroff');
        startPreviewAnimation('-1');
      }
    });
    
    speedSlider.addEventListener('input', () => {
      if (isUpdatingFromServer) return;
      
      const value = speedSlider.value;
      speedValue.textContent = value;
      
      const currentMode = animationSelect.value;
      if (powerToggle.checked && currentMode !== '0') {
        startPreviewAnimation(currentMode);
      }
      
      if (speedDebounceTimer) {
        clearTimeout(speedDebounceTimer);
      }
      
      speedDebounceTimer = setTimeout(() => {
        sendCommand('speed' + value);
      }, 50);
    });
    
    const initialAnimation = animationSelect.value;
    previewTitle.textContent = animationNames[initialAnimation];
    startPreviewAnimation('-1');
  </script>
</body>
</html>
)rawliteral";

void setLED(int index, uint8_t state) {
  if (ledStates[index] != state) {
    ledStates[index] = state;
    int pins[] = {output1, output2, output3, output4};
    digitalWrite(pins[index], state);
  }
}

void setAllLEDs(uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4) {
  setLED(0, s1);
  setLED(1, s2);
  setLED(2, s3);
  setLED(3, s4);
}

void broadcastState() {
  if (webSocket.connectedClients() == 0) return;
  
  unsigned long currentMillis = millis();
  
  if (stateChanged && (currentMillis - lastStateBroadcast >= STATE_BROADCAST_INTERVAL)) {
    int speedLevel = (1100 - animationSpeed) / 100;

    if (animationSpeed == 50 && speedLevel == 10) {
      speedLevel = 11;
    }
    
    if (speedLevel < 1) speedLevel = 1;
    if (speedLevel > 11) speedLevel = 11;
    
    String stateJson = "{\"animation\":" + String(selectedAnimation) + 
                      ",\"power\":" + String(isPowerOn ? "true" : "false") + 
                      ",\"speed\":" + String(speedLevel) + "}";
    webSocket.broadcastTXT(stateJson);
    
    lastStateBroadcast = currentMillis;
    stateChanged = false;
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
      
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      
      int speedLevel = (1100 - animationSpeed) / 100;
      
      if (animationSpeed == 50 && speedLevel == 10) {
        speedLevel = 11;
      }
      
      if (speedLevel < 1) speedLevel = 1;
      if (speedLevel > 11) speedLevel = 11;
      
      String stateJson = "{\"animation\":" + String(selectedAnimation) + 
                        ",\"power\":" + String(isPowerOn ? "true" : "false") + 
                        ",\"speed\":" + String(speedLevel) + "}";
      webSocket.sendTXT(num, stateJson);
      break;
    }
    
    case WStype_TEXT: {
      String message = String((char*)payload);
      
      if (message.startsWith("poweroff")) {
        isPowerOn = false;
        currentAnimation = -1;
        setAllLEDs(HIGH, HIGH, HIGH, HIGH);
        stateChanged = true;
      }
      else if (message.startsWith("poweron")) {
        isPowerOn = true;
        stateChanged = true;
      }
      else if (message.startsWith("select")) {
        int aniNum = message.substring(6).toInt();
        if (aniNum >= 0 && aniNum <= 15) {
          selectedAnimation = aniNum;
          stateChanged = true;
        }
      }
      else if (message.startsWith("ani")) {
        int aniNum = message.substring(3).toInt();
        if (aniNum >= 0 && aniNum <= 15) {
          selectedAnimation = aniNum;
          if (isPowerOn) {
            currentAnimation = aniNum;
            animationStep = 0;
            chaserPosition = 0;
          }
          stateChanged = true;
        }
      }
      else if (message.startsWith("speed")) {
        int speedLevel = message.substring(5).toInt();
        if (speedLevel >= 1 && speedLevel <= 11) {
          animationSpeed = 1100 - (speedLevel * 100);
          if (animationSpeed < 50) animationSpeed = 50;
          stateChanged = true;
        }
      }
      break;
    }
  }
}

void runAnimation() {
  if (!isPowerOn || currentAnimation == -1) {
    setAllLEDs(HIGH, HIGH, HIGH, HIGH);
    return;
  }
  
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastUpdate >= animationSpeed) {
    lastUpdate = currentMillis;
    
    switch(currentAnimation) {
      case 0:
        setAllLEDs(LOW, LOW, LOW, LOW);
        break;
        
      case 1:
        setLED(0, chaserPosition == 0 ? LOW : HIGH);
        setLED(1, chaserPosition == 1 ? LOW : HIGH);
        setLED(2, chaserPosition == 2 ? LOW : HIGH);
        setLED(3, chaserPosition == 3 ? LOW : HIGH);
        chaserPosition = (chaserPosition + 1) % 4;
        break;
        
      case 2:
        setLED(0, chaserPosition == 3 ? LOW : HIGH);
        setLED(1, chaserPosition == 2 ? LOW : HIGH);
        setLED(2, chaserPosition == 1 ? LOW : HIGH);
        setLED(3, chaserPosition == 0 ? LOW : HIGH);
        chaserPosition = (chaserPosition + 1) % 4;
        break;
        
      case 3: {
        int bouncePattern[] = {0, 1, 2, 3, 2, 1};
        int pos = bouncePattern[animationStep % 6];
        setLED(0, pos == 0 ? LOW : HIGH);
        setLED(1, pos == 1 ? LOW : HIGH);
        setLED(2, pos == 2 ? LOW : HIGH);
        setLED(3, pos == 3 ? LOW : HIGH);
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        break;
      }
        
      case 4:
        if (animationStep % 2 == 0) {
          setAllLEDs(LOW, LOW, LOW, LOW);
        } else {
          setAllLEDs(HIGH, HIGH, HIGH, HIGH);
        }
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        break;
        
      case 5:
        if (animationStep % 2 == 0) {
          setAllLEDs(LOW, LOW, HIGH, HIGH);
        } else {
          setAllLEDs(HIGH, HIGH, LOW, LOW);
        }
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        break;
        
      case 6: {
        int wavePattern[] = {1,1,1,1, 1,1,1,0, 1,1,0,0, 1,0,0,0, 
                             0,0,0,0, 0,0,0,1, 0,0,1,1, 0,1,1,1};
        int idx = (animationStep % 8) * 4;
        setLED(0, wavePattern[idx] ? HIGH : LOW);
        setLED(1, wavePattern[idx+1] ? HIGH : LOW);
        setLED(2, wavePattern[idx+2] ? HIGH : LOW);
        setLED(3, wavePattern[idx+3] ? HIGH : LOW);
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        break;
      }
        
      case 7: {
        int theater = animationStep % 3;
        setLED(0, (theater == 0 || theater == 2) ? LOW : HIGH);
        setLED(1, (theater == 1) ? LOW : HIGH);
        setLED(2, (theater == 0 || theater == 2) ? LOW : HIGH);
        setLED(3, (theater == 1) ? LOW : HIGH);
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        break;
      }
        
      case 8: {
        uint8_t splitPattern[][4] = {
          {HIGH,HIGH,HIGH,HIGH}, {LOW,HIGH,HIGH,LOW}, {HIGH,LOW,LOW,HIGH}, {LOW,LOW,LOW,LOW}
        };
        int idx = animationStep % 4;
        setAllLEDs(splitPattern[idx][0], splitPattern[idx][1], splitPattern[idx][2], splitPattern[idx][3]);
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        break;
      }
        
      case 9: {
        bool led1 = (chaserPosition == 0 || chaserPosition == 2);
        bool led4 = (chaserPosition == 0 || chaserPosition == 2);
        setLED(0, led1 ? LOW : HIGH);
        setLED(1, !led1 ? LOW : HIGH);
        setLED(2, !led4 ? LOW : HIGH);
        setLED(3, led4 ? LOW : HIGH);
        chaserPosition = (chaserPosition + 1) % 4;
        break;
      }
        
      case 10:
        setLED(0, random(0, 2) ? LOW : HIGH);
        setLED(1, random(0, 2) ? LOW : HIGH);
        setLED(2, random(0, 2) ? LOW : HIGH);
        setLED(3, random(0, 2) ? LOW : HIGH);
        break;
        
      case 11: {
        int count = animationStep % 16;
        setLED(0, (count & 0x01) ? LOW : HIGH);
        setLED(1, (count & 0x02) ? LOW : HIGH);
        setLED(2, (count & 0x04) ? LOW : HIGH);
        setLED(3, (count & 0x08) ? LOW : HIGH);
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        break;
      }
        
      case 12: {
        int pulsePattern[] = {1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1};
        int idx = animationStep % 16;
        if (pulsePattern[idx]) {
          setAllLEDs(LOW, LOW, LOW, LOW);
        } else {
          setAllLEDs(HIGH, HIGH, HIGH, HIGH);
        }
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        break;
      }
        
      case 13: {
        int kitt[] = {0, 1, 2, 3, 2, 1};
        int pos = kitt[animationStep % 6];
        setLED(0, pos == 0 ? LOW : HIGH);
        setLED(1, pos == 1 ? LOW : HIGH);
        setLED(2, pos == 2 ? LOW : HIGH);
        setLED(3, pos == 3 ? LOW : HIGH);
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        break;
      }
        
      case 14:
        if (animationStep % 4 < 2) {
          setAllLEDs(LOW, LOW, LOW, LOW);
        } else {
          setAllLEDs(HIGH, HIGH, HIGH, HIGH);
        }
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        break;
        
      case 15: {
        uint8_t cascade[][4] = {
          {LOW,HIGH,HIGH,HIGH}, {LOW,LOW,HIGH,HIGH}, {LOW,LOW,LOW,HIGH}, {LOW,LOW,LOW,LOW},
          {HIGH,LOW,LOW,LOW}, {HIGH,HIGH,LOW,LOW}, {HIGH,HIGH,HIGH,LOW}, {HIGH,HIGH,HIGH,HIGH}
        };
        int idx = animationStep % 8;
        setAllLEDs(cascade[idx][0], cascade[idx][1], cascade[idx][2], cascade[idx][3]);
        animationStep++;
        if (animationStep > 1000) animationStep = 0;
        break;
      }
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
  
  setAllLEDs(LOW, LOW, LOW, LOW);

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_STA);
  delay(100);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    if (attempts % 10 == 0) {
      Serial.println("\nRetrying connection...");
      WiFi.disconnect();
      delay(100);
      WiFi.begin(ssid, password);
    }
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi Connection FAILED!");
    delay(5000);
    ESP.restart();
  }
  
  Serial.println("\nWiFi Connected!");

  IPAddress gatewayIp = WiFi.gatewayIP();
  IPAddress local_IP(192, 168, gatewayIp[2], 50);
  IPAddress gateway(192, 168, gatewayIp[2], gatewayIp[3]);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(8, 8, 8, 8);
  IPAddress secondaryDNS(8, 8, 4, 4);

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Static IP failed");
  }

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  webSocket.loop();
  server.handleClient();
  runAnimation();
  broadcastState();

}

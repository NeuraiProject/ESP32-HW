let port = null;
let reader = null;
let writer = null;
let readableStreamClosed = null;
let writableStreamClosed = null;
let isReading = false;
let responseQueue = [];

function switchTab(tab) {
    document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
    document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
    
    event.target.classList.add('active');
    document.getElementById(tab + '-tab').classList.add('active');
}

function log(message, type = 'info') {
    const logContainer = document.getElementById('logContainer');
    const entry = document.createElement('div');
    entry.className = `log-entry log-${type}`;
    
    const time = new Date().toLocaleTimeString();
    entry.innerHTML = `<span class="log-time">[${time}]</span> ${message}`;
    
    logContainer.appendChild(entry);
    logContainer.scrollTop = logContainer.scrollHeight;
}

function clearLog() {
    document.getElementById('logContainer').innerHTML = '';
    log('Log cleared', 'info');
}

function showResult(data) {
    const resultBox = document.getElementById('resultBox');
    const resultContent = document.getElementById('resultContent');
    resultContent.textContent = JSON.stringify(data, null, 2);
    resultBox.classList.add('show');
}

async function connectToESP32() {
    try {
        log('🔌 Requesting serial port...', 'info');
        
        // Filtros para dispositivos ESP32
        const filters = [
            // ESP32-C6 (XIAO ESP32C6)
            { usbVendorId: 0x303A, usbProductId: 0x1001 }, // Espressif USB JTAG/serial debug unit
            // ESP32 genéricos
            { usbVendorId: 0x10C4, usbProductId: 0xEA60 }, // Silicon Labs CP210x
            { usbVendorId: 0x1A86, usbProductId: 0x7523 }, // CH340
            { usbVendorId: 0x0403, usbProductId: 0x6001 }, // FTDI FT232
            { usbVendorId: 0x067B, usbProductId: 0x2303 }, // Prolific PL2303
            // Seeed Studio (XIAO)
            { usbVendorId: 0x2886 }, // Seeed Studio
        ];
        
        port = await navigator.serial.requestPort({ filters });
        
        log('⚡ Opening port at 115200 baud...', 'info');
        await port.open({ baudRate: 115200 });
        
        const decoder = new TextDecoderStream();
        readableStreamClosed = port.readable.pipeTo(decoder.writable);
        reader = decoder.readable.getReader();
        
        const encoder = new TextEncoderStream();
        writableStreamClosed = encoder.readable.pipeTo(port.writable);
        writer = encoder.writable.getWriter();
        
        log('✅ Connected successfully!', 'success');
        
        document.getElementById('statusIndicator').classList.add('connected');
        document.getElementById('statusText').textContent = 'Connected to ESP32';
        document.getElementById('connectBtn').disabled = true;
        document.getElementById('disconnectBtn').disabled = false;
        
        // Enable buttons
        document.querySelectorAll('#saveBtn, #readBtn, #listBtn, #deleteBtn, #clearBtn').forEach(btn => {
            btn.disabled = false;
        });
        
        // Start reading
        isReading = true;
        readLoop();
        
    } catch (error) {
        log('❌ Connection error: ' + error.message, 'error');
    }
}

async function disconnectFromESP32() {
    try {
        isReading = false;
        
        if (reader) {
            await reader.cancel();
            await readableStreamClosed.catch(() => {});
        }
        
        if (writer) {
            await writer.close();
            await writableStreamClosed;
        }
        
        if (port) {
            await port.close();
        }
        
        log('🔌 Disconnected', 'info');
        
        document.getElementById('statusIndicator').classList.remove('connected');
        document.getElementById('statusText').textContent = 'Disconnected';
        document.getElementById('connectBtn').disabled = false;
        document.getElementById('disconnectBtn').disabled = true;
        
        // Disable buttons
        document.querySelectorAll('#saveBtn, #readBtn, #listBtn, #deleteBtn, #clearBtn').forEach(btn => {
            btn.disabled = true;
        });
        
    } catch (error) {
        log('⚠️ Disconnect error: ' + error.message, 'error');
    }
}

async function readLoop() {
    let buffer = '';
    
    while (isReading && reader) {
        try {
            const { value, done } = await reader.read();
            if (done) break;
            
            buffer += value;
            
            // Process complete lines
            let lines = buffer.split('\n');
            buffer = lines.pop(); // Keep incomplete line in buffer
            
            for (let line of lines) {
                line = line.trim().replace(/\r/g, '');
                if (line.length > 0) {
                    // Try to parse as JSON
                    if (line.startsWith('{')) {
                        try {
                            const data = JSON.parse(line);
                            log('<span class="log-rx">📥 RX:</span> ' + JSON.stringify(data), 'rx');
                            responseQueue.push(data);
                        } catch (e) {
                            log('<span class="log-rx">📥 RX:</span> ' + line, 'rx');
                        }
                    } else {
                        log('<span class="log-rx">📥 RX:</span> ' + line, 'rx');
                    }
                }
            }
        } catch (error) {
            log('❌ Read error: ' + error.message, 'error');
            break;
        }
    }
}

async function sendCommand(command) {
    try {
        const jsonCommand = JSON.stringify(command) + '\n';
        log('<span class="log-tx">📤 TX:</span> ' + jsonCommand.trim(), 'tx');
        
        await writer.write(jsonCommand);
        
        // Wait for response
        responseQueue = []; // Clear old responses
        const response = await waitForResponse(5000);
        
        if (response) {
            log('✅ Response received', 'success');
            showResult(response);
            return response;
        } else {
            log('⚠️ No response received (timeout)', 'error');
            return null;
        }
    } catch (error) {
        log('❌ Send error: ' + error.message, 'error');
        return null;
    }
}

function waitForResponse(timeout) {
    return new Promise((resolve) => {
        const startTime = Date.now();
        const checkResponse = () => {
            if (responseQueue.length > 0) {
                resolve(responseQueue.shift());
            } else if (Date.now() - startTime > timeout) {
                resolve(null);
            } else {
                setTimeout(checkResponse, 100);
            }
        };
        checkResponse();
    });
}

async function saveData() {
    const key = document.getElementById('saveKey').value.trim();
    const value = document.getElementById('saveValue').value.trim();
    
    if (!key || !value) {
        alert('Please enter both key and value');
        return;
    }
    
    await sendCommand({ action: 'save', key: key, value: value });
}

async function readData() {
    const key = document.getElementById('readKey').value.trim();
    
    if (!key) {
        alert('Please enter a key to read');
        return;
    }
    
    await sendCommand({ action: 'read', key: key });
}

async function listKeys() {
    await sendCommand({ action: 'list' });
}

async function deleteKey() {
    const key = document.getElementById('deleteKeyInput').value.trim();
    
    if (!key) {
        alert('Please enter a key to delete');
        return;
    }
    
    if (confirm(`Delete key "${key}"?`)) {
        await sendCommand({ action: 'delete', key: key });
    }
}

async function clearStorage() {
    if (confirm('⚠️ This will delete ALL stored data. Are you sure?')) {
        await sendCommand({ action: 'clear' });
    }
}

// Check Web Serial API support
if (!('serial' in navigator)) {
    document.getElementById('test-tab').innerHTML = '<div class="card"><h2>⚠️ Not Supported</h2><p>Your browser does not support Web Serial API. Please use Chrome, Edge, or Opera.</p></div>';
}

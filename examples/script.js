// MQTT Configuration
const config = {
    broker: 'wss://test.mosquitto.org:8081',
    clientId: 'iot-vision-pro-' + Math.random().toString(16).substr(2, 8),
    topics: {
        temperature: 'iot/visionpro/temperature',
        humidity: 'iot/visionpro/humidity',
        energy: 'iot/visionpro/energy',
        status: 'iot/visionpro/status',
        alerts: 'iot/visionpro/alerts'
    }
};

// Global variables
let client;
let temperatureChart;
let healthChart;
let notificationCount = 0;
const temperatureData = [];
const healthData = [];

// DOM Elements
const statusLight = document.getElementById('status-light');
const statusText = document.getElementById('status-text');
const temperatureValue = document.getElementById('temperature-value');
const humidityValue = document.getElementById('humidity-value');
const energyValue = document.getElementById('energy-value');
const notificationList = document.getElementById('notification-list');
const notificationCountEl = document.getElementById('notification-count');
const currentTime = document.getElementById('current-time');
const cpuUsage = document.getElementById('cpu-usage');
const memoryUsage = document.getElementById('memory-usage');
const uptime = document.getElementById('uptime');

// Initialize the application
function init() {
    // Set up real-time clock
    updateClock();
    setInterval(updateClock, 1000);
    
    // Set up system stats (mock data)
    updateSystemStats();
    setInterval(updateSystemStats, 5000);
    
    // Initialize charts
    initCharts();
    
    // Set up MQTT connection
    connectMQTT();
    
    // Set up event listeners
    setupEventListeners();
    
    // Generate some initial mock data
    generateMockData();
}

// Connect to MQTT broker
function connectMQTT() {
    statusText.textContent = 'Connecting...';
    statusLight.style.backgroundColor = '#ff9500'; // Orange for connecting
    
    client = new Paho.MQTT.Client(config.broker, config.clientId);
    
    client.onConnectionLost = onConnectionLost;
    client.onMessageArrived = onMessageArrived;
    
    const options = {
        timeout: 3,
        onSuccess: onConnect,
        onFailure: onConnectFailure,
        reconnect: true,
        cleanSession: true
    };
    
    client.connect(options);
}

// Connection success callback
function onConnect() {
    statusText.textContent = 'Connected';
    statusLight.style.backgroundColor = '#34c759'; // Green for connected
    
    // Subscribe to topics
    Object.values(config.topics).forEach(topic => {
        client.subscribe(topic, { qos: 0 });
        console.log(`Subscribed to ${topic}`);
    });
    
    // Add a connected notification
    addNotification('System connected', 'Successfully connected to MQTT broker', 'info');
}

// Connection failure callback
function onConnectFailure(error) {
    statusText.textContent = 'Connection failed';
    statusLight.style.backgroundColor = '#ff3b30'; // Red for error
    console.error('Connection failed:', error.errorMessage);
    
    // Try to reconnect after 5 seconds
    setTimeout(connectMQTT, 5000);
    
    // Add a connection failed notification
    addNotification('Connection error', `Failed to connect to broker: ${error.errorMessage}`, 'critical');
}

// Connection lost callback
function onConnectionLost(response) {
    if (response.errorCode !== 0) {
        console.error('Connection lost:', response.errorMessage);
        statusText.textContent = 'Disconnected';
        statusLight.style.backgroundColor = '#ff3b30'; // Red for disconnected
        
        // Add a disconnection notification
        addNotification('Connection lost', 'Disconnected from MQTT broker', 'critical');
        
        // Try to reconnect
        setTimeout(connectMQTT, 5000);
    }
}

// Message received callback
function onMessageArrived(message) {
    console.log(`Message received on ${message.destinationName}: ${message.payloadString}`);
    
    // Update UI based on topic
    switch(message.destinationName) {
        case config.topics.temperature:
            updateTemperature(message.payloadString);
            break;
        case config.topics.humidity:
            updateHumidity(message.payloadString);
            break;
        case config.topics.energy:
            updateEnergy(message.payloadString);
            break;
        case config.topics.status:
            updateSystemStatus(message.payloadString);
            break;
        case config.topics.alerts:
            processAlert(message.payloadString);
            break;
    }
}

// Update temperature display
function updateTemperature(value) {
    const temp = parseFloat(value).toFixed(1);
    temperatureValue.textContent = `${temp}°`;
    
    // Add to chart data
    temperatureData.push(temp);
    if (temperatureData.length > 15) temperatureData.shift();
    updateTemperatureChart();
}

// Update humidity display
function updateHumidity(value) {
    const hum = parseFloat(value).toFixed(1);
    humidityValue.textContent = `${hum}%`;
    
    // Update gauge position
    const gauge = document.getElementById('humidity-gauge');
    const percent = Math.min(100, Math.max(0, hum));
    gauge.style.setProperty('--pos', `${percent}%`);
}

// Update energy display
function updateEnergy(value) {
    const energy = parseFloat(value).toFixed(2);
    energyValue.textContent = `${energy}kW`;
    
    // Update sparkline (simplified)
    const sparkline = document.getElementById('energy-sparkline');
    sparkline.textContent = '📈 ' + '▁▂▃▄▅▆▇'.repeat(3).substring(0, 15);
}

// Update system status
function updateSystemStatus(status) {
    // This would update the health grid items based on status
    console.log('System status update:', status);
}

// Process alert messages
function processAlert(alert) {
    try {
        const alertData = JSON.parse(alert);
        addNotification(alertData.title, alertData.message, alertData.severity || 'warning');
    } catch (e) {
        addNotification('Alert', alert, 'warning');
    }
}

// Add notification to the panel
function addNotification(title, message, severity = 'info') {
    notificationCount++;
    notificationCountEl.textContent = notificationCount;
    
    const notification = document.createElement('div');
    notification.className = `notification-item ${severity}`;
    notification.innerHTML = `
        <div class="notification-icon">
            ${severity === 'critical' ? '⚠️' : severity === 'warning' ? '🔔' : 'ℹ️'}
        </div>
        <div class="notification-content">
            <div class="notification-title">${title}</div>
            <div class="notification-message">${message}</div>
        </div>
        <div class="notification-time">${new Date().toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'})}</div>
    `;
    
    notificationList.insertBefore(notification, notificationList.firstChild);
    
    // Limit to 50 notifications
    if (notificationList.children.length > 50) {
        notificationList.removeChild(notificationList.lastChild);
    }
    
    // Flash the notification button
    const btn = document.getElementById('notifications-btn');
    btn.classList.add('pulse');
    setTimeout(() => btn.classList.remove('pulse'), 1000);
}

// Initialize charts
function initCharts() {
    // Temperature chart
    const tempCtx = document.getElementById('temp-chart').getContext('2d');
    temperatureChart = new Chart(tempCtx, {
        type: 'line',
        data: {
            labels: Array(15).fill(''),
            datasets: [{
                data: temperatureData,
                borderColor: '#0071e3',
                backgroundColor: 'rgba(0, 113, 227, 0.1)',
                borderWidth: 2,
                tension: 0.4,
                fill: true,
                pointRadius: 0
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: { display: false },
                tooltip: { enabled: false }
            },
            scales: {
                x: { display: false },
                y: { display: false }
            }
        }
    });
    
    // Health chart
    const healthCtx = document.getElementById('health-chart').getContext('2d');
    healthChart = new Chart(healthCtx, {
        type: 'doughnut',
        data: {
            labels: ['Online', 'Warning', 'Offline'],
            datasets: [{
                data: [85, 10, 5],
                backgroundColor: ['#34c759', '#ff9500', '#ff3b30'],
                borderWidth: 0
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            cutout: '70%',
            plugins: {
                legend: { display: false },
                tooltip: { enabled: false }
            }
        }
    });
}

// Update temperature chart
function updateTemperatureChart() {
    temperatureChart.data.datasets[0].data = temperatureData;
    temperatureChart.update();
}

// Update clock
function updateClock() {
    currentTime.textContent = new Date().toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'});
}

// Update system stats (mock data)
function updateSystemStats() {
    cpuUsage.textContent = `${Math.floor(Math.random() * 30) + 10}%`;
    memoryUsage.textContent = `${Math.floor(Math.random() * 50) + 30}%`;
    
    // Simple uptime counter
    const now = new Date();
    const hours = now.getHours().toString().padStart(2, '0');
    const mins = now.getMinutes().toString().padStart(2, '0');
    const secs = now.getSeconds().toString().padStart(2, '0');
    uptime.textContent = `${hours}:${mins}:${secs}`;
}

// Set up event listeners
function setupEventListeners() {
    // Emergency stop button
    document.getElementById('emergency-stop').addEventListener('click', () => {
        if (confirm('Are you sure you want to emergency stop all systems?')) {
            addNotification('Emergency Stop', 'All systems are being shut down', 'critical');
            if (client && client.isConnected()) {
                client.send(config.topics.status, 'EMERGENCY_STOP', 1);
            }
        }
    });
    
    // Refresh data button
    document.getElementById('refresh-data').addEventListener('click', () => {
        addNotification('Data Refresh', 'Manual data refresh initiated', 'info');
        if (client && client.isConnected()) {
            client.send(config.topics.status, 'REFRESH_DATA', 0);
        }
    });
    
    // Clear notifications button
    document.getElementById('clear-notifications').addEventListener('click', () => {
        notificationList.innerHTML = '';
        notificationCount = 0;
        notificationCountEl.textContent = '0';
    });
    
    // Toggle switches
    document.getElementById('lighting-toggle').addEventListener('change', (e) => {
        const state = e.target.checked ? 'ON' : 'OFF';
        addNotification('Lighting System', `Turned ${state}`, 'info');
        if (client && client.isConnected()) {
            client.send(config.topics.status, `LIGHTING_${state}`, 0);
        }
    });
    
    document.getElementById('climate-toggle').addEventListener('change', (e) => {
        const state = e.target.checked ? 'ON' : 'OFF';
        addNotification('Climate Control', `Turned ${state}`, 'info');
        if (client && client.isConnected()) {
            client.send(config.topics.status, `CLIMATE_${state}`, 0);
        }
    });
    
    document.getElementById('security-toggle').addEventListener('change', (e) => {
        const state = e.target.checked ? 'ON' : 'OFF';
        addNotification('Security System', `Turned ${state}`, 'info');
        if (client && client.isConnected()) {
            client.send(config.topics.status, `SECURITY_${state}`, 0);
        }
    });
}

// Generate some mock data for demonstration
function generateMockData() {
    // Initial temperature data
    for (let i = 0; i < 15; i++) {
        temperatureData.push((Math.random() * 5 + 20).toFixed(1));
    }
    updateTemperatureChart();
    
    // Initial humidity gauge
    document.documentElement.style.setProperty('--pos', '65%');
    
    // Add some sample notifications
    addNotification('System Initialized', 'IoT Vision Pro dashboard is ready', 'info');
    addNotification('Temperature Alert', 'Temperature exceeding threshold in Zone 3', 'warning');
    addNotification('Device Offline', 'Sensor node 5 is not responding', 'critical');
}

// Initialize the app when the page loads
window.onload = init;
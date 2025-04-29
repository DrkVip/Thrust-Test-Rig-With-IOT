#include <Wire.h>
#include <HX711.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Adafruit_MLX90614.h>

// Define HX711 pins
#define LOAD_CELL_DT 4  // HX711 Data pin connected to GPIO32
#define LOAD_CELL_SCK 5 // HX711 Clock pin connected to GPIO33

// Define RPM sensor pin
#define RPM_SENSOR_PIN 13 // GPIO pin for RPM sensor

// Define accelerometer pins
#define ADXL335_X_PIN 36 // GPIO pin for X-axis of ADXL335
#define ADXL335_Y_PIN 39 // GPIO pin for Y-axis of ADXL335
#define ADXL335_Z_PIN 35 // GPIO pin for Z-axis of ADXL335

// Create sensor objects
HX711 loadCell;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// Variables to store sensor readings
long loadCellReading = 0;
float thrustN = 0.0;
volatile unsigned int rpmPulses = 0;
unsigned long lastRPMTime = 0;
float rpmValue = 0.0;
float tempMotor = 0.0; // Temperature of the motor
float accelX = 0.0, accelY = 0.0, accelZ = 0.0; // Acceleration values

// WiFi credentials
const char* ssid = "Your SSID"; 
const char* password = "Your SSID Password";

// Constants for thrust calculation
#define CALIBRATION_FACTOR 30.00

// Create an instance of the web server
WebServer server(80);

// Function declarations
void handleRoot();
void handleGetData();
void IRAM_ATTR rpmISR();

void setup() {
  Serial.begin(115200); // Initialize Serial Monitor
  Serial.println("Initializing sensors...");

  // Initialize Load Cell
  loadCell.begin(LOAD_CELL_DT, LOAD_CELL_SCK);

  // Set the calibration factor
  loadCell.set_scale(CALIBRATION_FACTOR);  // Set the calibration factor
  loadCell.tare(); // Tare the load cell

  Serial.println("Taring the load cell. Please ensure it's unloaded.");

  // Initialize MLX90614
  if (!mlx.begin()) {
    Serial.println("Error initializing MLX90614 sensor!");
  }

  // Setup RPM sensor
  pinMode(RPM_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RPM_SENSOR_PIN), rpmISR, FALLING);

  // Connect to Wi-Fi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // Wi-Fi connected, print the IP address
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Define HTTP handlers for routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/get_data", HTTP_GET, handleGetData);

  // Start the server
  server.begin();
}

void loop() {
  // Calculate RPM
  unsigned long currentTime = millis();
  if (currentTime - lastRPMTime >= 1000) { // Update every seconds
    rpmValue = (rpmPulses / 2.0) * 60.0;  // Calculate RPM (assuming 2 pulses per rotation)
    rpmPulses = 0;                        // Reset pulse count
    lastRPMTime = currentTime;
  }

  // Read MLX90614 for motor temperature
  tempMotor = mlx.readObjectTempC();

  // Read accelerometer values
  accelX = analogRead(ADXL335_X_PIN) * (3.3 / 4095.0); // Convert to voltage
  accelY = analogRead(ADXL335_Y_PIN) * (3.3 / 4095.0); // Convert to voltage
  accelZ = analogRead(ADXL335_Z_PIN) * (3.3 / 4095.0); // Convert to voltage

  // Read the load cell if it's ready
  if (loadCell.is_ready()) {
    loadCellReading = loadCell.get_units(10); // Average over 10 readings
  } else {
    Serial.println("Load cell not ready. Check connections.");
  }

  delay(500); // Wait for half a second before the next reading
  // Handle client requests
  server.handleClient();
}

void handleRoot() {
  String html = "<html><head><title>Sensor Data</title>";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  html += "</head><body>";
  html += "<h1>Real-Time Sensor Data</h1>";

  // Section for live chart
  html += "<h2>Live Chart</h2>";
  html += "<canvas id='myChart' width='50' height='10'></canvas>";
  html += "<script>";
  html += "var ctx = document.getElementById('myChart').getContext('2d');";
  html += "var myChart = new Chart(ctx, {";
  html += "type: 'line',";
  html += "data: {";
  html += "labels: [],"; // Empty array for timestamps
  html += "datasets: [";
  html += "{ label: 'Weight (kg)', data: [], borderColor: 'green', fill: false, yAxisID: 'y1' },"; 
  html += "{ label: 'Thrust (N)', data: [], borderColor: 'purple', fill: false, yAxisID: 'y2' },"; 
  html += "{ label: 'RPM', data: [], borderColor: 'orange', fill: false, yAxisID: 'y3' },"; 
  html += "{ label: 'Motor Temp (°C)', data: [], borderColor: 'red', fill: false, yAxisID: 'y4' }"; 
  html += "]},"; 
  html += "options: {";
  html += "scales: {";
  html += "y: {";
  html += "beginAtZero: true,";
  html += "title: { display: true, text: 'Values (kg / N / RPM / °C)' }";
  html += "},";
  html += "y1: { position: 'left', min: 0, max: 10, title: { display: true, text: 'Weight (kg)' } },";
  html += "y2: { position: 'right', min: 0, max: 1000, title: { display: true, text: 'Thrust (N)' } },";
  html += "y3: { position: 'right', min: 0, max: 10000, title: { display: true, text: 'RPM' } },";
  html += "y4: { position: 'left', min: 0, max: 100, title: { display: true, text: 'Motor Temp (°C)' } }";
  html += "}";
  html += "} });";
  html += "setInterval(function() {";
  html += "fetch('/get_data').then(response => response.json()).then(data => {";
  html += "myChart.data.labels.push((data.timestamp / 1000).toFixed(1));";
  html += "myChart.data.datasets[0].data.push(data.loadCellReading / 1000);"; // Convert grams to kilograms
  html += "myChart.data.datasets[1].data.push(data.thrustN);";
  html += "myChart.data.datasets[2].data.push(data.rpmValue);";
  html += "myChart.data.datasets[3].data.push(data.tempMotor);"; // Motor Temp
  html += "if (myChart.data.labels.length > 20) {";
  html += "myChart.data.labels.shift();";
  html += "myChart.data.datasets.forEach(dataset => dataset.data.shift());";
  html += "}";
  html += "myChart.update();";
  html += "});";
  html += "}, 1000);";
  html += "</script>";

  // Section for live textual data
  html += "<h2>Live Textual Data</h2>";
  html += "<div id='textData'></div>";
  html += "<script>";
  html += "setInterval(function() {";
  html += "fetch('/get_data').then(response => response.json()).then(data => {";
  html += "document.getElementById('textData').innerHTML = '<p>Weight: ' + (data.loadCellReading / 1000).toFixed(2) + ' kg</p>' + "; // Convert grams to kilograms
  html += "'<p>Thrust: ' + data.thrustN.toFixed(2) + ' N</p>' + ";
  html += "'<p>RPM: ' + data.rpmValue.toFixed(2) + ' RPM</p>' + ";
  html += "'<p>Motor Temp: ' + data.tempMotor.toFixed(2) + ' °C</p>' + ";
  html += "'<p>Accel X: ' + data.accelX.toFixed(2) + ' V</p>' + ";
  html += "'<p>Accel Y: ' + data.accelY.toFixed(2) + ' V</p>' + ";
  html += "'<p>Accel Z: ' + data.accelZ.toFixed(2) + ' V</p>';";
  html += "});";
  html += "}, 1000);";
  html += "</script>";

  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleGetData() {
  // Calculate thrust using the formula: Thrust = P * D^3 * RPM^2 * 10^-10
  thrustN = (0.0000029 * pow(10, 3) * 0.5 * 4.5 * pow(rpmValue, 2))/9812;  // Thrust in Newtons

  // Create JSON response
  StaticJsonDocument<300> doc;
  doc["timestamp"] = millis();
  doc["loadCellReading"] = loadCellReading;
  doc["thrustN"] = thrustN;
  doc["rpmValue"] = rpmValue;
  doc["tempMotor"] = tempMotor;   // Add motor temperature
  doc["accelX"] = accelX;         // Add accelerometer X-axis
  doc["accelY"] = accelY;         // Add accelerometer Y-axis
  doc["accelZ"] = accelZ;         // Add accelerometer Z-axis

  String response;
  serializeJson(doc, response);

  // Send JSON response
  server.send(200, "application/json", response);
}

void IRAM_ATTR rpmISR() {
  rpmPulses++;
}

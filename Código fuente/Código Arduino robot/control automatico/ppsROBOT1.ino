#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

///////////////MQTT/////////////////
const char* ssid = "RICARDO";
const char* password = "ofea5555";
const char* mqtt_server = "192.168.141.204";
const char* aut_topic = "casa/salon/automatico";
const char* foto = "casa/salon/foto";
const char* Temperature_topic = "casa/salon/temperatura";
const char* humidity_topic = "casa/salon/humedad";
const char* gas_topic = "casa/salon/gas";
const char* high_temp_topic = "falla/alta_temperatura";
const char* high_hum_topic = "falla/alta_humedad";
const char* high_gas_topic = "falla/alta_gas";
const char* nidos_topic = "casa/salon/nidosrecorridos";
const char* con_topic = "casa/salon/conteohuevos"; 
const char* nido2_topic = "casa/salon/conteohuevos"; 

//////////////////// sensor DHT11 ////////////////////////
#define DHTPIN 4       // Pin donde está conectado el DHT11
#define DHTTYPE DHT11  // Librería tipo de sensor DHT11
const int sensorPin1 = 5; // Define el pin del sensor
// Pines para Motor 1 y conexión L298N
#define PIN_IN1   33 
#define PIN_IN2   32 
#define PIN_ENA   14 
// Pines para Motor 2 y conexión L298N
#define PIN2_IN1  25 
#define PIN2_IN2  26 
#define PIN2_ENA  27 
// PWM 
#define MIN_PWM_SPEED 73  //ajustar pwm mín
#define MAX_PWM_SPEED 75 //ajustar pwm máx
////deginción de pines
int gas_sensor = 34;   
const int botonPin = 16;
const int NIDO2 = 21; //Entrada para habilitar nidos

// Configuración del sensor de gas más calibración
float m = -0.3523;        // Pendiente de la curva de calibración
float b = 0.714;          // Intersección de la curva de calibración
float R0 = 60;            // Valor inicial de R0 calculado
float sensorValue;
float sensor_volt; 
float RS_gas;
float ratio;
double ppm_log;
double ppm;

// Ajustes de calibración de temperaturas
float tempOffset = -1.0; 
float humOffset = -36.0;  
// Leer humedad y temperatura
float humedad;
float temperatura; 


unsigned long tiempoAnterior = 0;  //  el último tiempo
const unsigned long intervalo = 1000; //   segundos
unsigned long ultimoIntento = 0;

// Estados
bool automaticoActivo = false; // Esta variable se actualizará con el botón y con MQTT
bool automaticoActivo2 = false;
bool esperandoFoto = false;
unsigned long tiempoDeteccion = 0;
const unsigned long esperaFoto = 1000;

bool activacionNido1 = false;
unsigned long tiempoInicioNido1 = 0;
const unsigned long esperaNido1 = 900; //original 900
// estado de los motores por botón
bool motoresEncendidosPorBoton = false;
// Variable para el control del botón 
bool botonPresionadoPreviamente = false;
//contar nidos
int contadorNidos = 0;
bool estadoAnteriorSensor = HIGH;
unsigned long tiempoInicioLOW = 0;
const unsigned long tiempoMinimoLOW = 5000;  // ms
//pulsador manual
bool estadoNIDOs = false; // Estado actual apagado


WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conectar a MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("conectado");

      // Suscribirse al tópico de posición
      client.subscribe(aut_topic);
      client.subscribe(esp2P_topic);
      client.subscribe(con_topic);
      client.subscribe(nido2_topic);
    } else {
      Serial.print("Fallido, rc=");
      Serial.print(client.state());
      Serial.println(" Intentar de nuevo en 5 segundos...");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Mensaje recibido en tópico: ");
  Serial.println(topic);

  String messageTemp;

  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  Serial.print("Mensaje: ");
  Serial.println(messageTemp);

  // Control automático desde MQTT
    if (String(topic) == aut_topic) {
      if (messageTemp == "OFF") {
        automaticoActivo = false;
        Serial.println("automático desactivado por MQTT"); 
        detenerMotor(PIN_ENA);
        detenerMotor(PIN2_ENA);
        motoresEncendidosPorBoton = false; // estado del botón
     }  else if (messageTemp == "ON") {
        automaticoActivo = true;
        Serial.println("automático activado por MQTT.");
        activarMotor(PIN_IN1, PIN_IN2, PIN_ENA, HIGH, MAX_PWM_SPEED);
        activarMotor(PIN2_IN1, PIN2_IN2, PIN2_ENA, HIGH, MAX_PWM_SPEED);
        motoresEncendidosPorBoton = true; // estado del botón
       }
    }
    if (String(topic) == con_topic || String(topic) == nido2_topic ) {
      if (messageTemp == "Huevos detectados: 0" || messageTemp == "nido2") {
        Serial.println("No hay Huevos, volver al nivel 0 || nido2");
        if (digitalRead(sensorPin1) == LOW) { 
          activarMotor(PIN_IN1, PIN_IN2, PIN_ENA, HIGH, MAX_PWM_SPEED);
          activarMotor(PIN2_IN1, PIN2_IN2, PIN2_ENA, HIGH, MAX_PWM_SPEED);
          }
          activacionNido1 = true;
          tiempoInicioNido1 = millis();
        }
      }
}


void setup() {
  Serial.begin(115200);
  // Configuración de pines 
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);
  pinMode(botonPin, INPUT_PULLUP); // pull-up interno
  pinMode(PIN2_IN1, OUTPUT);
  pinMode(PIN2_IN2, OUTPUT);
  pinMode(PIN2_ENA, OUTPUT);
  pinMode(sensorPin1, INPUT); 
  pinMode(NIDO2, INPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  contadorNidos = 0;
}

void loop() {
  //  conexión MQTT activa
  if (!client.connected()) {
      reconnect();
    }
  client.loop();
  //botón automático
  int estadoBoton = digitalRead(botonPin);
  // flanco de bajada
  if (estadoBoton == LOW && !botonPresionadoPreviamente) {
    delay(50); //anti-rebote inicial
    motoresEncendidosPorBoton = !motoresEncendidosPorBoton;

    if (motoresEncendidosPorBoton || automaticoActivo) {
      Serial.println("Botón presionado - Motores ENCENDIDOS (retención)");
      activarMotor(PIN_IN1, PIN_IN2, PIN_ENA, HIGH, MAX_PWM_SPEED);   // Motor 1 activar
      activarMotor(PIN2_IN1, PIN2_IN2, PIN2_ENA, HIGH, MAX_PWM_SPEED); // Motor 2 activar
      automaticoActivo = true; 
      Serial.println("Modo automático activado por botón.");
    } else {
      Serial.println("Botón presionado - Motores APAGADOS (retención)");
      detenerMotor(PIN_ENA);   // Detener Motor 1
      detenerMotor(PIN2_ENA);  // Detener Motor 2
      Serial.println("Modo automático desactivado por botón.");
    }
    botonPresionadoPreviamente = true;
  }
  else if (estadoBoton == HIGH && botonPresionadoPreviamente) {
    botonPresionadoPreviamente = false; 
    delay(50);
  }
    // Lógica automática activación desde panel node-red
  if (automaticoActivo && !esperandoFoto) {
    if (digitalRead(sensorPin1) == LOW) {
      detenerMotor(PIN_ENA);   // Detener Motor 1
      detenerMotor(PIN2_ENA);  // Detener Motor 2
      automaticoActivo = false;
      esperandoFoto = true;
      tiempoDeteccion = millis();
    }
  }
      // Lógica automática despues de pasar el nido 1
  if (automaticoActivo2 && !esperandoFoto) {
    if (digitalRead(sensorPin1) == LOW) {
      detenerMotor(PIN_ENA);   // Detener Motor 1
      detenerMotor(PIN2_ENA);  // Detener Motor 2
      automaticoActivo2 = false;
      esperandoFoto = true;
      tiempoDeteccion = millis();
    }
  }
  if (esperandoFoto && millis() - tiempoDeteccion >= esperaFoto) {
    client.publish(foto, "ON");
    esperandoFoto = false;
  }

  unsigned long tiempoActual = millis();
    // Ejecutar la función cada 3 segundos
  if (tiempoActual - tiempoAnterior >= intervalo) {
    tiempoAnterior = tiempoActual;
    sensores(); // leer sensores
}
  if (digitalRead(NIDO2) == HIGH ) {
    Serial.println("nido2");
    // Publicar en mqtt ir al siguiente nido
    String mensaje = String("nido2");
    client.publish(nido2_topic, mensaje.c_str());
  }

 //contador de nidos
  bool estadoActual = digitalRead(sensorPin1);
  if (estadoActual == LOW && estadoAnteriorSensor == HIGH  ) {
    tiempoInicioLOW = millis();
  }
  if (estadoActual == LOW && (millis() - tiempoInicioLOW >= tiempoMinimoLOW)) {
    contadorNidos++;
    Serial.print("Nido detectado: ");
    Serial.println(contadorNidos);

    // Publicar en mqtt los nidos recorridos
    String mensaje = String(contadorNidos);
    client.publish(nidos_topic, mensaje.c_str());
    while (digitalRead(sensorPin1) == LOW) {
      delay(5);
    }
  }
}

// Función para activar motores
void activarMotor(int in1Pin, int in2Pin, int enaPin, int direccion, int velocidad) {
  int velocidad_ajustada = constrain(velocidad, MIN_PWM_SPEED, MAX_PWM_SPEED);

  if (direccion == HIGH) { 
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
  } else {
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, HIGH);
  }
  analogWrite(enaPin, velocidad_ajustada);
}

// Función para detener motores
void detenerMotor(int enaPin) {
  analogWrite(enaPin, 0); 
}
//sensores de temperatura, humedad, gas co2
void sensores(){
  humedad = dht.readHumidity() + humOffset;
  temperatura = dht.readTemperature() + tempOffset;
  // Publicar la temperatura y humedad
  String tempPayload = String(temperatura);
  client.publish(Temperature_topic, tempPayload.c_str());
  String humPayload = String(humedad);
  client.publish(humidity_topic, humPayload.c_str());

  // Verificar si la temperatura supera los 30°C enviar señal de adventencia
  if (temperatura > 30.0) {
    String highTempPayload = "Alerta: Alta temperatura detectada (" + tempPayload + "°C)";
    client.publish(high_temp_topic, highTempPayload.c_str());
  }

  // Verificar si la humedad supera los 70% enviar señal de advertencia
  if (humedad > 70.0) {
    String highHumPayload = "Alerta: Alta humedad detectada (" + humPayload + "%)";
    client.publish(high_hum_topic, highHumPayload.c_str());
  }

  // Leer y publicar el valor del sensor de gas
  sensorValue = analogRead(gas_sensor);
  sensor_volt = sensorValue * (5.0 / 4095.0);
  RS_gas = ((5.0 * 10.0) / sensor_volt) - 10.0;
  ratio = RS_gas / R0;
  ppm_log = (log10(ratio) - b) / m;
  ppm = pow(10, ppm_log);
  String gasPayload = String(ppm);
  client.publish(gas_topic, gasPayload.c_str());
  // Verificar si el gas supera los 3000 ppm enviar señal de advertencia
  if (ppm > 3000) {
    String highGasPayload = "Alerta: Alto contenido de gas detectado (" + gasPayload + " ppm)";
    client.publish(high_gas_topic, highGasPayload.c_str());
  }
}




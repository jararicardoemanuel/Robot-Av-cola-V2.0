#include <AccelStepper.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

////////////MQTT//////////////
const char* ssid = "RICARDO";
const char* password = "ofea5555";
const char* mqtt_server = "192.168.141.204";
const char* coord_topic = "casa/salon/coordenadas";


// Pines motores ESP32MAX V1
const int stepX = 27, dirX = 14;
const int stepY = 26, dirY = 13;
const int stepZ = 25, dirZ = 12;

// Pines ENA
const int enPinx = 23;
const int enPiny = 19;
const int enPinz = 17;

// Sensores fin de carrera
const int sensorX = 16; 
const int sensorY = 18; 
const int sensorZ = 17; 
const int NIDO2 = 5;

//////// Motor PP gripper
#define IN1 2
#define IN2 4
#define IN3 32
#define IN4 33

int steps[8][4] = {
  {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 1, 0},
  {0, 0, 1, 0}, {0, 0, 1, 1}, {0, 0, 0, 1}, {1, 0, 0, 1}
};
int vueltasDeseadas0 = 5;
const int pasosPorVuelta0 = 4096;
int pasosTotales0 = vueltasDeseadas0 * pasosPorVuelta0;
const int pasosPorVuelta1 = 200;
const int pasosPorVuelta23 = 1000;

const float offset2 = 40.0, offset3 = 55.0; //ángulos corregidos por condiciones mecánicas
float ultimoT1 = 0;
bool gripper = false;
bool nido2 = false;
AccelStepper motor1(AccelStepper::DRIVER, stepX, dirX);
AccelStepper motor2(AccelStepper::DRIVER, stepY, dirY);
AccelStepper motor3(AccelStepper::DRIVER, stepZ, dirZ);


WiFiClient espClient;
PubSubClient client(espClient);
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
      client.subscribe(coord_topic);
    } else {
      Serial.print("Fallido, rc=");
      Serial.print(client.state());
      Serial.println(" Intentar de nuevo en 5 segundos...");
      delay(5000);
    }
  }
}
// Variables globales para los ángulos thetas123
float t1x = 0, t2y = 0, t3z = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  String mensaje = "";
  for (int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }
  mensaje.trim();

  if (String(topic) == coord_topic) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, mensaje);
    if (!error) {
      float theta1 = doc["theta1"].as<float>();
      float theta2 = doc["theta2"].as<float>();
      float theta3 = doc["theta3"].as<float>();
      t1x = theta1;
      t2y = theta2 - offset2;
      t3z = theta3 - offset3;
      ultimoT1 = t1x;
      moverA(t1x, t2y, t3z);
      delay(10);
      gripper = true;
    }
  }

}



void setup() {
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.begin(115200);
  Serial.setTimeout(100);
  pinMode(NIDO2, OUTPUT);
  pinMode(enPinx, OUTPUT); digitalWrite(enPinx, LOW); //enPinx driver habilitados
  pinMode(enPiny, OUTPUT); digitalWrite(enPiny, LOW); //enPiny driver habilitados
  pinMode(enPinz, OUTPUT); digitalWrite(enPinz, LOW); //enPinz driver habilitados

  pinMode(sensorX, INPUT_PULLUP);
  pinMode(sensorY, INPUT_PULLUP);
  pinMode(sensorZ, INPUT_PULLUP);

  motor1.setAcceleration(100); motor1.setMaxSpeed(100);
  motor2.setAcceleration(300); motor2.setMaxSpeed(300);
  motor3.setAcceleration(300); motor3.setMaxSpeed(300);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  reconnect();
  hacerHoming();  // Al inicio se hace el homming para buscar el cero
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
 //secuencia en cascada del movimiento del robot
  if (gripper) {
    activarGripperHorario();
    hacerHoming1();
    delay(5);
    volverACero0(285);
    // definir a 0° los motores
    motor1.setCurrentPosition(0);
    motor2.setCurrentPosition(0);
    motor3.setCurrentPosition(0);
    long pasos50 = 125.0 * pasosPorVuelta1 / 360.0;
    motor1.moveTo(pasos50);
    while (motor1.distanceToGo() != 0) motor1.run();
    volverACero12(-130, 0);
    activarGripperAntiHorario();
    hacerHomingYZ(); //este es hacer homming de dos ejesYZ
    delay(5);
    hacerHoming();
    digitalWrite(NIDO2, HIGH);
    delay(900);
    digitalWrite(NIDO2, LOW);
    gripper = false;
  }
}


//Mover motores
void moverA(float t1x, float t2y, float t3z) {
  long p1 = t1x * pasosPorVuelta1 / 360.0;
  long p2 = t2y * pasosPorVuelta23 / 360.0;
  long p3 = t3z * pasosPorVuelta23 / 360.0;
  motor1.moveTo(p1);
  motor2.moveTo(p2);
  motor3.moveTo(p3);
  while (motor1.distanceToGo() != 0 || motor2.distanceToGo() != 0 || motor3.distanceToGo() != 0) {
    motor1.run(); motor2.run(); motor3.run();
  }
}

void volverACero0(float p11){
  long p21 = p11 * pasosPorVuelta1 / 360.0;
  motor1.moveTo(p21);
  while (motor1.distanceToGo() != 0) motor1.run();
}
void volverACero12(float p12, float p13) {
  long p22 = p12 * pasosPorVuelta23 / 360.0;
  long p23 = p13 * pasosPorVuelta23 / 360.0;
  motor2.moveTo(p22);
  while (motor2.distanceToGo() != 0) motor2.run();
  motor3.moveTo(p23);
  while (motor3.distanceToGo() != 0) motor3.run();
}
void volverACero1(float p11, float p12, float p13) {
  long p21 = p11 * pasosPorVuelta1 / 360.0;
  long p22 = p12 * pasosPorVuelta23 / 360.0;
  long p23 = p13 * pasosPorVuelta23 / 360.0;

  motor2.moveTo(p22);
  while (motor2.distanceToGo() != 0) motor2.run();
  motor3.moveTo(p23);
  while (motor3.distanceToGo() != 0) motor3.run();
  motor1.moveTo(p21);
  while (motor1.distanceToGo() != 0) motor1.run();
}

void hacerHoming() {
  Serial.println("Iniciando homing...");
  // Mover solo el eje X a +50° para encontrar el homming de seguridad 
  long pasos50 = 50.0 * pasosPorVuelta1 / 360.0;
  motor1.moveTo(pasos50);
  bool encontrado = false;

while (motor1.distanceToGo() != 0) {
    if (digitalRead(sensorX) == LOW) {
      motor1.stop();
      delay(100);
      motor1.setCurrentPosition(0);
      Serial.println("X listo +");
      encontrado = true;
      break;
    }
    motor1.run();
  }
  // FASE 2 buscar en - dirección antihorario
  if (!encontrado) {
    motor1.setMaxSpeed(100);
    motor1.moveTo(-10000); // largo recorrido negativo
    while (digitalRead(sensorX) == HIGH) {
      motor1.run();
    }
    motor1.stop();
    delay(100);
    motor1.setCurrentPosition(0);
    Serial.println("X listo -");
  }
  motor2.setMaxSpeed(300);
  motor2.moveTo(10000);
  while (digitalRead(sensorY) == HIGH) motor2.run();
  motor2.stop(); delay(100);
  motor2.setCurrentPosition(0);
  Serial.println("Y listo.");
  motor3.setMaxSpeed(300);
  motor3.moveTo(10000);
  while (digitalRead(sensorZ) == HIGH) motor3.run();
  motor3.stop(); delay(100);
  motor3.setCurrentPosition(0);
  Serial.println("Z listo.");
  delay(200);
  Serial.println("Homing completado.");
}

//hacer homming en dos ejes y z
void hacerHomingYZ() {
  Serial.println("Iniciando homing...");
  motor2.setMaxSpeed(300);
  motor2.moveTo(10000);
  while (digitalRead(sensorY) == HIGH) motor2.run();
  motor2.stop(); delay(100);
  motor2.setCurrentPosition(0);
  Serial.println("Y listo.");

  motor3.setMaxSpeed(300);
  motor3.moveTo(10000);
  while (digitalRead(sensorZ) == HIGH) motor3.run();
  motor3.stop(); delay(100);
  motor3.setCurrentPosition(0);
  Serial.println("Z listo.");
  delay(200);
  Serial.println("HomingYZ listo");
}


void hacerHoming1() {
  Serial.println("Iniciando homing salida");
  motor2.setMaxSpeed(300);
  motor2.moveTo(10000);
  while (digitalRead(sensorY) == HIGH) motor2.run();
  motor2.stop(); delay(100);
  motor2.setCurrentPosition(0);
  Serial.println("Y listo"); 

  motor3.setMaxSpeed(300);
  motor3.moveTo(10000);
  while (digitalRead(sensorZ) == HIGH) motor3.run();
  motor3.stop(); delay(100);
  motor3.setCurrentPosition(0);
  Serial.println("Z list");

  motor1.setMaxSpeed(200);
  if (ultimoT1 <= -30) {
    motor1.moveTo(10000); // sentido antihorario
  } else {
    motor1.moveTo(-10000);  // sentido horario
  }
  while (digitalRead(sensorX) == HIGH) motor1.run();
  motor1.stop(); delay(100);
  motor1.setCurrentPosition(0);
  digitalWrite(enPinx, LOW);
  Serial.println("X listo.");

  delay(200);
  Serial.println("Homing completado");
}
void activarGripperAntiHorario() {
  for (int paso = 0; paso < pasosTotales0; paso++) {
    int i = paso % 8; //paso actual del motor
    stepMotor(i);
    delay(1);
  }
  apagarMotor();
}

void activarGripperHorario() {
  for (int paso = 0; paso < pasosTotales0; paso++) {
    int i = 7 - (paso % 8); //pasos para sentido horario
    stepMotor(i);
    delay(1); 
  }
  apagarMotor();
}

void apagarMotor() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void stepMotor(int step) {
  digitalWrite(IN1, steps[step][0]);
  digitalWrite(IN2, steps[step][1]);
  digitalWrite(IN3, steps[step][2]);
  digitalWrite(IN4, steps[step][3]);
}

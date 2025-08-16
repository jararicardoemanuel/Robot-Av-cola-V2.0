# 🐔🥚Robot Avícola V2.0

Proyecto de la PPS en Ingeniería Mecatrónica realizado en la FI-UNLZ
Sistema robótico con visión artificial y IoT para detectar y recolectar huevos en nidos, monitoreo de variables ambientales temperatura, gas CO2, húmedad.

## 🚀 Tecnologías Aplicadas
- ESP32 - Shield cnc 3 ejes
- Python - OpenCV
- Node-RED - MQTT
- Sensores de CO₂ MQ135, temperatura y humedad DHT11

## 📸 Imagen del prototipo

<p align="center">
  <img src="robot.jpg" alt="Vista del robot" width="400"/>
</p>


## 📐 Modelo matemático (Cinemática inversa)
$$
\theta_2 = \cos^{-1}\left( \frac{x^2 + y^2 - L_1^2 - L_2^2}{2 L_1 L_2} \right)
$$

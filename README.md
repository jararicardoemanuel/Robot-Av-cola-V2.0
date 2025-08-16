# 🐔 Robot Avícola V2.0

Proyecto de la PPS en Ingeniería Mecatrónica.  
Sistema robótico con visión artificial y IoT para detectar y recolectar huevos en nidos.

## 🚀 Tecnologías
- ESP32 + Arduino
- Python + OpenCV
- Node-RED + MQTT
- Sensores de CO₂, temperatura y humedad

## 📸 Imagen del prototipo
![Vista del robot](robot.jpg)

## 📐 Modelo matemático (Cinemática inversa)
$$
\theta_2 = \cos^{-1}\left( \frac{x^2 + y^2 - L_1^2 - L_2^2}{2 L_1 L_2} \right)
$$

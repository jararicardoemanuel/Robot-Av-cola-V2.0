# 🐔🥚Robot Avícola V2.0

Proyecto de la PPS en Ingeniería Mecatrónica realizado en la FI-UNLZ.

Sistema robótico con visión artificial, IoT para detectar y recolectar huevos en nidos, implementación de monitoreo de variables ambientales temperatura, gas CO2, húmedad.

## 🟠Tecnologías Aplicadas
• ESP32 - Shield cnc 3 ejes
• Python - OpenCV
• Node-RED - MQTT
• Sensores de CO₂ MQ135, temperatura y humedad DHT11

## 🟠Prototipo REAL

<p align="center">
  <img src="robot.jpg" alt="Vista del robot" width="400"/>
</p>

## 🟠Modelo matemático (Cinemática inversa) Robot

 🔹 Articulación de la base $$\(\theta_1)$$

Este ángulo se obtiene a partir de la proyección del punto de acción sobre el plano XY.  
Las rotaciones se consideran respecto al eje **Z**.

$$
\theta_1 = 2\arctan\left(\frac{py}{px}\right)
$$

🔹 Articulación del brazo $$\(\theta_2)$$

$$
A_1 = l_1 + l_2 \cos(\theta_3), \quad A_2 = l_2 \sin(\theta_3)
$$

La distancia proyectada sobre el plano XY es:

$$
d_p = p_x \cos(\theta_1) + p_y \sin(\theta_1)
$$

Relaciones trigonométricas:

$$
\sin(\theta_2) = \frac{p'_z A_1 - d_p A_2}{A_1^2 + A_2^2}
$$

$$
\cos(\theta_2) = \frac{d_p A_1 - p'_z A_2}{A_1^2 + A_2^2}
$$

Finalmente:

$$
\theta_2 = \arctan\left(\frac{\sin(\theta_2)}{\cos(\theta_2)}\right)
$$

🔹  Articulación del codo $$\(\theta_3)$$

Se ajusta en el plano vertical de la base, considerando la distancia entre el gripper y el extremo del codo.  

1. Se calcula la coordenada corregida en Z:

$$\ p'_z = p_z - b \$$



1. Distancia proyectada:

$$\ D^2 = p_x^2 + p_y^2 + (p'_z)^2 \$$


2. Aplicando la ley de cosenos:

$$\cos(\theta_3) = \frac{D^2 - l_1^2 - l_2^2}{2 l_1 l_2}\$$


3. Ángulo del codo:


$$\theta_3 = \arctan\left(\frac{\sqrt{1-\cos^2(\theta_3)}}{\cos(\theta_3)}\right)\$$


configuración **codo arriba**


• Con estos tres ángulos articulares $$\(\theta_1, \theta_2, \theta_3)\$$ se obtiene la **configuración del robot** para alcanzar cualquier punto dentro de su espacio de trabajo.


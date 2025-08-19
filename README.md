<h1 align="center" style="color:brown;">
  🐔🥚Robot Avícola V2.0
</h1>

Proyecto de la PPS en Ingeniería Mecatrónica realizado en la FI-UNLZ.

Sistema robótico de 3 GDL para realizar la recolección de huevos en nidos horizontales, tiene implementado un software con procesamiento de imágenes para la detección y localización en python utilizando la librería cv2 y un modelo pre-entrenado de roboflow. Se emplea IoT para el accionamiento de control del sistema, apliación para el envío de mensajes y recepción a través de "MQTT" además del monitoreo de variables ambientales cómo la temperatura, gas CO2, húmedad.

## 🟠Tecnologías Aplicadas
• ESP32 - Shield cnc 3 ejes, para el control de los motores nema 17, motores eléctricos de cc y sensores.
• Python - OpenCV, para el diseño de software de detección y calculo de la cinematica inversa
• Node-RED - MQTT, para el envío y recepción de mensajes
• Sensores de CO₂ MQ135, temperatura y humedad DHT11

## 🟠Prototipo REAL

<p align="center">
  <img src="20250718_190812.jpg" alt="Vista del robot" width="400"/>
</p>
<p align="center">
  <img src="gif.gif" alt="Vista del robot" width="400"/>
</p>


## 🟠Procesamiento de Imágenes

Para la detección automática de huevos se utiliza un modelo de visión por computadora entrenado en la plataforma Roboflow. El flujo de trabajo se resume en las siguientes etapas:

• Captura de imagen
  La cámara ESP32-CAM obtiene imágenes en tiempo real desde la parte superior del nido, cubriendo toda la parte donde se ubican los huevos.
  
• Inferencia con el modelo
  Las imágenes capturadas son enviadas al modelo de Roboflow, el cual aplica un algoritmo de detección de objetos (YOLOv9). Como resultado, se obtiene un conjunto de predicciones que      incluyen:
  Clases detectadas (``huevo'').
  Confianza de detección (probabilidad asociada a la predicción).
  Coordenadas del recuadro delimitador:
  (x, y, w, h) donde $x$ e $y$ representan la posición central del objeto, mientras que $w$ y $h$ corresponden al ancho y alto del recuadro.

• Conversión a coordenadas del robot
  A partir de las coordenadas $(x, y)$ obtenidas en la cuadrícula de la imagen (resolución de $640 \times 480$ píxeles), se realiza una transformación a coordenadas físicas reales del     robot:
    (X_r, Y_r) = f(x, y)
  donde la función $f$ corresponde a la \textit{calibración} que traduce los píxeles en grados de los motores paso a paso, garantizando que el brazo robótico pueda posicionarse            correctamente sobre el huevo.

• Selección y validación
  Se consideran válidas únicamente las detecciones cuyo nivel de confianza sea mayor a un umbral predefinido $0.75$. 
    
 • Envío al sistema de control
  Finalmente, las coordenadas cartesianas corregidas $(X_r, Y_r)$ se calcula la cinemática inversa del robot RRR para luego transmitir los angulos por el protocolo MQTT al controlador     del robot, que ejecuta la secuencia de agarre y recolección.

  La salida del modelo Roboflow
  La predicción para un huevo detectado es el siguiente:
  "predictions": 
      "class": "egg",
      "confidence": 0.89,
      "x": 320,
      "y": 240,
      "width": 50,
      "height": 60
Este resultado indica que se detectó un \textbf{huevo} con una confianza del $89%$ en la posición central $(320, 240)$. Posteriormente, este punto se transforma en coordenadas físicas para accionar el brazo robótico.

<p align="center">
  <img src="detección.png" alt="Vista del robot" width="720"/>
</p>
<p align="center">
  <img src="coordenadas.png" alt="Vista del robot" width="720"/>
</p>
## 🟠Modelo matemático (Cinemática inversa) Robot

En los siguientes ítems se describe el modelo cinemático inverso aplicado al robot de 3GDL por método geométrico.

 🔹 Articulación de la base $$\(\theta_1)$$

Este ángulo se obtiene a partir de la proyección del punto de acción sobre el plano XY.  
Las rotaciones se consideran respecto al eje Z.

$Coordenadas (px, py)$


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
\theta_2 = 2\arctan\left(\frac{\sin(\theta_2)}{\cos(\theta_2)}\right)
$$

🔹  Articulación del codo $$\(\theta_3)$$

Se ajusta en el plano vertical de la base, considerando la distancia entre el gripper y el extremo del codo.  

Paso1. Se calcula la coordenada corregida en Z:

$$\ p'_z = p_z - b \$$



Paso2. Distancia proyectada:

$$\ D^2 = p_x^2 + p_y^2 + (p'_z)^2 \$$


Paso3. Aplicando la ley de cosenos:

$$\cos(\theta_3) = \frac{D^2 - l_1^2 - l_2^2}{2 l_1 l_2}\$$


Paso4. Ángulo del codo:


$$\theta_3 = 2\arctan\left(\frac{\sqrt{1-\cos^2(\theta_3)}}{\cos(\theta_3)}\right)\$$


Configuración **codo arriba**


• Con estos tres coordenadas articulares $$\(\theta_1, \theta_2, \theta_3)\$$ se obtiene la **configuración del robot** para alcanzar cualquier punto dentro de su espacio de trabajo 17x13cm.



## 🟠Video del Prototipo en funcionamiento

[Link de Google Drive](https://drive.google.com/file/d/1ZrKL6yfj1HvtoRiwAzGiAirnJcBjSen1/view?usp=drive_link)


<p align="center">
  <img src="Logo FIUNLZ.png" alt="Vista del robot" width="400"/>
</p>

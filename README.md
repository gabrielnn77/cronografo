# cronógrafo
cronógrafo para aire comprimido basado en Arduino

# Listado de materiales para la construccion
* Arduino uno (compatible) . Verificar que tenga cristal para los timer <link>https://articulo.mercadolibre.com.ar/MLA-802733629-arduino-uno-r3-cable-usb-atmega328p-smd-compatible-ch340-_JM</link>
* shield LCD 16x2 con botones <link>https://articulo.mercadolibre.com.ar/MLA-861304422-shield-display-lcd-1602-16x2-botones-arduino-_JM</link>
* 2 fotodiodos. Yo use estos : osram Sfh 203 Fa <link>https://articulo.mercadolibre.com.ar/MLA-698467492-sfh-203-fa-fotodiodo-receptor-infrarrojo-900nm-itytarg-_JM</link> . Los baratos no funcionan.
* 2 Leds infrarojos. Use unos baratos de 850 o 900 nm
* soporte para los sensores y caja para el Arduino. Impreso en 3D con resina PLA (puede ser una mejor, pero esta funciona)
* Alguna forma de alimentar el Arduino. Yo use un powerBank de esos para celulares (la contra es que para apagarlo hay que desenchufarlo, y queda fuera de la caja del arduino, con un cable usb de impresora), pero vienen modulos para arduino que usan una pila 18650 que se pueden montar dentro de la caja, y agregarle un switch de encendido, y hasta tienen una entrada microUSB para cargar la pila sin sacarla de la caja, con un cargador de celulares (tener en cuenta que hay que modificar la caja para darle lugar para el modulo de batería) <link>https://articulo.mercadolibre.com.ar/MLA-878152686-modulo-power-bank-para-1-bateria-18650-5v-3v-micro-usb-_JM</link>

# caracteristicas y limitaciones
* Es un cronógrafo de tubo, solo para aire comprimido. Por la forma de acoplarlo al caño, creo que seria incómo para rifles con miras abiertas. Se adapta bien a caños que van desde los 15 mm a los 40 mm (aprox). Para acoplarlo al caño, usé un par de cordones elásticos con esas trabas que se usan en ropa deportiva, capuchas de camperas, impermeables para lluvia, etc. Se puede usar una cinta con abrojo también, pero el elástico me pareció mas práctico para colocarlo.
* Mide la velocidad en FPS y m/s
* Mide hasta 150 velocidades (se puede cambiar en el firmware este valor, no lo crei necesario)
* Muestra velocidad Máx (con el nro de disparo), Mín (con el nro de disparo), Promedio (con la cantidad de disparos que usó para calcular el promedio), y spread de la totalidad de los disparos registrados.
* Muestra la energía en Joules. Por defecto lo calcula utilizando un peso de balín de 18 grains, pero se puede cambiar el peso desde los botones, y recalcula la energía de cada disparo
* Para verificar el funcionamiento, se compararon las mediciones en simultáneo con un cronógrafo comercial marca Chrony modelo alpha (rojo), dando diferencias de +- 6 FPS . Creo que es factible mejorar la presición por dos caminos (pero no lo probé): eliminar el código que en el loop principal llama a la función que lee los botones (hace una lectura analógica del pin A0, para determinar que boton se presionó, creo que eso podría generar variaciones en el tiempo para atender la interrupciones disparadas por los sensores), dejando que solamente se muestre la velocidad del último disparo, y la otra forma de mejorar la presición , sería aumentar la distancia entre sensores, actualmente está en 10 cm, con llevarlo a 20 cm se mejoraría la presición en la toma del tiempo.

# software o herramientas necesarias
* aplicacion fritzing para visualizar el diagrama de conexión, o algun visor de imágenes porque lo exporté a PNG
* IDE de arduino para subir el codigo a la placa
* freecad para ver o modificar el diseño de los soportes, y exportarlos a STL para imprimirlos en 3d

# pruebas, fotos y videos
* Simulacion 3d del soporte y caja: <link>https://youtu.be/3Tz094T5HoM</link>
* Prueba inicial contra un chrony, con el codigo actual la diferencia de FPS es mas chica(+-5FPS): <link>https://youtu.be/s2DOzHH34lQ</link>

/*
 para el LED IR : emisor
    330 ohms 
 fototransistor : receptor o sensor
    10k ohms 
    probar otros valores, por ejemplo 4.7 Kohms
    5V <-------[fotodiodo]----|-------[resistencia]------GND
                              |
                              |
                            pin digital

                            
https://www.luisllamas.es/leer-un-pulsador-con-arduino/
https://hardwarehackingmx.wordpress.com/2014/01/15/leccion-20-arduino-sensor-infrarrojo-basico/


conexion emisor y receptor

  Emisor -  LED IR
      (------- +   (pata larga a voltaje)
      (____    -   (pata corta a masa , negativo , marca plana , cuerpo grande dentro del led)

  Receptor - fotodiodo
      (----      +   (este es el colector: C tiene una marca plana o corte flat y es mas corto )
      (_________ -    ( pata larga a masa )




https://wiki.dfrobot.com/Arduino_LCD_KeyPad_Shield__SKU__DFR0009_
  pantalla LCD - shield
   usa el A0 para control de botones


   
FOTODIODO Sfh 203 Fa // Sfh203 Fa Osram

conexion, resistencia
https://electronics.stackexchange.com/questions/167328/photodiode-turn-digital-input-pin-into-1-with-little-light-input
   5 Megohms mas o menos para 10 lux
      change resitor to 1Megaohm and the digital pin turns 1 providing light (20 lux);
      without light its analog value is at 400

      este parece mejor:
      https://www.i-ciencias.com/pregunta/48286/como-utilizar-sfh235-ir-fotodiodo-correctamente



      IMPLEMENTACION UTILIZANDO LOS REGISTROS DEL TIMER DE ARDUINO
      PARA OBRTENER MEJOR RESOLUCION DEL TIEMPO

      con la funcion micros() se tiene una resolucion de 4us , que da un error de 8.5 FPS para lecturas de alrededor de 900 FPS

      https://stackoverflow.com/questions/36254652/arduino-tcnt1-to-count-clock-cycles-between-interrupts
      int t1;// declare global variable somewhere
      t1 = TCNT1; //save timer value in ISRx interrupts
      Be sure to setup prescaler value TCCR1B.CSn and handle timer overflow interrupt, else you will lose time data: ISR(TIMER1_OVF_vect)

      https://www.instructables.com/Arduino-Timer-Interrupts/

      voy a usar:
      timer1 : 16 bits :: NO LO USO PORQUE ME DA CONFLICTO CON EL LCD
      prescaler: 8 (2 MHz, deberia ser 0.5 us de resolucion) (1 us de diferencia da 2 FPS de error a 900 FPS)

      timer2 : 8 bits 
      prescaler: 1 (16 MHz, deberia ser [0.0000000625 s] - [0.0625 us] de resolucion)
        ---> era medio inestable, daba lecturas raras


      timer2
        prescaler : 8  --> 0.5 us
        modo interrupcion en overflow
        tomado de https://github.com/ElectricRCAircraftGuy/eRCaGuy_TimerCounter
      

      DESHABILITAR timer0
      no se puede usar delay(), millis() o micros()
      https://forum.arduino.cc/index.php?topic=206513.0
      https://forum.arduino.cc/index.php?topic=200992.0

*/

#include <CheapLCD.h>

CheapLCD lcd;

// --------------------------------------------------------------------------------------
// define some values used by the panel and buttons
int lcd_key     = 0;

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// constantes y variables GLOBALES relacionadas al chrony
//  sensor 1 : D2
#define pinS1     2
//  sensor 2 : D3
#define pinS2     3
//  tiempo para ignorar rebotes en las lecturas de los sensores
#define rebote    1000000
//  10 cm expresado en pie : distancia entre sensores
#define long_pie  0.328084
//  10 cm expresado en [m] : distancia entre sensores
#define long_m  0.1

// velocidad minima permitida
#define vel_min 1
// velocidad maxima permitida
#define vel_max 5000

// muestra velocidad del disparo
#define modoVEL     0
// muestra velocidad MAX
#define modoMAX     1
// muestra velocidad MIN
#define modoMIN     2
// muestra velocidad AVG
#define modoAVG     3
// muestra SPREAD
#define modoSPREAD  4
// muestra peso balin
#define modoPESO    5
// es el maximo valor de modo definido, es para manejar los botones y el cambio de modo
#define modoTOPE 5

//  es el tamaño del buffer para alamacenar mediciones
#define CANT_DISPAROS 150

  volatile unsigned long desborde = 0;
  volatile unsigned char running = 0;
  volatile unsigned long tiempos[CANT_DISPAROS];
  volatile unsigned char idx = 0;       //  indice de la lectura en el arreglo
  volatile unsigned long tiempo_actual = 0;
  volatile boolean flag_save;

  byte TCCR0A_old;
  byte TCCR0B_old;
  byte TCCR1A_old;
  byte TCCR1B_old;
  
  unsigned char idx_avg = 0;            //  cantidad de lecturas validas, para sacar el promedio
  unsigned char idx_ant = 0;            //  indice lectura anterior, para detectar nuevas lecturas
  unsigned char idx_show = 0;           //  indice lectura mostrada en el LCD
  unsigned char idx_min = 0;            //  el indice del disparo a MIN vel
  unsigned char idx_max = 0;            //  el indice del disparo a MAX vel
  unsigned char show_mode = 0;          //  define el tipo de dato mostrado en el LCD

  long peso_balin = 18;        // peso en gr, sin decimales


  

  long maxFPS = 0;   //  maxima
  long minFPS = 0;   //  minima
  long avgFPS = 0;   //  promedio
  long currFPS = 0;  //  actual

// --------------------------------------------------------------------------------------




// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------



// --------------------------------------------------------------------------------------
void setup() {

  unsigned long aux1;
  
//Serial.begin(9600);

  pinMode(pinS1, INPUT);
  pinMode(pinS2, INPUT);

  

  //  inicializo los arrreglos con 0 para descartar falsas lecturas
  for(unsigned char i = 0 ; i < CANT_DISPAROS ; i++){
    tiempos[i] = 0;
  }
  //  accedo a cada elemento del array, porque sino parece que no lo inicializa
  // parece alguna optimizacion del compilador o algo asi
  for(unsigned char i = 0 ; i < CANT_DISPAROS ; i++){
    aux1 += tiempos[i] ;
  }

  // INDICAMOS QUE TENEMOS CONECTADA UNA PANTALLA DE 16X2
  lcd.begin();
  lcd.backlightOff();
  lcd.backlightLevel(35);
  lcd.backlightOn();
  // MOVER EL CURSOR A LA PRIMERA POSICION DE LA PANTALLA (0, 0) Y BORRAR
  lcd.clear();
  // IMPRIMIR EN LA PRIMERA LINEA
  lcd.print("GC Chrony");
  // MOVER EL CURSOR A LA SEGUNDA LINEA (1) PRIMERA COLUMNA (0)
  lcd.setCursor ( 0, 1 );
  // IMPRIMIR OTRA CADENA EN ESTA POSICION
  lcd.print(" -- Listo -- ");


///  Serial.println("setup listo!");
///  Serial.println("------------------------------------------");

  show_mode = 0;
  idx_show = 0;


    //  preparo el timer2
    noInterrupts();//stop interrupts
    TCCR2A = 0;// set entire TCCR2A register to 0
    TCCR2B = 0;// same for TCCR2B
    TCNT2  = 0;//initialize counter value to 0
    
    //Enable Timer2 overflow interrupt; see datasheet pg. 159-160
    TIMSK2 |= 0b00000001; //enable Timer2 overflow interrupt. (by making the right-most bit in TIMSK2 a 1)

    //set timer2 to "normal" operation mode.  See datasheet pg. 147, 155, & 157-158 (incl. Table 18-8).  
    //-This is important so that the timer2 counter, TCNT2, counts only UP and not also down.
    //-To do this we must make WGM22, WGM21, & WGM20, within TCCR2A & TCCR2B, all have values of 0.
    TCCR2A &= 0b11111100; //set WGM21 & WGM20 to 0 (see datasheet pg. 155).
    TCCR2B &= 0b11110111; //set WGM22 to 0 (see pg. 158).

    desborde = 0;

    //  deshabilito timer0 y timer1
    //  tiene conflictos con LCD
    //TCCR0A = 0;
    //TCCR0B = 0;
    TCCR0A_old = TCCR0A;
    TCCR0B_old = TCCR0B;
    TCCR1A_old = TCCR1A;
    TCCR1B_old = TCCR1B;
    
    interrupts();//allow interrupts
  

  //  interrupciones
  attachInterrupt(digitalPinToInterrupt(pinS1), sensor1, RISING);
  attachInterrupt(digitalPinToInterrupt(pinS2), sensor2, RISING);



  /// datos para testear: de 200 a 168 m/s
/*
  for(int i = 0 ; i < 10; i++){
    idx = i;
    tiempos[idx] = (int) (1000 + (3*i*i) );
  }
*/

    //  la primer lectura me da muy lenta, cada ves que reseteo el arduino me pasa eso
    //  pruebo lanzar la interrupcion 1 para ver si 
    //  asi se inicializa todo mejor y queda "atento"
  delay(300);
  sensor1();
  
    
}
// --------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------
void loop() {


   if(idx_ant < idx){
    //  actualizo los datos agregados, porque hay una lectura nueva
    calcular_datos_FPS(minFPS, maxFPS, avgFPS, currFPS, idx_ant);
    
    // @todo
    //  mostrar datos actualizados
    idx_show = idx_ant;
    show_mode = 0;
    drawLCD(show_mode, idx_show);

    idx_ant++;
  }

  lcd_key = read_LCD_buttons();
  if(lcd_key != btnNONE){
    button_to_mode(lcd_key);
    drawLCD(show_mode, idx_show);
    delay(300);
  }
  

  //  evitar si se perdio la lectura de sensor 2, que quede esperando para siempre
  if(desborde > 10000){
//    Serial.print("deborde!: "); Serial.println(desborde);
    //resetear contadores
    //  paro el timer1
    //TCCR2B &= ~(1 << CS20);    //  prescaler : 1
    TCCR2B &= ~(1 << CS21);    //  prescaler : 8
    TCNT2 = 0;
    desborde = 0;
    running = 0;
    TCCR0A = TCCR0A_old;
    TCCR0B = TCCR0B_old;
    TCCR1A = TCCR1A_old;
    TCCR1B = TCCR1B_old;
  }


    
/*
   Serial.print("sensor 1 :  ");
   Serial.print(digitalRead(pinS1));
   Serial.print("    -    sensor 2 :  ");
   Serial.println(digitalRead(pinS2));
*/
}
// --------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------
// interrupcion cuando llega al desborde del timer
ISR(TIMER2_OVF_vect){
  desborde++;
  
}
// --------------------------------------------------------------------------------------
void sensor1(){

  
  noInterrupts();
  
  //TCCR2B = (1 << CS20);     //  prescaler : 1
  TCCR2B |= (1 << CS21);    //  prescaler : 8
  
  running = ! running;

  //  deshabilito timer0
  TCCR0A = 0;
  TCCR0B = 0;
  TCCR1A = 0;
  TCCR1B = 0;
   
  interrupts();
}
// --------------------------------------------------------------------------------------
void sensor2(){


//cli();//stop interrupts
  noInterrupts();
  
  tiempo_actual = TCNT2;
  flag_save = bitRead(TIFR2,0); //grab the timer2 overflow flag value; see datasheet pg. 160
  if (flag_save) { //if the overflow flag is set
    //update variable just saved since the overflow flag could have just tripped between previously saving the TCNT2 value and reading bit 0 of TIFR2.  
    //If this is the case, TCNT2 might have just changed from 255 to 0, and so we need to grab the new value of TCNT2 to prevent an error of up 
    //to 127.5us in any time obtained using the T2 counter (ex: T2_micros). (Note: 255 counts / 2 counts/us = 127.5us)
    tiempo_actual = TCNT2;
    desborde++; //force the overflow count to increment
    TIFR2 |= 0b00000001; //reset Timer2 overflow flag since we just manually incremented above; see datasheet pg. 160; this prevents execution of Timer2's overflow ISR
  }
  tiempo_actual += (desborde * 256);
  
  
  //  paro el timer1
  //TCCR2B &= ~(1 << CS20);    //  prescaler : 1
  TCCR2B &= ~(1 << CS21);    //  prescaler : 8

  //  dejo el timer0 como estaba y timer1 tambien!
  TCCR0A = TCCR0A_old;
  TCCR0B = TCCR0B_old;
  TCCR1A = TCCR1A_old;
  TCCR1B = TCCR1B_old;
  
  interrupts();
//sei();//allow interrupts
  


  //  registro todo y reseteo contadores
  if(running){
    tiempos[idx] = tiempo_actual;
  }
  else{
    //  error de lectura, no estaba corriendo el reloj
    tiempos[idx] = 0;
  }

  //  controlo de no desbordar el buffer de lecturas
  if(idx < (CANT_DISPAROS - 1)) idx++;
  TCNT2 = 0;
  desborde = 0;
  running = 0;
  
  
  

}
// --------------------------------------------------------------------------------------





// --------------------------------------------------------------------------------------
//  dibuja la pantalla LCD, dependiendo del tipo de informacion a mostrar
void drawLCD(unsigned char modo, unsigned char idx_info){


//Serial.print(" MODO: "); Serial.println(modo);

  switch (modo)               // depending on which button was pushed, we perform an action
 {
   case modoVEL:
     {
     drawLCD_VEL(idx_info);
     break;
     }
   case modoMAX:
     {
     drawLCD_MAX(idx_info);
     break;
     }
   case modoMIN:
     {
     drawLCD_MIN(idx_info);
     break;
     }
   case modoAVG:
     {
     drawLCD_AVG(idx_info);
     break;
     }
   case modoSPREAD:
     {
     drawLCD_SPREAD(idx_info);
     break;
     }
   case modoPESO:
     {
     drawLCD_PESO(idx_info);
     break;
     }
   /*
   case modoVEL:
     {
     drawLCD_VEL(idx_info);
     break;
     }
   case modoVEL:
     {
     drawLCD_VEL(idx_info);
     break;
     }
     */
 }
  
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
void drawLCD_VEL(unsigned char idx_info){

  long vel;
///Serial.println(" drawLCD_VEL ");
  
  lcd.clear();

  //  linea 1
  lcd.setCursor(0,0);
  lcd.print("N#| FPS|M/S|Joul");

  //  linea 2
  lcd.setCursor(0,1);
  if(idx_info < 9){
    lcd.print(" ");
  }
  lcd.print(idx_info+1);
  lcd.print("|");
  
  vel = getVel_FPS(idx_info);
  if((vel < vel_min) || (vel > vel_max)){
    lcd.print("ERRO");
  }
  else{
    //  FPS
    if(vel < 1000){
      lcd.print(" ");
    }
    if(vel < 100){
      lcd.print(" ");
    }
    if(vel < 10){
      lcd.print(" ");
    }
    lcd.print(vel);

    // M/S
    vel = FPS_to_MS(vel);
    lcd.print("|");
    if(vel < 100){
      lcd.print(" ");
    }
    if(vel < 10){
      lcd.print(" ");
    }
    lcd.print(vel);
    
    //  joules
    lcd.print("|");
    double pot;
    pot = (double)((vel * vel * peso_balin) / 30865.0);
    if(pot < 99.9) lcd.print(pot, 1);
    else lcd.print(pot, 0);
/*
Serial.print(" vel: "); Serial.print(vel);
Serial.print("  pot: "); Serial.print(pot);
Serial.print("  peso: "); Serial.print(peso_balin);
Serial.println("");
*/
  }
  
}
// --------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------
void drawLCD_MAX(unsigned char idx_info){

   int vel;
//Serial.println(" drawLCD_MAX ");

  lcd.clear();

  //  linea 1
  lcd.setCursor(0,0);
  lcd.print("MAX | FPS  | M/S");

  //  linea 2
  lcd.setCursor(0,1);
  if(idx_max < 9){
    lcd.print("  ");
  }
  else{
    lcd.print(" ");
  }

  lcd.print(idx_max+1);
  lcd.print(" | ");

  vel = maxFPS;
  if(vel < 1000){
    lcd.print(" ");
  }
  if(vel < 100){
    lcd.print(" ");
  }
  if(vel < 10){
    lcd.print(" ");
  }
  lcd.print(vel);

  vel = FPS_to_MS(vel);
  lcd.print(" | ");
  if(vel < 100){
    lcd.print(" ");
  }
  if(vel < 10){
    lcd.print(" ");
  }
  lcd.print(vel);
  
}
// --------------------------------------------------------------------------------------



// --------------------------------------------------------------------------------------
void drawLCD_MIN(unsigned char idx_info){

  int vel;
  
//Serial.println(" drawLCD_MIN ");

  lcd.clear();

  //  linea 1
  lcd.setCursor(0,0);
  lcd.print("MIN | FPS  | M/S");

  //  linea 2
  lcd.setCursor(0,1);
  if(idx_min < 9){
    lcd.print("  ");
  }
  else{
    lcd.print(" ");
  }

  lcd.print(idx_min+1);
  lcd.print(" | ");

  vel = minFPS;
  if(vel < 1000){
    lcd.print(" ");
  }
  if(vel < 100){
    lcd.print(" ");
  }
  if(vel < 10){
    lcd.print(" ");
  }
  lcd.print(vel);

  vel = FPS_to_MS(vel);
  lcd.print(" | ");
  if(vel < 100){
    lcd.print(" ");
  }
  if(vel < 10){
    lcd.print(" ");
  }
  lcd.print(vel);
  
}
// --------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------
void drawLCD_AVG(unsigned char idx_info){

  int vel;
//Serial.println(" drawLCD_AVG ");

  lcd.clear();

  //  linea 1
  lcd.setCursor(0,0);
  lcd.print("AVG | FPS  | M/S");

  //  linea 2
  lcd.setCursor(0,1);
  if(idx_avg < 9){
    lcd.print("  ");
  }
  else{
    lcd.print(" ");
  }

  lcd.print(idx_avg);
  lcd.print(" | ");

  vel = avgFPS;
  if(vel < 1000){
    lcd.print(" ");
  }
  if(vel < 100){
    lcd.print(" ");
  }
  if(vel < 10){
    lcd.print(" ");
  }
  lcd.print(vel);

  vel = FPS_to_MS(vel);
  lcd.print(" | ");
  if(vel < 100){
    lcd.print(" ");
  }
  if(vel < 10){
    lcd.print(" ");
  }
  lcd.print(vel);
  
}
// --------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------
void drawLCD_SPREAD(unsigned char idx_info){

  int vel;
///Serial.println(" drawLCD_SPREAD ");

  lcd.clear();

  //  linea 1
  lcd.setCursor(0,0);
  lcd.print("SPRE| FPS  | M/S");

  //  linea 2
  lcd.setCursor(0,1);
  if(idx_avg < 9){
    lcd.print("  ");
  }
  else{
    lcd.print(" ");
  }

  lcd.print(idx_avg);
  lcd.print(" | ");

  vel = maxFPS - minFPS;
  if(vel < 1000){
    lcd.print(" ");
  }
  if(vel < 100){
    lcd.print(" ");
  }
  if(vel < 10){
    lcd.print(" ");
  }
  lcd.print(vel);

  vel = FPS_to_MS(vel);
  lcd.print(" | ");
  if(vel < 100){
    lcd.print(" ");
  }
  if(vel < 10){
    lcd.print(" ");
  }
  lcd.print(vel);
}
// --------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------
void drawLCD_PESO(unsigned char idx_info){

  int vel;
///Serial.println(" drawLCD_PESO ");

  lcd.clear();

  //  linea 1
  lcd.setCursor(0,0);
  lcd.print("Shot| Gr | Joule");

  //  linea 2
  lcd.setCursor(0,1);
  if(idx_info < 9){
    lcd.print("  ");
  }
  else{
    lcd.print(" ");
  }

  lcd.print(idx_info+1);
  lcd.print(" |");

  if(peso_balin < 100.0){
    lcd.print(" ");
  }
  if(peso_balin < 10.0){
    lcd.print(" ");
  }
  lcd.print(peso_balin);
  lcd.print(" |  ");

  vel = getVel_FPS(idx_info);
  vel = FPS_to_MS(vel);
  
  float pot;
  pot = (float) ((vel * vel * peso_balin) / 30865.0);
  lcd.print(pot, 1);
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
void button_to_mode(int lcd_key){

  switch (lcd_key){
   
   case btnRIGHT:
     {
     if(show_mode == modoTOPE) show_mode = 0;
     else show_mode++;
//     lcd.print("RIGHT ");
     break;
     }
   case btnLEFT:
     {
     if(show_mode == 0) show_mode = modoTOPE;
     else show_mode--;
//     lcd.print("LEFT   ");
     break;
     }
   case btnUP:
     {
      if(show_mode == modoPESO){
        if(peso_balin < 150) peso_balin++;
      }
      else{
        if(idx_show < (idx-1)) idx_show++;
      }
//     lcd.print("UP    ");
     break;
     }
   case btnDOWN:
     {
      if(show_mode == modoPESO){
        if(peso_balin > 0) peso_balin--;
      }
      else{
        if(idx_show > 0) idx_show--;
      }
//     lcd.print("DOWN  ");
     break;
     }
   case btnSELECT:
     {
//     lcd.print("SELECT");
       show_mode = 0;
       idx_show = idx-1;
     break;
     }
     case btnNONE:
     {
//     lcd.print("NONE  ");
     break;
     }
 }
    
    
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// read the buttons
int read_LCD_buttons(){
  
  int adc_key_in  = 0;
  
   adc_key_in = analogRead(0);      // read the value from the sensor : A0 para el shield LCD robot
   // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
   // we add approx 50 to those values and check to see if we are close
   if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
   // For V1.1 us this threshold
   if (adc_key_in < 50)   return btnRIGHT;
   if (adc_key_in < 250)  return btnUP;
   if (adc_key_in < 450)  return btnDOWN;
   if (adc_key_in < 650)  return btnLEFT;
   if (adc_key_in < 850)  return btnSELECT;
  
   // For V1.0 comment the other threshold and use the one below:
  /*
   if (adc_key_in < 50)   return btnRIGHT;
   if (adc_key_in < 195)  return btnUP;
   if (adc_key_in < 380)  return btnDOWN;
   if (adc_key_in < 555)  return btnLEFT;
   if (adc_key_in < 790)  return btnSELECT;
  */
  
  
   return btnNONE;  // when all others fail, return this...
}
// --------------------------------------------------------------------------------------




// ========================================================================================

// --------------------------------------------------------------------------------------
long getVel_FPS(unsigned char idx_info){

  long vel;
    if( tiempos[idx_info] ){
      //  datos correctos
      vel = calcular_velocidad_FPS(tiempos[idx_info]);
      if((vel < vel_min) || (vel > vel_max)){
        vel = 0;
      }
    }
    else{
      vel = 0;
    }

/*
  Serial.print("  getVEL - idx: ");
  Serial.print(idx_info);
  Serial.print("   tiempo:");
  Serial.print(tiempos[idx_info]);
  Serial.print("   vel:");
  Serial.println(vel);
  */  
    return vel;
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
long getVel_MS(unsigned char idx_info){

    long vel;
    vel = getVel_FPS(idx_info);
    return FPS_to_MS(vel);
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
//  en realidad, calculo la velocidad con t2 solamente, que tiene el tiempo total desde el primer evento
long calcular_velocidad_FPS(unsigned long t1){

  //  t1 : tiempo , cada unidad es 0.0625 us
  // vel = (dist/tiempo) * frec reloj
  //long vel = (long)  (( (long_pie/t1) * 16000000 ) + 0.5);  //  presaler 1

  //  t1 : tiempo , cada unidad es 0.5 us
  long vel = (long)  (( (long_pie/t1) * 2000000 ) + 0.5);  //  prescaler 8
  

  return vel;
}
// --------------------------------------------------------------------------------------
long FPS_to_MS(long fps){
  return ( (long) ((fps/3.281) + 0.5) );
}
// --------------------------------------------------------------------------------------
void calcular_datos_FPS(long& minVel, long& maxVel, long& avgVel, long& currVel, unsigned char& idx_act){

//Serial.println("  Calculo de datos: ");
//Serial.println(" idx   lectura 1      lectura 2  ");
//Serial.print(idx_act);Serial.print("     ");Serial.print(tiempos[idx_act]);Serial.print("  ");Serial.print(lectura2[idx_act]);
//Serial.println("");

    if( tiempos[idx_act] ){
      //  datos correctos
      currVel = calcular_velocidad_FPS(tiempos[idx_act]);
      if((currVel < vel_min) || (currVel > vel_max)){
        //descartar
        return;
      }
      idx_avg++;  //  incremento la cantidad de lecturas correctas
      if((minVel < vel_min) || (currVel < minVel)){
        minVel = currVel;
        idx_min = idx_act;
      }
      if(currVel > maxVel){
        maxVel = currVel;
        idx_max = idx_act;
      }

      avgVel =  (long) ( ( ( (avgVel * (idx_avg-1) ) + currVel ) / idx_avg ) + 0.5);
    }
  
}
// --------------------------------------------------------------------------------------

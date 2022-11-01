//incluindo biblioteca do motor de passos
#include <Stepper.h>

//instanciando e configurando os pinos do motor de passos, stepsPerRevolution = 2050, portanto 1º grau equivale a aproximadamente 6 passos
Stepper myStepper(stepsPerRevolution,10,11,12,13)

//incluindo a biblioteca do LCD
#include <LiquidCrystal_I2C.h> 
//SDA --> A4, SCL --> A5

//instanciando, colocando o endereço do LCD e o numero de colunas e linhas
LiquidCrystal_I2C lcd(0x3F,16,2);

//definindo os pins do encoder
int RotCLK_01 = 3;                //pino que fica o CLK do encoder
int RotDT_01 = 4;                 //pino que fica o DT do encoder 
int PushButton_01 = 8;            //pino que fica o SW do encoder

//definindo os pins do encoder
int RotCLK_02 = 2;                //pino que fica o CLK do encoder
int RotDT_02 = 5;                 //pino que fica o DT do encoder 
int PushButton_02 = 9;            //pino que fica o SW do encoder


int lastCount_Az = 0;
int lastCount_Alt = 0;


//definindo as variaveis do azimute e da altitude
volatile int az_Value = 0;                                   //valor do ângulo azimutal que vai até 180º
volatile int alt_Value = 0;                                  //valor do ângulo da altitude que vai até 90º


//---------------------------------------------------------------Variaveis para o motor de passos----------------------------------------------------------------------------------

const int stepsPerRevolution = 2050;                         //quantidade de passos em uma volta
volatile boolean rot_direction == false;                     // se for true então o encoder está mexendo na direção horária, caso contrário está mexendo na direção anti-horária
volatile boolean turnDetected_Az == false;                   //detecta se o motor do eixo azimutal vai girar


//---------------------------------------------------------------Variaveis para o motor de passos-----------------------------------------------------------------------------


// ------------------------------------------------------------------
// INTERRUPT     INTERRUPT     INTERRUPT     INTERRUPT     INTERRUPT
// ------------------------------------------------------------------

void rot_Az(){
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();

  //Se as interrupções vierem mais rápido que 5ms, assume que é um salto e ignora
  if(interruptTime - lastInterruptTime > 5){
    
    //se ambos pinos tem o mesmo estado LOW, então o movimento é dado na direção anti-horária
    if(digitalRead(RotDT_01)== LOW){
      az_Value--;
      
    }else{
      
      az_Value++;

      //encoder girou na direção horária, então nossa variavél que indica a direção para o motor vai receber o valor true
      rot_direction == true;
    }

    // Restringe valor indo de 0 a +100
    az_Value = min(180, max(0, az_Value));
    
    turnDetected_Az == true;                                            
  }
  
  // mantendo armazenado quando passamos por aqui (não mais que 5ms)
  lastInterruptTime = interruptTime;
}

void rot_Al(){
  static unsigned long lastIntptTime = 0;
  unsigned long inteptTime = millis();

  //Se as interrupções vierem mais rápido que 5ms, assume que é um salto e ignora
  if(inteptTime - lastIntptTime > 5){
    
    //se ambos pinos tem o mesmo estado LOW, então o movimento é dado na direção anti-horária
    if(digitalRead(RotDT_02)== LOW){
      alt_Value--;
    }else{
      alt_Value++;
    }

    // Restringe valor indo de 0 a +100
    alt_Value = min(90, max(0, alt_Value));
  }
  
  // mantendo armazenado quando passamos por aqui (não mais que 5ms)
  lastIntptTime = inteptTime;
}

// ------------------------------------------------------------------
// SETUP    SETUP    SETUP    SETUP    SETUP    SETUP    SETUP
// ------------------------------------------------------------------
void setup(){

  //configurando a velocidade do motor
  myStepper.setSpeed(5);

  //pra acompanhar no monitor serial e fazer o debug
  Serial.begin(9600);

  // os pulsos do encoder são INPUTs
  pinMode(RotCLK_01, INPUT);
  pinMode(RotDT_01, INPUT);

  pinMode(RotCLK_02, INPUT);
  pinMode(RotDT_02, INPUT);

  //no switch usamos o PullUp para não precisarmos de resistor na protoboard
  pinMode(PushButton_01, INPUT_PULLUP);
  pinMode(PushButton_02, INPUT_PULLUP);

  
  //---------------------------------------------------------------------------LCD-----------------------------------------------------------------
  //Initializing the LCD
  lcd.init();
  lcd.backlight();

  //Adjusting the LCD
  lcd.setCursor(0,0);      //Defining position to write from the first row, first column.
  lcd.print("Bem-vindo");
  lcd.setCursor(0,1);      //Cursor goes to second row, first column
  lcd.print("ABCD-Teste");
  delay(5000);

  lcd.clear(); //clear the LCD's whole screen and positions the cursor in the upper-left corner

  printLCD();  //calling a function that print the stationary parts on the screen
  //---------------------------------------------------------------------------LCD-----------------------------------------------------------------

  //criando o attachInterrupt que será responsavel por chamar o rot_Az() E rot_Al()
  attachInterrupt(digitalPinToInterrupt(RotCLK_01), rot_Az, LOW);
  attachInterrupt(digitalPinToInterrupt(RotCLK_02), rot_Al, LOW);

  // Ready to go!
  Serial.println("Start");

}

// ------------------------------------------------------------------
// MAIN LOOP     MAIN LOOP     MAIN LOOP     MAIN LOOP     MAIN LOOP
// ------------------------------------------------------------------
void loop() {

  // checando se o botão do encoder foi pressionado
  //!digitalRead(pin) --> devolve HIGH se lê LOW
  if ((!digitalRead(PushButton_01))) {
    rot_motor = az_Value;
    az_Value = 0;
    while (!digitalRead(PushButton_01))
      delay(10);
    Serial.println("Reset");
  }

  
  if (az_Value != lastCount_Az) {

    // escrevendo no monitor serial os valores e a direção
    Serial.print(az_Value > lastCount_Az ? "Up  :" : "Down:");
    Serial.println(az_Value);

    updateValue_Az();

    //guardando esse valor
    lastCount_Az = az_Value ;
  }

  
  if (alt_Value != lastCount_Alt) {

    //escrevendo no monitor serial os valores e a direção
    Serial.print(alt_Value > lastCount_Alt ? "Up  :" : "Down:");
    Serial.println(alt_Value);

    updateValue_Alt();

    //guardando esse valor
    lastCount_Alt = alt_Value ;
  }

  if(turnDetected_Az == true){
    
    StepsToMove = az_Value*6;
    
    if(rot_direction == true;){
      myStepper.step(StepsToMove);
    }else{
      myStepper.step(-StepsToMove);
    }
    delay(20);
  }
  //delay(100);
}

void printLCD()
{
  //valores que não mudam
  
  lcd.setCursor(1,0); //1st line, 2nd block
  lcd.print("Azi_Value"); //text
  //----------------------
  lcd.setCursor(1,1); //2nd line, 2nd block
  lcd.print("Lat_Value"); //text
  
  
}

void updateValue_Az()
{  
  lcd.setCursor(12,0); //1º linha line, 13º coluna
  lcd.print("   "); //apagando o conteúdo printando um espaço em branco nele
  lcd.setCursor(12,0); //1º linha line, 13º coluna
  lcd.print(az_Value); //printando o valor da variavel que contém o ângulo azimutal
  
}

void updateValue_Alt(){
  
  lcd.setCursor(12,1);
  lcd.print("   ");
  lcd.setCursor(12,1);
  lcd.print(alt_Value); //printando o valor da variavel que contém o ângulo da altitude
}

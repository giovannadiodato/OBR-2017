#include <Arduino.h>
#include <Wire.h>


/*
  FUNÇÕES de leitura DO ROBÔ
*/

/*int lerQTR(int pino) {
  pino = pino - 1;
  qtra.read(sensorValues);
  return sensorValues[pino];
  }*/


void MPUMath() {
  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
  Yaw = (ypr[0] * 180 / M_PI);
  Pitch = (ypr[1] * 180 / M_PI);
  Roll = (ypr[2] * 180 / M_PI);
}


void GetDMP() { // Best version I have made so far
  // Serial.println(F("FIFO interrupt at:"));
  // Serial.println(micros());
  mpuInterrupt = false;
  FifoAlive = 1;
  fifoCount = mpu.getFIFOCount();
  /*
    fifoCount is a 16-bit unsigned value. Indicates the number of bytes stored in the FIFO buffer.
    This number is in turn the number of bytes that can be read from the FIFO buffer and it is
    directly proportional to the number of samples available given the set of sensor data bound
    to be stored in the FIFO
  */

  // PacketSize = 42; refference in MPU6050_6Axis_MotionApps20.h Line 527
  // FIFO Buffer Size = 1024;
  uint16_t MaxPackets = 20;// 20*42=840 leaving us with  2 Packets (out of a total of 24 packets) left before we overflow.
  // If we overflow the entire FIFO buffer will be corrupt and we must discard it!

  // At this point in the code FIFO Packets should be at 1 99% of the time if not we need to look to see where we are skipping samples.
  if ((fifoCount % packetSize) || (fifoCount > (packetSize * MaxPackets)) || (fifoCount < packetSize)) { // we have failed Reset and wait till next time!
    digitalWrite(LED_PIN, LOW); // lets turn off the blinking light so we can see we are failing.
    Serial.println(F("Reset FIFO"));
    if (fifoCount % packetSize) Serial.print(F("\t Packet corruption")); // fifoCount / packetSize returns a remainder... Not good! This should never happen if all is well.
    Serial.print(F("\tfifoCount ")); Serial.print(fifoCount);
    Serial.print(F("\tpacketSize ")); Serial.print(packetSize);

    mpuIntStatus = mpu.getIntStatus(); // reads MPU6050_RA_INT_STATUS       0x3A
    Serial.print(F("\tMPU Int Status ")); Serial.print(mpuIntStatus , BIN);
    // MPU6050_RA_INT_STATUS       0x3A
    //
    // Bit7, Bit6, Bit5, Bit4          , Bit3       , Bit2, Bit1, Bit0
    // ----, ----, ----, FIFO_OFLOW_INT, I2C_MST_INT, ----, ----, DATA_RDY_INT

    /*
      Bit4 FIFO_OFLOW_INT: This bit automatically sets to 1 when a FIFO buffer overflow interrupt has been generated.
      Bit3 I2C_MST_INT: This bit automatically sets to 1 when an I2C Master interrupt has been generated. For a list of I2C Master interrupts, please refer to Register 54.
      Bit1 DATA_RDY_INT This bit automatically sets to 1 when a Data Ready interrupt is generated.
    */
    if (mpuIntStatus & B10000) { //FIFO_OFLOW_INT
      Serial.print(F("\tFIFO buffer overflow interrupt "));
    }
    if (mpuIntStatus & B1000) { //I2C_MST_INT
      Serial.print(F("\tSlave I2c Device Status Int "));
    }
    if (mpuIntStatus & B1) { //DATA_RDY_INT
      Serial.print(F("\tData Ready interrupt "));
    }
    Serial.println();
    //I2C_MST_STATUS
    //PASS_THROUGH, I2C_SLV4_DONE,I2C_LOST_ARB,I2C_SLV4_NACK,I2C_SLV3_NACK,I2C_SLV2_NACK,I2C_SLV1_NACK,I2C_SLV0_NACK,
    mpu.resetFIFO();// clear the buffer and start over
    mpu.getIntStatus(); // make sure status is cleared we will read it again.
  } else {
    while (fifoCount  >= packetSize) { // Get the packets until we have the latest!
      if (fifoCount < packetSize) break; // Something is left over and we don't want it!!!
      mpu.getFIFOBytes(fifoBuffer, packetSize); // lets do the magic and get the data
      fifoCount -= packetSize;
    }
    MPUMath(); // <<<<<<<<<<<<<<<<<<<<<<<<<<<< On success MPUMath() <<<<<<<<<<<<<<<<<<<
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Blink the Light
    if (fifoCount > 0) mpu.resetFIFO(); // clean up any leftovers Should never happen! but lets start fresh if we need to. this should never happen.
  }
}


int lerSensorVerde(int lado, int rgb) {


  // LER SENSOR VERDE DA ESQUERDA
  if (lado == ESQUERDA) {
    if (rgb == R) {

      // Reading R
      digitalWrite(Left_s2, LOW);
      digitalWrite(Left_s3, LOW);

      Left_green = pulseIn(Left_out, digitalRead(Left_out) == HIGH ? LOW : HIGH);
      //Left_green = map(Left_green, 25, 70, 255, 0);

    }
    if (rgb == G) {

      // Reading G
      digitalWrite(Left_s2, HIGH);
      digitalWrite(Left_s3, HIGH);

      Left_green = pulseIn(Left_out, digitalRead(Left_out) == HIGH ? LOW : HIGH);
      //Left_green = map(Left_green, 30, 90, 255, 0);

    }

    if (rgb == B) {

      // Reading B
      digitalWrite(Left_s2, LOW);
      digitalWrite(Left_s3, HIGH);

      Left_green = pulseIn(Left_out, digitalRead(Left_out) == HIGH ? LOW : HIGH);
      //Left_green = map(Left_green, 25, 70, 255, 0);


    }
    /*if (Left_green < 0) {
      Left_green = 0;
    } else if (Left_green > 255) {
      Left_green = 255;
    }*/
    //delay(100);
    return Left_green;
    
  }


  // LER SENSOR VERDE DA DIREITA
  if (lado == DIREITA) {

    if (rgb == R) {

      // Reading R
      digitalWrite(Right_s2, LOW);
      digitalWrite(Right_s3, LOW);

      Right_green = pulseIn(Right_out, digitalRead(Right_out) == HIGH ? LOW : HIGH);
      //Right_green = map(Right_green, 15, 90, 255, 0);


    }
    if (rgb == G) {

      // Reading G
      digitalWrite(Right_s2, HIGH);
      digitalWrite(Right_s3, HIGH);

      Right_green = pulseIn(Right_out, digitalRead(Right_out) == HIGH ? LOW : HIGH);
      //Right_green = map(Right_green, 15, 101, 255, 0);

    }

    if (rgb == B) {

      // Reading B
      digitalWrite(Right_s2, LOW);
      digitalWrite(Right_s3, HIGH);

      Right_green = pulseIn(Right_out, digitalRead(Right_out) == HIGH ? LOW : HIGH);
      //Right_green = map(Right_green, 25, 70, 255, 0);


    }
    /*if (Right_green < 0) {
      Right_green = 0;
    } else if (Right_green > 255) {
      Right_green = 255;
    }*/
    //delay(50);
    return Right_green;



  }

}



int lerTodosQTR() {
  unsigned int position = qtra.readLine(sensorValues);
  return position;
}

unsigned int lerQTR(int pino) {

  //qtra.read(sensorValues);
  return analogRead(QTR[pino]);
  //return sensorValues[pino-1];

}

int lerSharp(int pino) {
  return analogRead(Sharp[pino]);
}

int lerSharpDigital(int pino) {

  /*if (pino == 4) {
    return digitalRead(Sharp_Digital[pino]);
    }*/
  if (digitalRead(Sharp_Digital[pino]) == 1) {
    return 0;
  } else if (digitalRead(Sharp_Digital[pino]) == 0) {
    return 1;
  }
}

int lerBtnRedutor() {
  return digitalRead(BTN_REDUTOR);
}
// sensors 0 through 5 are connected to analog inputs 0 through 5, respectively
double lendoMpuAccel() {

  if (mpuInterrupt) { // wait for MPU interrupt or extra packet(s) available
    GetDMP(); // Gets the MPU Data and canculates angles
  }
  //*****************************************************************************************************************************************************************************
  //************************************              Put any code you want to use the values that come from your MPU6050 here               ************************************
  //*****************************************************************************************************************************************************************************

  static long QTimer = millis();
  if ((long)( millis() - QTimer ) >= 100) {
    QTimer = millis();
    // Serial.print(F("\t Yaw")); Serial.print(Yaw);
    //Serial.print(F("\t Pitch ")); Serial.print(Pitch);
    // Serial.print(F("\t Roll ")); Serial.print(Roll);
    // Serial.println();
  }
  return Roll;

}
double lendoMpuGyro() {

  if (mpuInterrupt) { // wait for MPU interrupt or extra packet(s) available
    GetDMP(); // Gets the MPU Data and canculates angles
  }
  //*****************************************************************************************************************************************************************************
  //************************************              Put any code you want to use the values that come from your MPU6050 here               ************************************
  //*****************************************************************************************************************************************************************************

  static long QTimer = millis();
  if ((long)( millis() - QTimer ) >= 100) {
    QTimer = millis();
    //Serial.print(F("\t Yaw")); Serial.print(Yaw);
    //Serial.print(F("\t Pitch ")); Serial.print(Pitch);
    //Serial.print(F("\t Roll ")); Serial.print(Roll);
    //Serial.println();
  }
  return Yaw;

}

String allsensors;

void lerTodosSensores() {
  
  allsensors = "";
  /*allsensors += lerQTR(1);
  allsensors += " ";
  allsensors += lerQTR(2);
  allsensors += " ";
  allsensors += lerQTR(3);
  allsensors += " ";
  allsensors += lerQTR(4);
  allsensors += " ";
  allsensors += lerQTR(5);
  allsensors += " ";
  allsensors += lerQTR(6);
  allsensors += " ";
  allsensors += lerQTR(7);
  allsensors += " ";
  allsensors += lerQTR(8);
  */
  allsensors += " / ESQUERDA R: ";
    allsensors += lerSensorVerde(ESQUERDA, R);
    allsensors += " / ESQUERDA G: ";
    allsensors += lerSensorVerde(ESQUERDA, G);
    allsensors += " / ESQUERDA B: ";
    allsensors += lerSensorVerde(ESQUERDA, B);
    allsensors += " / DIREITA R: ";
    allsensors += lerSensorVerde(DIREITA, R);
    allsensors += " / DIREITA G: ";
    allsensors += lerSensorVerde(DIREITA, G);
    allsensors += " / DIREITA B: ";
    allsensors += lerSensorVerde(DIREITA, B);
  /*allsensors += " / Botao: ";
  allsensors += lerBtnRedutor();*/
  /*allsensors += " / Todos QTR: ";
  allsensors += lerTodosQTR();*/


  Serial.println(allsensors);
  Serial.println();
  //delay(250);
}


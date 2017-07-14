/***************************************************************************
 * 
 *          FOR MAX31865
 * 
* File Name: serial_MAX31865.h
* Processor/Platform: Arduino Uno R3 (tested)
* Development Environment: Arduino 1.0.5
*
* Designed for use with with Playing With Fusion MAX31865 Resistance
* Temperature Device (RTD) breakout board: SEN-30202 (PT100 or PT1000)
*   ---> http://playingwithfusion.com/productview.php?pdid=25
*   ---> http://playingwithfusion.com/productview.php?pdid=26
*
* Copyright © 2014 Playing With Fusion, Inc.
* SOFTWARE LICENSE AGREEMENT: This code is released under the MIT License.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
* **************************************************************************
* REVISION HISTORY:
* Author      Date    Comments
* J. Steinlage      2014Feb17 Original version
* J. Steinlage                  2014Aug07       Reduced SPI clock to 1MHz - fixed occasional missing bit
*                                               Fixed temp calc to give only unsigned resistance values
*                                               Removed unused #defines for chip config (they're hard coded)
*
* Playing With Fusion, Inc. invests time and resources developing open-source
* code. Please support Playing With Fusion and continued open-source
* development by buying products from Playing With Fusion!
*
* **************************************************************************
* ADDITIONAL NOTES:
* This file configures then runs a program on an Arduino Uno to read a 2-ch
* MAX31865 RTD-to-digital converter breakout board and print results to
* a serial port. Communication is via SPI built-in library.
*    - Configure Arduino Uno
*    - Configure and read resistances and statuses from MAX31865 IC
*      - Write config registers (MAX31865 starts up in a low-power state)
*      - RTD resistance register
*      - High and low status thresholds
*      - Fault statuses
*    - Write formatted information to serial port
*  Circuit:
*    Arduino Uno   Arduino Mega  -->  SEN-30201
*    CS0: pin  9   CS0: pin  9   -->  CS, CH0
*    CS1: pin 10   CS1: pin 10   -->  CS, CH1
*    MOSI: pin 11  MOSI: pin 51  -->  SDI (must not be changed for hardware SPI)
*    MISO: pin 12  MISO: pin 50  -->  SDO (must not be changed for hardware SPI)
*    SCK:  pin 13  SCK:  pin 52  -->  SCLK (must not be changed for hardware SPI)
*    GND           GND           -->  GND
*    5V            5V            -->  Vin (supply with same voltage as Arduino I/O, 5V)
**************************************************************************************/


/*************************************************************************************
 * 
 *              FOR PID Autotune Library 
 * 
 * 
 *
 * Arduino PID AutoTune Library - Version 0.0.1
 * by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
 *
 * This Library is ported from the AutotunerPID Toolkit by William Spinelli
 * (http://www.mathworks.com/matlabcentral/fileexchange/4652) 
 * Copyright (c) 2004
 *
 * This Library is licensed under the BSD License:
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are 
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the distribution
 *       
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************************/

/**************************************************************************************
 * 
 *              FOR ARDUINO PID LIBRARY
 * 
* Arduino PID Library - Version 1.1.1
* by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
*
* This Library is licensed under a GPLv3 License


 - For an ultra-detailed explanation of why the code is the way it is, please visit: 
   http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/

 - For function documentation see:  http://playground.arduino.cc/Code/PIDLibrary
**************************************************************************************/
//Include hardware SPI library to enable commnication by SPI
#include <SPI.h>

//Include the PID library
#include <PID_v1.h>

//Include the PID autotuner library
#include <PID_AutoTune_v0.h>

#define MOSFET_COOLING_FAN_1 5 //PWM
#define MOSFET_COOLING_FAN_2 7 //PWM

/***************************************************************************
* BEGIN: Pneumatic controller initialisation
***************************************************************************/
#define VAC_PUMP_1 3 //PWM
#define VAC_PUMP_2 6 //PWM
#define LIQUID_PUMP_BIG 2 //PWM
#define LIQUID_PUMP_180 4 //PWM
#define SOLENOID_1 31
#define SOLENOID_2 29
#define SOLENOID_3 27
#define SOLENOID_4 25
#define SOLENOID_5 23

/***************************************************************************
* END: Pneumatic/Liquid controller initialisation
***************************************************************************/

/***************************************************************************
* BEGIN: Thermo controller initialisation
***************************************************************************/
//#define DNA_EXTRACTION_HEATER 10 //PWM
//#define DNA_EXTRACTION_COOLLING 13 //PWM
#define DNA_HYDROLYSIS_HEATER 8 //PWM
//#define DERIVATISATION_HEATER 9 //PWM
//#define DERIVATISATION_COOLER 12 //PWM

/***************************************************************************
* END: Thermo controller initialisation
***************************************************************************/

/***************************************************************************
* BEGIN: Dual MAX31865 PT-1000 RTD to Digital Breakout initialisation
***************************************************************************/
// The board communicates using SPI, include Playing With Fusion MAX31865
// libraries
#include <PlayingWithFusion_MAX31865.h>              // core library
#include <PlayingWithFusion_MAX31865_STRUCT.h>       // struct library

// CS pins used for the connection with the MAX31865 board
// other connections are controlled by the SPI library)
#define RTD_CHIP_SELECT0 45
#define RTD_CHIP_SELECT1 43
#define RTD_CHIP_SELECT2 41
#define RTD_CHIP_SELECT3 39
#define RTD_CHIP_SELECT4 37
#define RTD_CHIP_SELECT5 35


PWFusion_MAX31865_RTD rtd_ch0(RTD_CHIP_SELECT0);
PWFusion_MAX31865_RTD rtd_ch1(RTD_CHIP_SELECT1);
PWFusion_MAX31865_RTD rtd_ch2(RTD_CHIP_SELECT2);
PWFusion_MAX31865_RTD rtd_ch3(RTD_CHIP_SELECT3);
PWFusion_MAX31865_RTD rtd_ch4(RTD_CHIP_SELECT4);
PWFusion_MAX31865_RTD rtd_ch5(RTD_CHIP_SELECT5);
/***************************************************************************
* END: Dual MAX31865 PT-1000 RTD to Digital Breakout initialisation
***************************************************************************/


/***************************************************************************
 * BEGIN: Global variables to store values to be used for PID control algorithms
 * 
 * DNA_ex_heat = DNA extraction heater
 * 
 ***************************************************************************/
//Temperature values read from RTD sensors embedded in the PDMS device on each temperature actuator
//This will be the input to our PID compute function
double DNA_extraction_heater_tmp, DNA_extraction_cooler_tmp, DNA_hydrolysis_heater_tmp, derivatisation_heater_tmp, derivatisation_cooler_tmp;

//Define variables we'll be connecting to the PID object
double DNA_ex_heater_Setpoint, DNA_ex_heater_output; //OUT IS PWM SO HIGH OR LOW and for HOW LONG? DEPENDS ON FREQUENCY OF PID_COMPUTE FUNCTION
/*
//Define the aggressive and conservative Tuning Parameters
double DNA_ex_heat_aggKp=4, DNA_ex_heat_aggKi=0.2, DNA_ex_heat_aggKd=1;
double DNA_ex_heat_consKp=1, DNA_ex_heat_consKi=0.05, DNA_ex_heat_consKd=0.25;
*/

//Specify the links and initial tuning parameters
//PID myPID(&DNA_extraction_heater_tmp, &DNA_ex_heater_output, &DNA_ex_heater_Setpoint, DNA_ex_heat_consKp, DNA_ex_heat_consKi, DNA_ex_heat_consKd, DIRECT);

//Define the PID tuning Parameters
double Kp=5, Ki=10, Kd=8;

//Specify the PID links
PID myPID(&DNA_extraction_heater_tmp, &DNA_ex_heater_output, &DNA_ex_heater_Setpoint, Kp, Ki, Kd, DIRECT);

int WindowSize = 2;          //Set a 1 sec window size for PWM PID
unsigned long windowStartTime;

/***************************************************************************
 * END: Global variables to store values to be used for control algorithms
***************************************************************************/
 
 

long previous_temp_read_time = 0; //Store last time temperature was read (milliseconds)
long temp_read_interval = 1000; //Interval at which temperature is read (milliseconds)

void setup()
{
    /***************************************************************************
     * BEGIN: Dual MAX31865 PT-1000 RTD setup
     ***************************************************************************/
    Serial.begin(115200); //The value in the serial monitor window must match this
 
    // setup for the the SPI library:
    SPI.begin();                            // begin SPI
    SPI.setClockDivider(SPI_CLOCK_DIV16);   // SPI speed to SPI_CLOCK_DIV16 (1MHz)
    SPI.setDataMode(SPI_MODE3);             // MAX31865 works in MODE1 or MODE3

    // Initalize the Dual MAX31865 PT-1000 chip select pins
    pinMode(RTD_CHIP_SELECT0, OUTPUT);
    pinMode(RTD_CHIP_SELECT1, OUTPUT);
    pinMode(RTD_CHIP_SELECT2, OUTPUT);
    pinMode(RTD_CHIP_SELECT3, OUTPUT);
    pinMode(RTD_CHIP_SELECT4, OUTPUT);
    pinMode(RTD_CHIP_SELECT5, OUTPUT);

    rtd_ch0.MAX31865_config();
    rtd_ch1.MAX31865_config();
    rtd_ch2.MAX31865_config();
    rtd_ch3.MAX31865_config();
    rtd_ch4.MAX31865_config();
    rtd_ch5.MAX31865_config();

    // give the Dual MAX31865 PT-1000 sensors time to set up *****PUT THIS DELAY FURTHER UP AND TAKE INTO ACCOUNT WHAT OTHER COMPONENTS ARE DOING, DONT REPLICATE DELAYS
    delay(1000);
    /***************************************************************************
     * END: Dual MAX31865 PT-1000 RTD setup
     ***************************************************************************/
    
    // Setup pneumatic, liquid, and thermo controller components as outputs 
    pinMode(VAC_PUMP_1, OUTPUT);
    pinMode(VAC_PUMP_2, OUTPUT);
    pinMode(LIQUID_PUMP_180, OUTPUT);
    pinMode(LIQUID_PUMP_BIG, OUTPUT);
    /*pinMode(SOLENOID_1, OUTPUT);
    pinMode(SOLENOID_2, OUTPUT);
    pinMode(SOLENOID_3, OUTPUT);
    pinMode(SOLENOID_4, OUTPUT);
    pinMode(SOLENOID_5, OUTPUT);*/
    pinMode(DNA_HYDROLYSIS_HEATER, OUTPUT);
    //pinMode(DNA_EXTRACTION_HEATER, OUTPUT);
    //pinMode(DNA_EXTRACTION_COOLLING, OUTPUT);
    //pinMode(DERIVATISATION_HEATER, OUTPUT);
    //pinMode(DERIVATISATION_COOLER, OUTPUT);
    pinMode(MOSFET_COOLING_FAN_1, OUTPUT);
    pinMode(MOSFET_COOLING_FAN_2, OUTPUT);

    //Turn MOSFET cooling fans on immediately
    digitalWrite(MOSFET_COOLING_FAN_2,HIGH);
    digitalWrite(MOSFET_COOLING_FAN_1,HIGH);


    DNA_ex_heater_Setpoint = 35.9;

    windowStartTime = millis();    //get current time

    //tell the PID to range between 0 and the full window size
    myPID.SetOutputLimits(0, WindowSize);

    //turn the PID on
    myPID.SetMode(AUTOMATIC);

    
    Serial.println("Time(sec) \tHydrolysis Heater \t\tDerivatisation Cooler \t\tDNA Extraction Heater \t\tUNUSED \t\tDNA Extraction Cooler \t\tDerivatisation Heater");

}// void setup

void loop()
{
    //digitalWrite(LIQUID_PUMP_BIG,HIGH);
    //digitalWrite(LIQUID_PUMP_180,HIGH);

    //Read RTD temp here and set DNA_extraction_heater_tmp
    ReadRTDTemperatures();// Read and print RTD temperatures to serialout
    myPID.Compute();

    unsigned long now = millis();  //get now time
  
    if(now - windowStartTime > WindowSize)
    { //time to shift the Relay Window
        windowStartTime += WindowSize;
    }
    if(DNA_ex_heater_output > now - windowStartTime) 
        digitalWrite(DNA_HYDROLYSIS_HEATER,HIGH);   //ON
    else 
        digitalWrite(DNA_HYDROLYSIS_HEATER,LOW);    //OFF



    
    unsigned long current_time = now; //the amount of time that has elapsed since the board was switched on
    if(now - previous_temp_read_time > temp_read_interval)
    {
        // Print current time in seconds, move cursor to next column
        Serial.print(current_time/1000);Serial.print("\t\t");
        ReadRTDTemperatures();// Read and print RTD temperatures to serialout
        Serial.println(DNA_extraction_heater_tmp);                          // print RTD2 temp
        Serial.print("\t\t\t\t");
        
        previous_temp_read_time = current_time;
    }// if
}// void loop


void ReadRTDTemperatures()
{
    static struct var_max31865 RTD_CH0;
    static struct var_max31865 RTD_CH1;
    static struct var_max31865 RTD_CH2;
    static struct var_max31865 RTD_CH3;
    static struct var_max31865 RTD_CH4;
    static struct var_max31865 RTD_CH5;
    
    double tmp;
    
    struct var_max31865 *rtd_ptr;
    rtd_ptr =&RTD_CH0;
    rtd_ch0.MAX31865_full_read(rtd_ptr);          // Update MAX31855 readings
    
    rtd_ptr = &RTD_CH1;
    rtd_ch1.MAX31865_full_read(rtd_ptr);          // Update MAX31855 readings
    
    rtd_ptr = &RTD_CH2;
    rtd_ch2.MAX31865_full_read(rtd_ptr);          // Update MAX31855 readings
    
    rtd_ptr = &RTD_CH3;
    rtd_ch3.MAX31865_full_read(rtd_ptr);          // Update MAX31855 readings
    
    rtd_ptr = &RTD_CH4;
    rtd_ch4.MAX31865_full_read(rtd_ptr);          // Update MAX31855 readings
    
    rtd_ptr = &RTD_CH5;
    rtd_ch5.MAX31865_full_read(rtd_ptr);          // Update MAX31855 readings
    
    /*
    // ******************** Print RTD 0 Information ********************
    if(0 == RTD_CH0.status)                       // no fault, print info to serial port
    {
        // calculate RTD temperature (simple calc, +/- 2 deg C from -100C to 100C)
        // more accurate curve can be used outside that range
        tmp = ((double)RTD_CH0.rtd_res_raw / 32) - 256;
        
        Serial.print(tmp);                          // print RTD1 temp
        Serial.print("\t\t\t\t");
    }  // end of no-fault handling
    else
    {
        Serial.print("RTD Fault, register: ");
        Serial.print(RTD_CH0.status);
        if(0x80 & RTD_CH0.status)
    {
        Serial.println("RTD High Threshold Met");  // RTD high threshold fault
    }
    else if(0x40 & RTD_CH0.status)
    {
        Serial.println("RTD Low Threshold Met");   // RTD low threshold fault
    }
    else if(0x20 & RTD_CH0.status)
    {
        Serial.println("REFin- > 0.85 x Vbias");   // REFin- > 0.85 x Vbias
    }
    else if(0x10 & RTD_CH0.status)
    {
        Serial.println("FORCE- open");             // REFin- < 0.85 x Vbias, FORCE- open
    }
    else if(0x08 & RTD_CH0.status)
    {
        Serial.println("FORCE- open");             // RTDin- < 0.85 x Vbias, FORCE- open
    }
    else if(0x04 & RTD_CH0.status)
    {
        Serial.println("Over/Under voltage fault");  // overvoltage/undervoltage fault
    }
    else
    {
        Serial.println("Unknown fault, check connection"); // print RTD temperature heading
    }
    }  // end of fault handling
    
    
    // ******************** Print RTD 1 Information ********************
    //Serial.println("RTD Sensor 1:");              // Print RTD0 header
    
    if(0 == RTD_CH1.status)                       // no fault, print info to serial port
    {
    // calculate RTD temperature (simple calc, +/- 2 deg C from -100C to 100C)
    // more accurate curve can be used outside that range
    tmp = ((double)RTD_CH1.rtd_res_raw / 32) - 256;
    
    Serial.print(tmp);                          // print RTD2 temp
    Serial.print("\t\t\t\t");
    }  // end of no-fault handling
    else
    {
    Serial.print("RTD Fault, register: ");
    Serial.print(RTD_CH1.status);
    if(0x80 & RTD_CH1.status)
    {
    Serial.println("RTD High Threshold Met");  // RTD high threshold fault
    }
    else if(0x40 & RTD_CH1.status)
    {
    Serial.println("RTD Low Threshold Met");   // RTD low threshold fault
    }
    else if(0x20 & RTD_CH1.status)
    {
    Serial.println("REFin- > 0.85 x Vbias");   // REFin- > 0.85 x Vbias
    }
    else if(0x10 & RTD_CH1.status)
    {
    Serial.println("FORCE- open");             // REFin- < 0.85 x Vbias, FORCE- open
    }
    else if(0x08 & RTD_CH1.status)
    {
    Serial.println("FORCE- open");             // RTDin- < 0.85 x Vbias, FORCE- open
    }
    else if(0x04 & RTD_CH1.status)
    {
    Serial.println("Over/Under voltage fault");  // overvoltage/undervoltage fault
    }
    else
    {
    Serial.println("Unknown fault, check connection"); // print RTD temperature heading
    }
    }  // end of fault handling

*/
    
    // ******************** Print RTD 2 Information ********************
    //Serial.println("RTD Sensor 1:");              // Print RTD0 header
    
    if(0 == RTD_CH2.status)                       // no fault, print info to serial port
    {
    // calculate RTD temperature (simple calc, +/- 2 deg C from -100C to 100C)
    // more accurate curve can be used outside that range
    DNA_extraction_heater_tmp = ((double)RTD_CH2.rtd_res_raw / 32) - 256;
    
    }  // end of no-fault handling
    else
    {
    Serial.print("RTD Fault, register: ");
    Serial.print(RTD_CH2.status);
    if(0x80 & RTD_CH2.status)
    {
    Serial.println("RTD High Threshold Met");  // RTD high threshold fault
    }
    else if(0x40 & RTD_CH2.status)
    {
    Serial.println("RTD Low Threshold Met");   // RTD low threshold fault
    }
    else if(0x20 & RTD_CH2.status)
    {
    Serial.println("REFin- > 0.85 x Vbias");   // REFin- > 0.85 x Vbias
    }
    else if(0x10 & RTD_CH2.status)
    {
    Serial.println("FORCE- open");             // REFin- < 0.85 x Vbias, FORCE- open
    }
    else if(0x08 & RTD_CH2.status)
    {
    Serial.println("FORCE- open");             // RTDin- < 0.85 x Vbias, FORCE- open
    }
    else if(0x04 & RTD_CH2.status)
    {
    Serial.println("Over/Under voltage fault");  // overvoltage/undervoltage fault
    }
    else
    {
    Serial.println("Unknown fault, check connection"); // print RTD temperature heading
    }
    }  // end of fault handling

    /*
    // ******************** Print RTD 3 Information ********************
    //Serial.println("RTD Sensor 1:");              // Print RTD0 header
    
    if(0 == RTD_CH3.status)                       // no fault, print info to serial port
    {
    // calculate RTD temperature (simple calc, +/- 2 deg C from -100C to 100C)
    // more accurate curve can be used outside that range
    tmp = ((double)RTD_CH3.rtd_res_raw / 32) - 256;
    
    Serial.print(tmp);                          // print RTD2 temp
    Serial.print("\t\t\t\t");
    }  // end of no-fault handling
    else
    {
    Serial.print("RTD Fault, register: ");
    Serial.print(RTD_CH3.status);
    if(0x80 & RTD_CH3.status)
    {
    Serial.println("RTD High Threshold Met");  // RTD high threshold fault
    }
    else if(0x40 & RTD_CH3.status)
    {
    Serial.println("RTD Low Threshold Met");   // RTD low threshold fault
    }
    else if(0x20 & RTD_CH3.status)
    {
    Serial.println("REFin- > 0.85 x Vbias");   // REFin- > 0.85 x Vbias
    }
    else if(0x10 & RTD_CH3.status)
    {
    Serial.println("FORCE- open");             // REFin- < 0.85 x Vbias, FORCE- open
    }
    else if(0x08 & RTD_CH3.status)
    {
    Serial.println("FORCE- open");             // RTDin- < 0.85 x Vbias, FORCE- open
    }
    else if(0x04 & RTD_CH3.status)
    {
    Serial.println("Over/Under voltage fault");  // overvoltage/undervoltage fault
    }
    else
    {
    Serial.println("Unknown fault, check connection"); // print RTD temperature heading
    }
    }  // end of fault handling

    
    // ******************** Print RTD 4 Information ********************
    //Serial.println("RTD Sensor 1:");              // Print RTD0 header
    
    if(0 == RTD_CH4.status)                       // no fault, print info to serial port
    {
    // calculate RTD temperature (simple calc, +/- 2 deg C from -100C to 100C)
    // more accurate curve can be used outside that range
    tmp = ((double)RTD_CH4.rtd_res_raw / 32) - 256;
    
    Serial.print(tmp);                          // print RTD2 temp
    Serial.print("\t\t\t\t");
    }  // end of no-fault handling
    else
    {
    Serial.print("RTD Fault, register: ");
    Serial.print(RTD_CH4.status);
    if(0x80 & RTD_CH4.status)
    {
    Serial.println("RTD High Threshold Met");  // RTD high threshold fault
    }
    else if(0x40 & RTD_CH4.status)
    {
    Serial.println("RTD Low Threshold Met");   // RTD low threshold fault
    }
    else if(0x20 & RTD_CH4.status)
    {
    Serial.println("REFin- > 0.85 x Vbias");   // REFin- > 0.85 x Vbias
    }
    else if(0x10 & RTD_CH4.status)
    {
    Serial.println("FORCE- open");             // REFin- < 0.85 x Vbias, FORCE- open
    }
    else if(0x08 & RTD_CH4.status)
    {
    Serial.println("FORCE- open");             // RTDin- < 0.85 x Vbias, FORCE- open
    }
    else if(0x04 & RTD_CH4.status)
    {
    Serial.println("Over/Under voltage fault");  // overvoltage/undervoltage fault
    }
    else
    {
    Serial.println("Unknown fault, check connection"); // print RTD temperature heading
    }
    }  // end of fault handling
    
    // ******************** Print RTD 5 Information ********************
    //Serial.println("RTD Sensor 1:");              // Print RTD0 header
    
    if(0 == RTD_CH5.status)                       // no fault, print info to serial port
    {
    // calculate RTD temperature (simple calc, +/- 2 deg C from -100C to 100C)
    // more accurate curve can be used outside that range
    tmp = ((double)RTD_CH5.rtd_res_raw / 32) - 256;
    
    Serial.print(tmp);                          // print RTD2 temp
    Serial.println("\t\t\t\t");
    }  // end of no-fault handling
    else
    {
    Serial.print("RTD Fault, register: ");
    Serial.print(RTD_CH5.status);
    if(0x80 & RTD_CH5.status)
    {
    Serial.println("RTD High Threshold Met");  // RTD high threshold fault
    }
    else if(0x40 & RTD_CH5.status)
    {
    Serial.println("RTD Low Threshold Met");   // RTD low threshold fault
    }
    else if(0x20 & RTD_CH5.status)
    {
    Serial.println("REFin- > 0.85 x Vbias");   // REFin- > 0.85 x Vbias
    }
    else if(0x10 & RTD_CH5.status)
    {
    Serial.println("FORCE- open");             // REFin- < 0.85 x Vbias, FORCE- open
    }
    else if(0x08 & RTD_CH5.status)
    {
    Serial.println("FORCE- open");             // RTDin- < 0.85 x Vbias, FORCE- open
    }
    else if(0x04 & RTD_CH5.status)
    {
    Serial.println("Over/Under voltage fault");  // overvoltage/undervoltage fault
    }
    else
    {
    Serial.println("Unknown fault, check connection"); // print RTD temperature heading
    }
    }  // end of fault handling
    
    tmp = 0;*/
}// void temperatureLogger()


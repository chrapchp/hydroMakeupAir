

/**
*  @file    hydroMakeupAir.ino
*  @author  peter c
*  @date    11/09/2017
*  @version 0.1
*
*
*  @section DESCRIPTION
*  Makeup Air  and Exhaust Air share the same I/O. Makeup Air used as the template
*
** @section HISTORY
** 2017Nov9 - created
*/
#include <HardwareSerial.h>

#include <Streaming.h>

#include <DHT.h> // DHT-22 humidity sensor
#include <DA_Analoginput.h>
#include <DA_Discreteinput.h>
#include <DA_DiscreteOutput.h>
#include <DA_DiscreteOutputTmr.h>
#include <DA_HOASwitch.h>
#include <flowmeter.h>
#include <DA_NonBlockingDelay.h>


#include "UnitModbus.h"
// comment out to  include terminal processing for debugging
 //#define PROCESS_TERMINAL
 //#define TRACE_1WIRE
//#define TRACE_ANALOGS
 //define TRACE_FLOW_SENSOR
// #define TRACE_DISCRETES
//#define TRACE_MODBUS_COILS
//#define TRACE_MODBUS_HR
// comment//out to disable modbus
#define PROCESS_MODBUS
// refresh intervals
#define POLL_CYCLE_SECONDS 2 // sonar and 1-wire refresh rate






// 
// 
// /#define FAN_SPEED_INTERUPT   2 // pin does not work return 0 for now to host

#define HUMIDITY_INTAKE_FEED 9 // pin


// DHT-22 - one wire type humidity sensor (won't work with one wire lib)
DHT B1R1_1A_AT_003_DHT = DHT(HUMIDITY_INTAKE_FEED, DHT22);
float B1R1_1A_AT_003 = NAN; // or B1R1-1A-AT-007 for right
float B1R1_1A_TT_010 = NAN; // or B1R1-1A-TT-014 for right


DA_AnalogInput B1R1_1A_PDT_004 = DA_AnalogInput(A3, 0.0, 1023.); // min max
DA_AnalogInput B1R1_1A_PDT_005 = DA_AnalogInput(A3, 0.0, 1023.); // min max


// PWM control
#define B1R1_1A_SY_001_PIN 10 // Pin 10 for PWM to control fan speed
unsigned short B1R1_1A_SY_001 = 0; // current duty cycle to write to fan





// HEARTBEAT
unsigned int heartBeat = 0;


// poll I/O every 2 seconds
DA_NonBlockingDelay pollTimer = DA_NonBlockingDelay( POLL_CYCLE_SECONDS*1000, &doOnPoll);



#ifdef PROCESS_TERMINAL
HardwareSerial *tracePort = &Serial;
#endif




void setup()
{

#ifdef PROCESS_TERMINAL
  tracePort -> begin(9600);
#endif

#ifdef PROCESS_MODBUS
  slave.begin(MB_SPEED);
#endif

  pinMode(B1R1_1A_SY_001, OUTPUT);
  randomSeed(analogRead(3));


  // humidity sensors start
  B1R1_1A_AT_003_DHT.begin();


}

void loop()
{

#ifdef PROCESS_MODBUS
  refreshModbusRegisters();
  slave.poll(modbusRegisters, MODBUS_REG_COUNT);
  processModbusCommands();
#endif
  pollTimer.refresh();



  analogWrite(B1R1_1A_SY_001_PIN, B1R1_1A_SY_001);
}

// update sonar and 1-wire DHT-22 readings
void doOnPoll()
{
  doReadAnalogs();
  B1R1_1A_AT_003 = B1R1_1A_AT_003_DHT.readHumidity();
  B1R1_1A_TT_010 = B1R1_1A_AT_003_DHT.readTemperature();

   #ifdef TRACE_1WIRE
  *tracePort << "B1R1_1A_PDT_001:" << B1R1_1A_AT_003  ; 
  *tracePort << " B1R1_1A_TT_010:" << B1R1_1A_TT_010  ;

  #endif
  heartBeat++;
}



void doReadAnalogs()
{
  B1R1_1A_PDT_004.refresh();

  B1R1_1A_PDT_005.refresh();

#ifdef TRACE_ANALOGS
  B1R1_1A_PDT_004.serialize(tracePort, true);
    B1R1_1A_PDT_005.serialize(tracePort, true);

#endif

}

// 
/*
** Modbus related functions
*/

#ifdef PROCESS_MODBUS
void refreshModbusRegisters()
{

  modbusRegisters[B1R1_1A_PDT_004_MB] =  B1R1_1A_PDT_004.getRawSample();
    modbusRegisters[B1R1_1A_PDT_005_MB] = B1R1_1A_PDT_005.getRawSample();

  modbusRegisters[B1R1_1A_AT_003_MB] =  B1R1_1A_AT_003 * 100;
  modbusRegisters[B1R1_1A_TT_010_MB] = B1R1_1A_TT_010 * 100;



  //modbusRegisters[HR_FLOW1] = B1R1_1A_FT_001.getCurrentPulses();
  modbusRegisters[B1R1_1A_ST_001_MB] =  -1; // not implemented
    modbusRegisters[HEART_BEAT] = heartBeat;



}


void processSetFanSpeedCommand()
{
  B1R1_1A_SY_001 = modbusRegisters[B1R1_1A_SY_001_MB];
    #ifdef TRACE_MODBUS_HR
  *tracePort << "Set Fan Speed :" << B1R1_1A_SY_001 << endl;
  #endif
}

void processModbusCommands()
{

  processSetFanSpeedCommand();
}

#endif

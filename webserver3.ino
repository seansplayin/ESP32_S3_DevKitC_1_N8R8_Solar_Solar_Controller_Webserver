// Compiles successfully on ESP32-S3_DevkitC_1,
// Built from Webserver_080723-5.2- uses Ethernet and now properly uses Webserver to display webpage however Webbuttons are not working.
// pins for SDIO Adafruit 4682: DET=38, DAT2=39, D1=40, D3=41, CMD=42, D0=2, clk=1
// 4682 pins  D1>21,  D0>47,  CLK>39, CMD>40,  D3>41,   D2>42,   DET>38
// 01-27-24; began updating this sketch with new code added to ESP32_S3_DevkitC_1_N8R8_Solar_Controller_4.9 which includes new programming for addition of Boiler into system. 
/*
CS = Cable Select
SDO = Serial Data Out (also known as MISO). This is the data signal that is output from the master device to the slave device.
SDI = Serial Data In (also known as MOSI). This is the data signal that is input from the slave device to the master device.
SCK = Serial Clock (also known as SCLK). This is the clock signal that synchronizes the data transfer between the master and slave devices.
The terms MOSI and MISO are outdated and are being replaced by SDO and SDI. However, you will still see the terms MOSI and MISO used in many 
places, so it is important to be familiar with both sets of terms.
*/

// SDA pin 8, SCL pin 9
#include "uptime_formatter.h"
#include <NTPClient_Generic.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Time.h>

 // Time and Date
   WiFiUDP ntpUDP;
   NTPClient timeClient(ntpUDP);
   #define TIME_ZONE_OFFSET_HRS            (-6)
   unsigned long lastSyncTime = 0;  // Variable to store the last synchronization time
   const unsigned long syncInterval = 86400000;  // Synchronization interval 86400000 of 24 hours

#define VERSION_INFO " -Webserver3- "

#include <SPI.h>
#include <WebServer_ESP32_SC_W5500.h>
#include <Adafruit_MAX31865.h>

WebServer server(80); // Initialize the WebServer

  
//Define a variable to store the last loop execution time:
  unsigned long lastLoopTime = 5000; 

  // Flowmeter
  #include <FlowMeter.h>  // https://github.com/sekdiy/FlowMeter

  FlowSensorProperties MySensor1 = {45.0f, 8.10f, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}}; 
  FlowSensorProperties MySensor2 = {45.0f, 7.95f, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}}; 
  FlowSensorProperties MySensor3 = {45.0f, 7.95f, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}}; 
  FlowSensorProperties MySensor4 = {45.0f, 7.95f, {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}}; 

// connect a flow meter to an interrupt pin (see notes on your Arduino model for pin numbers)
  FlowMeter *Meter1;
  FlowMeter *Meter2;
  FlowMeter *Meter3;
  FlowMeter *Meter4;
  // set the measurement update period to 1s (1000 ms)
  const unsigned long period = 1000;

// define an 'interrupt service handler' (ISR) for every interrupt pin you use
  void Meter1ISR() {
  // let our flow meter count the pulses
  Meter1->count();
  }

// define an 'interrupt service handler' (ISR) for every interrupt pin you use
  void Meter2ISR() {
  // let our flow meter count the pulses
  Meter2->count();
  }

// define an 'interrupt service handler' (ISR) for every interrupt pin you use
  void Meter3ISR() {
  // let our flow meter count the pulses
  Meter3->count();
  }

// define an 'interrupt service handler' (ISR) for every interrupt pin you use
  void Meter4ISR() {
  // let our flow meter count the pulses
  Meter4->count();
  }

  float DTemp1, DTemp2, DTemp3, DTemp4, DTemp5, DTemp6, DTemp7, DTemp8, DTemp9, DTemp10, DTemp11, DTemp12, DTemp13;
  float DTemp1Average, DTemp2Average, DTemp3Average, DTemp4Average, DTemp5Average, DTemp6Average, DTemp7Average, DTemp8Average, DTemp9Average, DTemp10Average, DTemp11Average, DTemp12Average, DTemp13Average;

  float outsideT;  //Outdoor Temperature
  float storageT;  //Temperature Reading of the inner shell in the 600 Gallon Tank
  float dhwT;  //Temperature probe outlet of DHW heater.
  float CircReturnT; //water returing from the circulating pump
  float supplyT; //water going to the collectors
  float CreturnT; //water returing from the collectors
  float DhwSupplyT; //water going to the DHW
  float DhwReturnT; //water returning from the DHW
  float HeatingSupplyT; //water going to the heating loop
  float HeatingReturnT; //water returing from the heating loop
  float CSupplyT; //water going to collectors read from after the pumps
  float pt1000Average;  //Average value of the last 10 pt1000 readings
  float panelT;  //Temperature Sensor inside collector manifold currently assigned to PT1000Average
  float panelTminimum; //minimum temperature manifold must achieve to turn on lead pump
  float CollectorTemperatureRise; //Temperature rise of water going through collector manifold
  float DhwTemperatureDrop;  //Temperature drop of water through the Domestic Hot Water Loop
  float HeatingTemperatureDrop;  //Temperature drop of water going through the heating loop
  float CirculationTemperatureDrop;  //Temperature drop of water leaving the 600 gallon tank and returning
  float StoraqeHeatingLimit; //Temperaturee to cut off heating of the 600 gallon tank
  float PotHeatXinletT; // Water Temperature coming into the heat exchanger  
  float PotHeatXoutletT; // Water Temperature coming out of the heat exchanger 

  // Define pins
  const int ONE_WIRE_BUS = 35;   // One Wire Bus for DS18B20 Temperature Sensors
  const int ONE_WIRE_BUS_2 = 36; // One Wire Bus 2 for DS18B20 Temperature Sensors
  #define PANEL_LEAD_PUMP_RELAY  6
  #define PANEL_LAG_PUMP_RELAY   7
  #define HEAT_TAPE_RELAY        14
  #define Circ_Pump_Relay        15
  #define DHW_PUMP_RELAY         16
  #define STORAGE_HEAT_RELAY     17
  #define BOILER_CIRC_RELAY      47
  #define FURNACE_HEATING_PIN    48
  #define RECIRC_VALVE_RELAY     35
  #define DHW_HEATING_PIN        36
  #define W5500_MOSI             11  // Define the SPI2 (HSPI) pins for the W5500 Ethernet Adapter 
  #define W5500_MISO             13  // Define the SPI2 (HSPI) pins for the W5500 Ethernet Adapter 
  #define W5500_SCLK             12  // Define the SPI2 (HSPI) pins for the W5500 Ethernet Adapter 
  #define W5500_CS               10  // Define the SPI2 (HSPI) pins for the W5500 Ethernet Adapter 
  #define W5500_INT              4   // Define the SPI2 (HSPI) pins for the W5500 Ethernet Adapter 
  #define ETH_SPI_HOST           SPI2_HOST  // Define the SPI2 (HSPI) pins for the W5500 Ethernet Adapter 
  #define MAX31865_CS_PIN        5   // spi Cable Select
  #define MAX31865_DO_PIN        19  // spi MOSI
  #define MAX31865_DI_PIN        45  // spi MISO
  #define MAX31865_CLK_PIN       18  // spi Clock
 
// Operating Parameters for circulation pumps
  #define PanelOnDifferential     30  // Panels must be this much warmer than sUpply to turn on pumps
  #define PanelLowDifferential    15   //  If Panels are only this much warmer, run slower
  #define PanelOffDifferential    3   //  If Panels are only this much warmer, turn off
  #define HotWaterOnDifferential  1  // Storage must be this much warmer than dhw to turn on pump
  #define HotWaterOffDifferential 0.1   // If Storage is only this much warmer than dhw, turn off pump
  #define StorageTooCold          125  // If Storage isn't hot enough, don't let heat exch pump run
  #define StoraqeHeatingLimit     175  //stop heating the 600gallon tank at this temperature
  #define DHW_HIGHTEMP_LIMIT      150 // shut off DHW Pump when this temperature is reached in DHW storage tank
  #define HIGHTEMP_LIMIT          200 // shut off when this temperature is reached in storage tank
  #define Circ_Pump_On            5  // temperature difference when circulation pump comes on
  #define Circ_Pump_Off           4   // temperature difference when circulation pump turns off
  #define panelTminimum           125 // lead pump on criteria, minimum manifold temp in addition to collector vs supply requirement
  #define Heat_Tape_On            35 // Temperature when heat tape turns on
  #define Heat_Tape_Off           45 // Temperature when heat tape turns off
  #define Boiler_Circ_On          100 // Temperature at which a call for heat from DHW or Heating will also bring on boiler circulator.
  #define Boiler_Circ_Off         105 // Temperature at which a call for heat from DHW or Heating will NOT also bring on boiler circulator.

//Temp Delta calculations for flow meters 
  #define Collector_Temperature_Rise ((CreturnT) - (supplyT))
  #define DHT_Temp_Drop ((DhwSupplyT) - (DhwReturnT))
  #define Heating_Temp_Drop ((HeatingSupplyT) - (HeatingReturnT))
  #define Circ_Loop_Temp_Drop ((supplyT) - (CircReturnT))
  
//Circ_Loop_Differential
  #define CollectorTemperatureRise ((CreturnT) - (supplyT))
  #define DhwTemperatureDrop ((DhwSupplyT) - (DhwReturnT))
  #define HeatingTemperatureDrop ((HeatingSupplyT) - (HeatingReturnT))
  #define CirculationTemperatureDrop ((supplyT) - (CircReturnT)) 

// Flowmeter declarations 
  #define meter1volume  (Meter1->getCurrentFlowrate())
  #define meter2volume  (Meter2->getCurrentFlowrate())
  #define meter3volume  (Meter3->getCurrentFlowrate())
  #define meter4volume  (Meter4->getCurrentFlowrate())

// Alarm state values
  #define ALARM_OFF   0
  #define ALARM_HOT  2


 int ALARM = ALARM_OFF;  // Alarm value, set to off initially
  String alarmMessage1;   // Alarm message in 2 lines to fit LCD
  String alarmMessage2;

// pump state values
  #define PUMP_OFF   0
  #define PUMP_ON    1
  #define PUMP_AUTO  2

// pump state - 0 off, 1 on, 2 auto as controlled by ButtonControl or web page function
  int state_panel_lead   = PUMP_AUTO;   // default to auto allows temperature logic to control pumps
  int state_panel_lag    = PUMP_AUTO;
  int state_dhw          = PUMP_AUTO;
  int state_heat         = PUMP_AUTO;
  int state_circ         = PUMP_AUTO;
  int state_heat_tape    = PUMP_AUTO;
  int state_boiler_circ  = PUMP_AUTO;
  int state_recirc_valve = PUMP_AUTO;

 
 // One Wire Bus Settings -
 // setup digital oneWire sensor addresses for system
   DeviceAddress DSensor1 =  {0x28, 0x37, 0x16, 0x49, 0xF6, 0x0D, 0x3C, 0x2D};
   DeviceAddress DSensor2 =  {0x28, 0x69, 0x9A, 0x48, 0xF6, 0x7A, 0x3C, 0xAD};
   DeviceAddress DSensor3 =  {0x28, 0x52, 0x16, 0x96, 0xF0, 0x01, 0x3C, 0x02};
   DeviceAddress DSensor6 =  {0x28, 0x2A, 0x84, 0x96, 0xF0, 0x01, 0x3C, 0x0F};
   DeviceAddress DSensor5 =  {0x28, 0xE5, 0x91, 0x96, 0xF0, 0x01, 0x3C, 0x1A};
   DeviceAddress DSensor4 =  {0x28, 0x85, 0xEA, 0x81, 0xE3, 0x2B, 0x3C, 0xF2};
   DeviceAddress DSensor7 =  {0x28, 0x29, 0x1A, 0x81, 0xE3, 0x53, 0x3C, 0xB5};
   DeviceAddress DSensor8 =  {0x28, 0x22, 0xC6, 0x81, 0xE3, 0x17, 0x3C, 0x92};
   DeviceAddress DSensor9 =  {0x28, 0x66, 0xF1, 0x81, 0xE3, 0xD2, 0x3C, 0xA5};
   DeviceAddress DSensor10 =  {0x28, 0x22, 0x14, 0x81, 0xE3, 0xDC, 0x3C, 0xD8};
   DeviceAddress DSensor11 =  {0x28, 0xF1, 0x15, 0x48, 0xF6, 0xCD, 0x3C, 0x75};
   DeviceAddress DSensor12 =  {0x28, 0xDD, 0x9B, 0x96, 0xF0, 0x01, 0x3C, 0xEB};
   DeviceAddress DSensor13 =  {0x28, 0x37, 0xE2, 0x48, 0xF6, 0xE6, 0x3C, 0x4D};
 
   /* 
   // setup digital oneWire sensor addresses for desktop probes
   DeviceAddress DSensor1 =  {0x28, 0x5D, 0x4F, 0x57, 0x04, 0xA7, 0x3C, 0xA1};
   DeviceAddress DSensor2 =  {0x28, 0x37, 0xE2, 0x48, 0xF6, 0xE6, 0x3C, 0x4D};
   DeviceAddress DSensor3 =  {0x28, 0x04, 0x35, 0x57, 0x04, 0x69, 0x3C, 0xA1};
   DeviceAddress DSensor4 =  {0x28, 0xDC, 0xFD, 0x57, 0x04, 0x35, 0x3C, 0xE2};
   DeviceAddress DSensor5 =  {0x28, 0xE4, 0xFE, 0x57, 0x04, 0xF6, 0x3C, 0x01};
   DeviceAddress DSensor6 =  {0x28, 0xD0, 0xBB, 0x57, 0x04, 0x91, 0x3C, 0x3F};
   DeviceAddress DSensor7 =  {0x28, 0xE7, 0x76, 0x57, 0x04, 0xF2, 0x3C, 0x96};
   DeviceAddress DSensor8 =  {0x28, 0xFB, 0xD1, 0x57, 0x04, 0x33, 0x3C, 0xA2};
   DeviceAddress DSensor9 =  {0x28, 0x9E, 0xE2, 0x57, 0x04, 0xC8, 0x3C, 0xFA};
   DeviceAddress DSensor10 =  {0x28, 0xC0, 0x45, 0x57, 0x04, 0x54, 0x3C, 0x2B};
   DeviceAddress DSensor11 =  {0x28, 0x7C, 0xC9, 0x81, 0xE3, 0x6F, 0x3C, 0x00};
   DeviceAddress DSensor6 =  {0x28, 0xDD, 0x9B, 0x96, 0xF0, 0x01, 0x3C, 0xEB};
   DeviceAddress DSensor12 =  {0x28, 0xDD, 0x9B, 0x96, 0xF0, 0x01, 0x3C, 0xEB};
   DeviceAddress DSensor13 =  {0x28, 0x37, 0xE2, 0x48, 0xF6, 0xE6, 0x3C, 0x4D};
   */


 // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
   OneWire oneWire(ONE_WIRE_BUS);
   OneWire oneWire2(ONE_WIRE_BUS_2);

 // Pass our oneWire reference to Dallas Temperature.
   DallasTemperature sensors(&oneWire);
   DallasTemperature sensors2(&oneWire2);



 // Create an instance of the MAX31865 library with the specified pins
 //Adafruit_MAX31865 thermo = Adafruit_MAX31865(5, 19, 45, 18); use for manifold rtd
   Adafruit_MAX31865 thermo = Adafruit_MAX31865(MAX31865_CS_PIN, MAX31865_DO_PIN, MAX31865_DI_PIN,  MAX31865_CLK_PIN);

 // Define the reference resistance and nominal resistance of the PT1000 sensor
   #define RREF      4300.0
   #define RNOMINAL  1000.0

 // Function to read the PT1000 sensor and return the temperature in Fahrenheit
   float pt1000() {
   uint16_t rtd = thermo.readRTD();
   float ratio = rtd;
   ratio /= 32768;
   float resistance = RREF*ratio; 
   float pt1000 = thermo.temperature(RNOMINAL, RREF);

   return (pt1000 * 1.8) + 32;  // Convert Celsius to 
  }

  // Define the number of readings to average for pt1000
   #define pt1000NumReadings 10

  // Define an array to store the readings
   float pt1000Values[pt1000NumReadings];

  // Define a variable to keep track of the index of the current reading
   int pt1000Index = 0;

// Function to calculate and return the rolling average of the last 10 readings from pt1000 in Fahrenheit
   float pt1000AverageValue() {
   float sum = 0;
   int count = 0;
   for (int i = 0; i < pt1000NumReadings; i++) {
      sum += pt1000Values[i];
      count++;
   }
   return sum / count;
  }

  // Function to calculate and return the rolling average of the last 10 readings from DTemp1 Through DTemp13

   // Define the number of readings to average for DTemp1
     #define DTemp1NumReadings 3

     // Define an array to store the readings
     float DTemp1Values[DTemp1NumReadings];

     // Define a variable to keep track of the index of the current reading
     int DTemp1Index = 0;

     // Function to calculate and return the rolling average of the last 10 readings
     float DTemp1AverageValue() {
     float sum = 0;
     int count = 0;
     for (int i = 0; i < DTemp1NumReadings; i++) {
     if (DTemp1Values[i] > -100) { // exclude values below threshold
      sum += DTemp1Values[i];
      count++;
     }
     }
     return sum / count;
    }

   // Define the number of readings to average for DTemp2
     #define DTemp2NumReadings 3

     // Define an array to store the readings
     float DTemp2Values[DTemp2NumReadings];

     // Define a variable to keep track of the index of the current reading
     int DTemp2Index = 0;

     // Function to calculate and return the rolling average of the last 10 readings
     float DTemp2AverageValue() {
     float sum = 0;
     int count = 0;
     for (int i = 0; i < DTemp2NumReadings; i++) {
     if (DTemp2Values[i] > -100) { // exclude values below threshold
      sum += DTemp2Values[i];
      count++;
     }
     }
     return sum / count + 4.85;
    }

   // Define the number of readings to average for DTemp3
     #define DTemp3NumReadings 3

     // Define an array to store the readings
     float DTemp3Values[DTemp3NumReadings];

     // Define a variable to keep track of the index of the current reading
     int DTemp3Index = 0;

     // Function to calculate and return the rolling average of the last 10 readings
     float DTemp3AverageValue() {
     float sum = 0;
     int count = 0;
     for (int i = 0; i < DTemp3NumReadings; i++) {
     if (DTemp3Values[i] > -100) { // exclude values below threshold
      sum += DTemp3Values[i];
      count++;
     }
     }
     return sum / count;
    }

   // Define the number of readings to average for DTemp4
     #define DTemp4NumReadings 3

     // Define an array to store the readings
     float DTemp4Values[DTemp4NumReadings];

     // Define a variable to keep track of the index of the current reading
     int DTemp4Index = 0;

     // Function to calculate and return the rolling average of the last 10 readings
     float DTemp4AverageValue() {
     float sum = 0;
     int count = 0;
     for (int i = 0; i < DTemp4NumReadings; i++) {
     if (DTemp4Values[i] > -100) { // exclude values below threshold
      sum += DTemp4Values[i];
      count++;
     }
     }
     return sum / count + 1.4;
    }

   // Define the number of readings to average for DTemp5
     #define DTemp5NumReadings 3

     // Define an array to store the readings
     float DTemp5Values[DTemp5NumReadings];

     // Define a variable to keep track of the index of the current reading
     int DTemp5Index = 0;

     // Function to calculate and return the rolling average of the last 10 readings
     float DTemp5AverageValue() {
     float sum = 0;
     int count = 0;
     for (int i = 0; i < DTemp5NumReadings; i++) {
     if (DTemp5Values[i] > -100) { // exclude values below threshold
      sum += DTemp5Values[i];
      count++;
     }
     }
     return sum / count;
    }

   // Define the number of readings to average for DTemp6
     #define DTemp6NumReadings 3

     // Define an array to store the readings
     float DTemp6Values[DTemp6NumReadings];

     // Define a variable to keep track of the index of the current reading
     int DTemp6Index = 0;

     // Function to calculate and return the rolling average of the last 10 readings
     float DTemp6AverageValue() {
     float sum = 0;
     int count = 0;
     for (int i = 0; i < DTemp6NumReadings; i++) {
     if (DTemp6Values[i] > -100) { // exclude values below threshold
      sum += DTemp6Values[i];
      count++;
     }
     }
     return sum / count + 0.55;
    }

   // Define the number of readings to average for DTemp7
   #define DTemp7NumReadings 3

   // Define an array to store the readings
     float DTemp7Values[DTemp7NumReadings];

     // Define a variable to keep track of the index of the current reading
     int DTemp7Index = 0;

     // Function to calculate and return the rolling average of the last 10 readings
     float DTemp7AverageValue() {
     float sum = 0;
     int count = 0;
     for (int i = 0; i < DTemp7NumReadings; i++) {
     if (DTemp7Values[i] > -100) { // exclude values below threshold
      sum += DTemp7Values[i];
      count++;
     }
     }
     return sum / count + 0.2;
    }

   // Define the number of readings to average for DTemp8
     #define DTemp8NumReadings 3

     // Define an array to store the readings
     float DTemp8Values[DTemp8NumReadings];

     // Define a variable to keep track of the index of the current reading
     int DTemp8Index = 0;

     // Function to calculate and return the rolling average of the last 10 readings
     float DTemp8AverageValue() {
     float sum = 0;
     int count = 0;
     for (int i = 0; i < DTemp8NumReadings; i++) {
     if (DTemp8Values[i] > -100) { // exclude values below threshold
      sum += DTemp8Values[i];
      count++;
     }
     }
     return sum / count + 0.70;
    }

   // Define the number of readings to average for DTemp9
     #define DTemp9NumReadings 3

     // Define an array to store the readings
     float DTemp9Values[DTemp9NumReadings];

     // Define a variable to keep track of the index of the current reading
     int DTemp9Index = 0;

     // Function to calculate and return the rolling average of the last 10 readings
     float DTemp9AverageValue() {
     float sum = 0;
     int count = 0;
     for (int i = 0; i < DTemp9NumReadings; i++) {
     if (DTemp9Values[i] > -100) { // exclude values below threshold
      sum += DTemp9Values[i];
      count++;
     }
     }
     return sum / count + 1;
    }

   // Define the number of readings to average for DTemp10
     #define DTemp10NumReadings 3

     // Define an array to store the readings
     float DTemp10Values[DTemp10NumReadings];

     // Define a variable to keep track of the index of the current reading
     int DTemp10Index = 0;

     // Function to calculate and return the rolling average of the last 10 readings
     float DTemp10AverageValue() {
     float sum = 0;
     int count = 0;
     for (int i = 0; i < DTemp10NumReadings; i++) {
     if (DTemp10Values[i] > -100) { // exclude values below threshold
      sum += DTemp10Values[i];
      count++;
     }
     }
     return sum / count + 0.50;
    }

   // Define the number of readings to average for DTemp11
     #define DTemp11NumReadings 3

     // Define an array to store the readings
     float DTemp11Values[DTemp11NumReadings];

     // Define a variable to keep track of the index of the current reading
     int DTemp11Index = 0;

     // Function to calculate and return the rolling average of the last 10 readings
     float DTemp11AverageValue() {
     float sum = 0;
     int count = 0;
     for (int i = 0; i < DTemp11NumReadings; i++) {
     if (DTemp11Values[i] > -100) { // exclude values below threshold
      sum += DTemp11Values[i];
      count++;
     }
     }
     return sum / count + 0.3;
    }

   // Define the number of readings to average for DTemp12
     #define DTemp12NumReadings 3

     // Define an array to store the readings
     float DTemp12Values[DTemp12NumReadings];

     // Define a variable to keep track of the index of the current reading
     int DTemp12Index = 0;

     // Function to calculate and return the rolling average of the last 10 readings
     float DTemp12AverageValue() {
     float sum = 0;
     int count = 0;
     for (int i = 0; i < DTemp12NumReadings; i++) {
      if (DTemp12Values[i] > -100) { // exclude values below threshold
      sum += DTemp12Values[i];
      count++;
     }
     }
     return sum / count - 0.4;
    }

   // Define the number of readings to average for DTemp13
     #define DTemp13NumReadings 3

     // Define an array to store the readings
     float DTemp13Values[DTemp13NumReadings];

     // Define a variable to keep track of the index of the current reading
     int DTemp13Index = 0;

     // Function to calculate and return the rolling average of the last 10 readings
     float DTemp13AverageValue() {
     float sum = 0;
     int count = 0;
     for (int i = 0; i < DTemp13NumReadings; i++) {
     if (DTemp13Values[i] > -100) { // exclude values below threshold
      sum += DTemp13Values[i];
      count++;
     }
     }
     return sum / count - 0.3;
     }

  // Digital read of DS18B20 one-wire bus sensors

   void ReadDigitalTemperatures()
   {
    
   // First bus get temperatures from outside devices
     sensors.requestTemperatures();
     DTemp1 = sensors.getTempF(DSensor1);
     DTemp2 = sensors.getTempF(DSensor2);
     DTemp3 = sensors.getTempF(DSensor3);
     DTemp4 = sensors.getTempF(DSensor4);
     DTemp5 = sensors.getTempF(DSensor5);
     DTemp6 = sensors.getTempF(DSensor6);
     
  // Second bus get temperatures from inside devices
     sensors2.requestTemperatures();
     DTemp7 = sensors2.getTempF(DSensor7);
     DTemp8 = sensors2.getTempF(DSensor8);
     DTemp9 = sensors2.getTempF(DSensor9);
     DTemp10 = sensors2.getTempF(DSensor10);
     DTemp11 = sensors2.getTempF(DSensor11);
     DTemp12 = sensors2.getTempF(DSensor12);
     DTemp13 = sensors2.getTempF(DSensor13);
  }

 void UpdateSerialMonitor(){
    Serial.print("Panel Pump Status Lag/Lead = ");
    if (digitalRead(PANEL_LAG_PUMP_RELAY))
    Serial.print( "OFF"); else Serial.print("ON");
    Serial.print(" , ");
    if (digitalRead(PANEL_LEAD_PUMP_RELAY))
    Serial.println( "OFF"); else Serial.println("ON");
    Serial.print("DHW Pump Status = ");
    if (digitalRead(DHW_PUMP_RELAY))
    Serial.println( "OFF"); else Serial.println("ON");
    Serial.print("Heat Exch Pump Status = ");
    if (digitalRead(STORAGE_HEAT_RELAY))
    Serial.println( "OFF"); else Serial.println("ON");
    Serial.print("Circ Loop Pump Status = ");
    if (digitalRead(Circ_Pump_Relay))
    Serial.println( "OFF"); else Serial.println("ON");
    Serial.print("Heat Tape Status = ");
    if (digitalRead(HEAT_TAPE_RELAY))
    Serial.println( "OFF"); else Serial.println("ON");
    Serial.print("Boiler Circulation Loop Status = ");
    if (digitalRead(BOILER_CIRC_RELAY))
    Serial.println( "OFF"); else Serial.println("ON");
    Serial.print("Recirculation Valve Status = ");
    if (digitalRead(RECIRC_VALVE_RELAY))
    Serial.println( "OFF"); else Serial.println("ON");
    Serial.println ();
    Serial.print("Alarm state = ");
    Serial.println ();
    if (ALARM != ALARM_OFF)
    {
    Serial.print(alarmMessage1);
    Serial.print(" ");
    Serial.println(alarmMessage2); 
    }   
    else Serial.println ("no Alarms");
    Serial.println();
    Serial.print("Flow Meter 1 ");
    Serial.print ((meter1volume) /3.785411784);
    Serial.print ("GPM");
    Serial.println ();
    Serial.print("Flow Meter 2 ");
    Serial.print ((meter2volume) /3.785411784);
    Serial.print ("GPM");
    Serial.println ();
    Serial.print("Flow Meter 3 ");
    Serial.print ((meter3volume) /3.785411784);
    Serial.print ("GPM");
    Serial.println ();
    Serial.print("Flow Meter 4 ");
    Serial.print ((meter4volume) /3.785411784);
    Serial.print ("GPM");
    Serial.println ();
    Serial.println();

  // report the last temperature reading
   Serial.print("PT1000 Last Temperature Reading: ");
   Serial.print(pt1000());
   Serial.println(" F");

  // report the average temperature
   Serial.print("PT1000 Average Temperature: ");
   Serial.print(pt1000AverageValue());
   Serial.println(" F");
   Serial.println();
   Serial.print("D 1 Temp = ");
   Serial.println(DTemp1);
   Serial.print("D 2 Temp = ");
   Serial.println(DTemp2);
   Serial.print("D 3 Temp = ");
   Serial.println(DTemp3);
   Serial.print("D 4 Temp = ");
   Serial.println(DTemp4);
   Serial.print("D 5 Temp = ");
   Serial.println(DTemp5);
   Serial.print("D 6 Temp = ");
   Serial.println(DTemp6);
   Serial.print("D 7 Temp = ");
   Serial.println(DTemp7);
   Serial.print("D 8 Temp = ");
   Serial.println(DTemp8);
   Serial.print("D 9 Temp = ");
   Serial.println(DTemp9);
   Serial.print("D 10 Temp = ");
   Serial.println(DTemp10);
   Serial.print("D 11 Temp = ");
   Serial.println(DTemp11);
   Serial.print("D 12 Temp = ");
   Serial.println(DTemp12);
   Serial.print("D 13 Temp = ");
   Serial.println(DTemp13);
   Serial.println();
   Serial.print("Panel Temperature     = ");
   Serial.println(panelT);
   Serial.print("sUpply Temperature    = ");
   Serial.println(supplyT);
   Serial.print("Creturn Temperature    = ");
   Serial.println(CreturnT);
   Serial.print("Outside Temperature   = ");
   Serial.println(outsideT);
   Serial.print("Storage Temperature   = ");
   Serial.println(storageT);
   Serial.print("POT DHW Heater Outlet Temperature = ");
   Serial.println(dhwT);
   Serial.print("DHW Supply Temperature = ");
   Serial.println(DhwSupplyT);
   Serial.print("DHW Return Temperature = ");
   Serial.println(DhwReturnT);
   Serial.print("Heating Supply Temperature = ");
   Serial.println(HeatingSupplyT);
   Serial.print("Heating Return Temperature = ");
   Serial.println(HeatingReturnT);
   Serial.print("Circulation Supply = ");
   Serial.println(supplyT);
   Serial.print("Circulation Return = ");
   Serial.println(CircReturnT);
   Serial.print("Potable Heat Exchanger InletT = ");
   Serial.println(PotHeatXinletT);
   Serial.print("Potable Heat Exchanger OutletT = ");
   Serial.println(PotHeatXoutletT);
   Serial.println ();
   Serial.print("Collector Temperature Rise ")& Serial.println(Collector_Temperature_Rise);
   Serial.print ("DHT Temperature Drop ")& Serial.println (DHT_Temp_Drop);
   Serial.print ("Heating Temperature Drop ") & Serial.println (Heating_Temp_Drop);
   Serial.print ("Circulation Loop Temp Drop ") & Serial.println (Circ_Loop_Temp_Drop);
   Serial.println ();
  }

 // Check for any Alarm conditions and if found, turn off panel pumps and set Alarm and Message 

  // Check for any Alarm conditions and if found, turn off panel pumps and set Alarm and Message 
   void Alarm()
   {
   if (storageT >= HIGHTEMP_LIMIT)
   {
    ALARM = ALARM_HOT;
    alarmMessage1 = "Storage at";
    alarmMessage2 = "High Limit";
    // ensure panel pumps are off (they should be anyway)
    if (state_panel_lead == PUMP_AUTO)  digitalWrite (PANEL_LEAD_PUMP_RELAY, HIGH);
    if (state_panel_lag == PUMP_AUTO)   digitalWrite (PANEL_LAG_PUMP_RELAY, HIGH);
   }
  }

 // Differential Temperature Pump Control
   void PumpControl()
   {
   if (state_panel_lead == PUMP_ON)       digitalWrite (PANEL_LEAD_PUMP_RELAY, LOW);
   else 
   {
         if (state_panel_lead == PUMP_OFF)    digitalWrite (PANEL_LEAD_PUMP_RELAY, HIGH);
         else 
      {
                      // auto panel lead pump control: // if no alarms set control pin to LOW to turn relay on, HIGH to turn relay off
        // turn on if panel vs supply differential is greater than turnOnDifferential, checking for button override
        if (ALARM == ALARM_OFF)
       {
         if (panelT >= panelTminimum && (panelT > (supplyT + PanelOnDifferential)))
             {
                digitalWrite (PANEL_LEAD_PUMP_RELAY, LOW);    
               digitalWrite (PANEL_LAG_PUMP_RELAY, LOW);
              }
           else 
          {
           if (storageT >= StoraqeHeatingLimit)
           {
             digitalWrite (PANEL_LEAD_PUMP_RELAY, HIGH);    
             digitalWrite (PANEL_LAG_PUMP_RELAY, HIGH);
            } 
          }
        }
      }
    }

   // manual panel lag pump control:
   if (state_panel_lag == PUMP_ON)       digitalWrite (PANEL_LAG_PUMP_RELAY, LOW);
   else 
     {
       if (state_panel_lag == PUMP_OFF)    digitalWrite (PANEL_LAG_PUMP_RELAY, HIGH);
       else 
       {
            // auto panel lead pump control: //turn off lag pump if differential is down far enough or back on if risen enough
            if (!digitalRead(PANEL_LEAD_PUMP_RELAY)) //if the lead pump is running
         {
           //if (panelT < (supplyT + PanelLowDifferential) ){
           if (Collector_Temperature_Rise < (PanelLowDifferential) )
           {
             digitalWrite (PANEL_LAG_PUMP_RELAY, HIGH);
           } else 
            {
             digitalWrite (PANEL_LAG_PUMP_RELAY, LOW);
            }   
         }
       }
     }

   //turn off both pumps if differential is less than turnOffDifferential
    if (panelT < (supplyT + PanelOffDifferential) )
    {
     if (state_panel_lead == PUMP_AUTO)  digitalWrite (PANEL_LEAD_PUMP_RELAY, HIGH);
     if (state_panel_lag == PUMP_AUTO)   digitalWrite (PANEL_LAG_PUMP_RELAY, HIGH);
    }

   int DHW_Heating_Call = digitalRead(DHW_HEATING_PIN);  // Read the state of pin 36 (assuming it's configured as an input or output)

   if (state_dhw == PUMP_ON)
   {
     digitalWrite (DHW_PUMP_RELAY, LOW);
   }  else 
   {
     if (state_heat == PUMP_OFF)
     {
     digitalWrite (DHW_PUMP_RELAY, HIGH);
     }  
     // auto control: turn on DHW Pump when pin 36 is pulled high from sail switch closing when domestic hot water is used. 
  
     else
     {
       if (DHW_Heating_Call == LOW) 
       {
       digitalWrite (DHW_PUMP_RELAY, LOW);
       Serial.println("Pin 36 is LOW call for DHW heating from potable water Sail Switch");
       } else 
       {
       if (DHW_Heating_Call == HIGH) 
        {
         digitalWrite (DHW_PUMP_RELAY, HIGH);
         Serial.println("Pin 36 is HIGH no call for DHW heating from Potable water Sail Switch");
        }
       }
      }
    }

   // manual heat exchanger pump control
   //01-14-24. New gas tankless installed, code no longer needs to reference storage tank temp and only needs to bring on heat circ pump now.

   //int Furnace_Heating_Call = digitalRead(48);  // Read the state of pin 48 (assuming it's configured as an input or output)   
   int Furnace_Heating_Call = digitalRead(FURNACE_HEATING_PIN);  // Read the state of pin 48 (assuming it's configured as an input or output)

   if (state_heat == PUMP_ON)
     {
      digitalWrite (STORAGE_HEAT_RELAY, LOW);
     }  else 
   {
     if (state_heat == PUMP_OFF)
     {
      digitalWrite (STORAGE_HEAT_RELAY, HIGH);
     }  
     // auto control: call for Heating circ Pump when Furnace relay pulls pin 48 low. 
   
     else
     {
     if (Furnace_Heating_Call == LOW) 
         {
           digitalWrite (STORAGE_HEAT_RELAY, LOW);
           Serial.println("Pin 48 is LOW call for heating from furance");
          } else 
       {
          if (Furnace_Heating_Call == HIGH) 
         {
           digitalWrite (STORAGE_HEAT_RELAY, HIGH);
           Serial.println("Pin 48 is HIGH no call for heating from furnace");
          }
        }
      }
    }

 
   // manual Circ pump control
   if (state_circ == PUMP_ON)
   {
     digitalWrite (Circ_Pump_Relay, LOW);
    }  else 
    { 
      if (state_circ == PUMP_OFF)
      {
        digitalWrite (Circ_Pump_Relay, HIGH);
      }  else
       {
          if (!digitalRead(DHW_PUMP_RELAY))
         {
           digitalWrite (Circ_Pump_Relay, HIGH);
          }  else
         {
             if (!digitalRead(STORAGE_HEAT_RELAY))
             {
               digitalWrite (Circ_Pump_Relay, HIGH);
              }  else
           {
              // auto control: turn on Circ Pump when CircReturnT =< supplyT + 15
              if (Circ_Pump_On <= (Circ_Loop_Temp_Drop)) 
              {       
                digitalWrite (Circ_Pump_Relay, LOW);
              } 
                if (Circ_Pump_Off >= (Circ_Loop_Temp_Drop)) 
              {
                digitalWrite (Circ_Pump_Relay, HIGH);
              }
            }
          }
       }
    }
 
   //if (!digitalRead(DHW_PUMP_RELAY)) //if the DHW relay is energized //if (!digitalRead(STORAGE_HEAT_RELAY)) //if the Storage Heat relay is energized 
   int DHW_Circ_Call = digitalRead(16); // Read the state of pin 16 which when pulled low brings on the DHW Relay
   int Furnace_Coil_Circ_Call = digitalRead(17);  // Read the state of pin 17 which when pulled low brings on Heat Relay
   // manual Boiler Circ Pump control
    if (state_boiler_circ == PUMP_ON)
   {
    digitalWrite (BOILER_CIRC_RELAY, LOW);
   }  else 
    {
     if (state_boiler_circ == PUMP_OFF)
      {
       digitalWrite (BOILER_CIRC_RELAY, HIGH);
      }  else 
      {
         //Furnace_Coil_Circ_Call
         // auto control: turn on Boiler Circulator when storageT =< Boiler_Circ_On and turn off Boiler Circulator when storageT >= Boiler_Circ_Off
         if ((Furnace_Coil_Circ_Call == LOW && Boiler_Circ_On >= storageT) || (DHW_Circ_Call == LOW && Boiler_Circ_On >= storageT))
       {       
         digitalWrite (BOILER_CIRC_RELAY, LOW);
         Serial.println ("Boiler Circ On");
       }
        //if (Furnace_Coil_Circ_Call == HIGH || Boiler_Circ_Off <= storageT || DHW_Circ_Call == HIGH)
       else 
        if ((Boiler_Circ_Off <= storageT) || (Furnace_Coil_Circ_Call == HIGH && DHW_Circ_Call == HIGH))
       {
         digitalWrite (BOILER_CIRC_RELAY, HIGH);
         Serial.println ("Boiler circ Off");
       }
      }
    }
   if (Furnace_Coil_Circ_Call == LOW)
    {
      Serial.println ("state_heat = LOW, Heat Circulator is On");
    }
   if (Furnace_Coil_Circ_Call == HIGH)
    {
      Serial.println ("state_heat = HIGH, Heat Circulator is Off");
    }
   if (DHW_Circ_Call == LOW)
    {
     Serial.println ("state_dhw = LOW, DHW Circulator is On");
    }
   if (DHW_Circ_Call == HIGH)
    {
     Serial.println ("state_dhw = HIGH, DHW Circulator is Off");
    }

   // if (!digitalRead(STORAGE_HEAT_RELAY))
  
   // manual RECIRC_VALVE_RELAY
    if (state_recirc_valve == PUMP_ON)
    {
     digitalWrite (RECIRC_VALVE_RELAY, LOW);
    }  else 
   {
    if (state_recirc_valve == PUMP_OFF)
       {
         digitalWrite (RECIRC_VALVE_RELAY, HIGH);
       }  else 
    {
  
       //RECIRC_VALVE_RELAY
       // auto control: turn on Boiler Circulator when storageT =< Boiler_Circ_On and turn off Boiler Circulator when storageT >= Boiler_Circ_Off
       if ((Furnace_Coil_Circ_Call == LOW && Boiler_Circ_On >= storageT && HeatingReturnT > storageT) || (DHW_Circ_Call == LOW && Boiler_Circ_On >= storageT && DhwReturnT > storageT))
      {       
       digitalWrite (RECIRC_VALVE_RELAY, LOW);
       Serial.println ("RECIRC VALVE CLOSED");
      }
  
       else 
       if ((Boiler_Circ_Off <= storageT) || (Furnace_Coil_Circ_Call == HIGH && DHW_Circ_Call == HIGH))
      {
       digitalWrite (RECIRC_VALVE_RELAY, HIGH);
       Serial.println ("RECIRC VALVE OPEN");
      }
    }
   }
  

   // manual Heat Tape control
   if (state_heat_tape == PUMP_ON)
   {
     digitalWrite (HEAT_TAPE_RELAY, LOW);
   }  else {
   if (state_heat_tape == PUMP_OFF)
   {
     digitalWrite (HEAT_TAPE_RELAY, HIGH);
   }  else {
  
   // auto control: turn on Heat Tape when CSupplyT =< Heat_Tape_On and turn off Heat Tape when CSupplyT >= Heat_Tape_Off
   if (Heat_Tape_On >= (CSupplyT)) {       
   digitalWrite (HEAT_TAPE_RELAY, LOW);
   Serial.println ("Heat Tape On");
   } 
   if (Heat_Tape_Off <= (CSupplyT)) {
     digitalWrite (HEAT_TAPE_RELAY, HIGH);
     Serial.println ("Heat Tape Off");
     }
    }
   }
   int pinState = digitalRead(14);  // Read the state of pin 14 (assuming it's configured as an input or output)

   if (pinState == HIGH) {
   Serial.println("Pin 14 is HIGH");
   } else if (pinState == LOW) {
   Serial.println("Pin 14 is LOW");
   } else {
   Serial.println("Invalid pin state"); // This will be printed if the state is neither HIGH nor LOW
   }
  }
 

 String readString = ""; // Used to store Form value

 void doreadStringAction()      
  {
    if (readString.indexOf("?panelleadon") >0) 
      {
      state_panel_lead = PUMP_ON;
      }
      else
       {
       if (readString.indexOf("?panelleadoff") >0)
         {
         state_panel_lead = PUMP_OFF;
         }          
         else
           {
           if (readString.indexOf("?panelleadauto") >0)
             {
             state_panel_lead = PUMP_AUTO;
             }
             else
             {
    if (readString.indexOf("?panellagon") >0) 
      {
      state_panel_lag = PUMP_ON;
      }
      else
       {
       if (readString.indexOf("?panellagoff") >0)
         {
         state_panel_lag = PUMP_OFF;
         }          
         else
           {
           if (readString.indexOf("?panellagauto") >0)
             {
             state_panel_lag = PUMP_AUTO;
             }
             else
             {
    if (readString.indexOf("?dhwon") >0) 
      {
      state_dhw = PUMP_ON;
      }
      else
       {
       if (readString.indexOf("?dhwoff") >0)
         {
         state_dhw = PUMP_OFF;
         }          
         else
           {
           if (readString.indexOf("?dhwauto") >0)
             {
             state_dhw = PUMP_AUTO;
             }
             else
             {
    if (readString.indexOf("?heaton") >0) 
      {
      state_heat = PUMP_ON;
      }
      else
       {
       if (readString.indexOf("?heatoff") >0)
         {
         state_heat = PUMP_OFF;
         }          
         else
           {
           if (readString.indexOf("?heatauto") >0)
             {
             state_heat = PUMP_AUTO;
             }
             else
               {
    if (readString.indexOf("?circon") >0) 
      {
      state_circ = PUMP_ON;
      }
      else
       {
       if (readString.indexOf("?circoff") >0)
         {
         state_circ = PUMP_OFF;
         }          
         else
           {
           if (readString.indexOf("?circauto") >0)
             {
             state_circ = PUMP_AUTO;
             }
             else
       {
    if (readString.indexOf("?heattapeon") >0) 
      {
      state_heat_tape = PUMP_ON;
      }
      else
       {
       if (readString.indexOf("?heattapeoff") >0)
         {
         state_heat_tape = PUMP_OFF;
         }          
         else
           {
           if (readString.indexOf("?heattapeauto") >0)
             {
             state_heat_tape = PUMP_AUTO;
             }
             else
               {    
    if (readString.indexOf("?recirculatingvalveon") >0) 
      {
      state_recirc_valve = PUMP_ON;
      }
      else
       {
       if (readString.indexOf("?recirculatingvalveoff") >0)
         {
         state_recirc_valve = PUMP_OFF;
         }          
         else
           {
           if (readString.indexOf("?recirculatingvalveauto") >0)
             {
             state_recirc_valve = PUMP_AUTO;
             }
             else
               {    
      if (readString.indexOf("?boilercircon") >0) 
      {
      state_boiler_circ = PUMP_ON;
      }
      else
       {
         if (readString.indexOf("?boilercircoff") >0)
         {
         state_boiler_circ = PUMP_OFF;
         }          
         else
           {
             if (readString.indexOf("?boilercircauto") >0)
             {
             state_boiler_circ = PUMP_AUTO;
             }
             else
               {
                 if (readString.indexOf("?auto") >0)
                 {
                   state_panel_lead   = PUMP_AUTO;
                   state_panel_lag    = PUMP_AUTO;
                   state_dhw          = PUMP_AUTO;
                   state_heat         = PUMP_AUTO;
                   state_circ         = PUMP_AUTO;
                   state_heat_tape    = PUMP_AUTO;
                   state_boiler_circ  = PUMP_AUTO;
                   state_recirc_valve = PUMP_AUTO;
                 }
                 else
                   {
                     if (readString.indexOf("?alloff") >0)
                     {
                       state_panel_lead   = PUMP_OFF;
                       state_panel_lag    = PUMP_OFF;
                       state_dhw          = PUMP_OFF;
                       state_heat         = PUMP_OFF; 
                       state_circ         = PUMP_OFF;
                       state_heat_tape    = PUMP_OFF;
                       state_boiler_circ  = PUMP_OFF;
                       state_recirc_valve = PUMP_OFF;                   
                     }
                    }
                }
            }
        }
   }}}}}}}}}}}}}}}}}}}}}
   Serial.print("Received request1: ");
   Serial.println(readString); 
   }
   

/*   

void doreadStringAction() {
  // Access parameters directly
  

  String panelLeadOnValue = server.arg("panelleadon");
  String panelLeadOffValue = server.arg("panelleadoff");
  String panelLeadAutoValue = server.arg("panelleadauto");
  String panelLagOnValue = server.arg("panellagon");
  String panelLagOffValue = server.arg("panellagoff");
  String panelLagAutoValue = server.arg("panellagauto");
  String dhwOnValue = server.arg("dhwon");
  String dhwOffValue = server.arg("dhwoff");
  String dhwAutoValue = server.arg("dhwauto");
  String heatOnValue = server.arg("heaton");
  String heatOffValue = server.arg("heatoff");
  String heatAutoValue = server.arg("heatauto");
  String circOnValue = server.arg("circon");
  String circOffValue = server.arg("circoff");
  String circAutoValue = server.arg("circauto");
  String heatTapeOnValue = server.arg("heattapeon");
  String heatTapeOffValue = server.arg("heattapeoff");
  String heatTapeAutoValue = server.arg("heattapeauto");
  String autoValue = server.arg("auto");
  String allOffValue = server.arg("alloff");
  
  Serial.print("Received request: ");
  Serial.println(readString); 
  Serial.println();
  Serial.println("Received request");  // Add this line
  Serial.println("heattapeon: " + heatTapeOnValue);
  Serial.println("heattapeoff: " + heatTapeOffValue);
  Serial.println("heattapeauto: " + heatTapeAutoValue);
  Serial.println();

  // Process the parameters and set component states
  if (panelLeadOnValue == "1") {
    state_panel_lead = PUMP_ON;
  } else if (panelLeadOffValue == "1") {
    state_panel_lead = PUMP_OFF;
  } else if (panelLeadAutoValue == "1") {
    state_panel_lead = PUMP_AUTO;
  }

  if (panelLagOnValue == "1") {
    state_panel_lag = PUMP_ON;
  } else if (panelLagOffValue == "1") {
    state_panel_lag = PUMP_OFF;
  } else if (panelLagAutoValue == "1") {
    state_panel_lag = PUMP_AUTO;
  }

  if (dhwOnValue == "1") {
    state_dhw = PUMP_ON;
  } else if (dhwOffValue == "1") {
    state_dhw = PUMP_OFF;
  } else if (dhwAutoValue == "1") {
    state_dhw = PUMP_AUTO;
  }

  if (heatOnValue == "1") {
    state_heat = PUMP_ON;
  } else if (heatOffValue == "1") {
    state_heat = PUMP_OFF;
  } else if (heatAutoValue == "1") {
    state_heat = PUMP_AUTO;
  }

  if (circOnValue == "1") {
    state_circ = PUMP_ON;
  } else if (circOffValue == "1") {
    state_circ = PUMP_OFF;
  } else if (circAutoValue == "1") {
    state_circ = PUMP_AUTO;
  }

//if (heatTapeOnValue == "ON") {
//    state_heat_tape = PUMP_ON;
//  } else if (heatTapeOffValue == "OFF") {
//    state_heat_tape = PUMP_OFF;
//  } else if (heatTapeAutoValue == "AUTO") {
//    state_heat_tape = PUMP_AUTO;
//  }

  if (server.hasArg("heattapeon")) {
     state_heat_tape = PUMP_ON;
     } else if (server.hasArg("heattapeoff")) {
     state_heat_tape = PUMP_OFF;
     } else if (server.hasArg("heattapeauto")) {
     state_heat_tape = PUMP_AUTO;
    }


  if (autoValue == "1") {
    state_panel_lead = PUMP_AUTO;
    state_panel_lag = PUMP_AUTO;
    state_dhw = PUMP_AUTO;
    state_heat = PUMP_AUTO;
    state_circ = PUMP_AUTO;
    state_heat_tape = PUMP_AUTO;
  }

  if (allOffValue == "1") {
    state_panel_lead = PUMP_OFF;
    state_panel_lag = PUMP_OFF;
    state_dhw = PUMP_OFF;
    state_heat = PUMP_OFF;
    state_circ = PUMP_OFF;
    state_heat_tape = PUMP_OFF;
  }
  

}
*/



void handleRoot() {
 
      
  // Include the meta tag for automatic page refresh every 5 seconds
  String html = "<html>";
html += "<head>";
html += "<style type=\"text/css\">";
html += "body{";
html += "background-color:#CAD4E0;";
html += "font-family:'Lucida Sans Unicode', 'Lucida Grande', sans-serif, Helvetica;";
html += "font-size:12px;";
html += "line-height:120%;";
html += "text-align:left;";
html += "}";
html += "h1 {";
html += "color:purple;";
html += "font-size:30px;";
html += "line-height:100%;";
html += "font-weight:bold;";
html += "}";
html += "h2 {";
html += "color:#459;";
html += "font-size:12px;";
html += "line-height:100%;";
html += "font-weight:bold;";
html += "text-align:center;";
html += "}";
html += "h3 {";
html += "color:#459;";
html += "font-size:14px;";
html += "line-height:100%;";
html += "font-weight:bold;";
html += "}";
html += "h4 {";
html += "color:purple;";
html += "font-size:11px;";
html += "line-height:100%;";
html += "font-weight:bold;";
html += "text-align:right;";
html += "}";
html += "h5 {";
html += "color:purple;";
html += "font-size:11px;";
html += "line-height:100%;";
html += "font-weight:bold;";
html += "text-align:left;";
html += "}";
html += "</style>";
html += "<title>Solar Control System</title>";
html += "<meta http-equiv=\"refresh\" content=\"2\">"; // This is used to refresh the page
html += "</head>";

html += "<body>";
html += "<table border=\"10\" cellpadding=\"4\" cellspacing=\"5\">";
html += "<tr>";

html += "<td valign=\"top\" font-size=\"11px\" align=\"left\" bgcolor=\"#0e1117\"><h5>";
html += "<div>";

// Print the current time
html += "Current time: ";
html += "<span style=\"color:blue\">";
html += timeClient.getFormattedTime();
html += "</span>";
html += (String("&emsp; &emsp; &emsp; &emsp;") + ("Date: ") + "<span style=\"color:blue\">" + timeClient.getUTCDoW() + ", " + timeClient.getUTCMonthStr() + " " + timeClient.getUTCDay() + " " + timeClient.getUTCYear());
html += "</span>";
html += "</div>";

html += "<br><br>";
//html += "<td valign=\"Bottom\" bgcolor=\"#0e1117\"><h5> ";

html += "<h5>";
html += "<div>";

html += (String("Uptime :") + "<span style=\"color:blue\">" + uptime_formatter::getUptime());

html += "</div>";
html += "</h5></td>";

html += "<td align=\"center\" bgcolor=\"#0e1117\"><h1 align=\"center\">Solar Thermal System</h1></td>";

html += "<td valign=\"Bottom\" font-size=\"11px\" align=\"right\" bgcolor=\"#0e1117\"><h4>";
html += "<div>";
html += "Version = ";
html += "<span style=\"color:blue\">";
html += VERSION_INFO;

html += "</span>";
html += "</div>";

html += "<br><br>";

html += "<h4>";

html += "<div>";
html += "Alarm state = ";
if (ALARM != ALARM_OFF)
{
  html += "<span valign=\"Bottom\" align=\"right\" style=\"color:red\">";
  html += alarmMessage1;
  html += " ";
  html += alarmMessage2;
}
else
{
  html += "<span style=\"color:blue\">";
  html += "no Alarms";
}
html += "</span></h4>";
html += "</div>";
html += "</td>";
html += "</tr>";


html += "<tr><td valign=\"top\" width=\"33%\" valign=\"top\" bgcolor=\"#419527\">";

html += "<h3 align=\"center\">System Temperatures</h3><div style=\"font-size:14px\">";

html += "<h2 align=\"center\">Outside Temperatures</h2><div style=\"font-size:14px\">";

html += "Outside Ambient (DTemp3Average):";
html += "&ensp;";
html += String(outsideT);
html += "&#176;";
html += "<br>";

html += "600 Gal Storage (DTemp2Average):";
html += "&ensp;";
html += String(storageT);
html += "&#176;";

html += "<br>";
html += "Collector Manifold (PT1000Average):";
html += "&ensp;";
html += String(panelT);
html += "&#176;";
html += "<br>";

html += "Collector Supply (DTemp1Average):";
html += "&ensp;";
html += String(CSupplyT);
html += "&#176;";
html += "<br>";

html += "Collector Return (DTemp6Average):";
html += "&ensp;";
html += String(CreturnT);
html += "&#176;";
html += "<br>";

html += "Circ Loop Supply (DTemp5Average):";
html += "&ensp;";
html += String(supplyT);
html += "&#176;";
html += "<br>";

html += "Circ Loop Return (DTemp4Average):";
html += "&ensp;";
html += String(CircReturnT);
html += "&#176;";
html += "<br>";

html += "<h2 align=\"center\">Inside Temperatures</h2><div style=\"font-size:14px\">";

html += "Domestic HW Supply (DTemp7Average):";
html += "&ensp;";
html += String(DhwSupplyT);
html += "&#176;";
html += "<br>";

html += "Domestic HW Return (DTemp8Average):";
html += "&ensp;";
html += String(DhwReturnT);
html += "&#176;";
html += "<br>";

html += "Heat Loop Supply (DTemp9Average):";
html += "&ensp;";
html += String(HeatingSupplyT);
html += "&#176;";
html += "<br>";

html += "Heat Loop Return (DTemp10Average):";
html += "&ensp;";
html += String(HeatingReturnT);
html += "&#176;";
html += "<br>";

html += "Potable Heat X in T (DTemp12Average):";
html += "&ensp;";
html += String(PotHeatXinletT);
html += "&#176;";
html += "<br>";

html += "Potable Heat X out T (DTemp13Average):";
html += "&ensp;";
html += String(PotHeatXoutletT);
html += "&#176;";
html += "<br>";

html += "Potable DHW Heater Out (DTemp11Average):";
html += "&ensp;";
html += String(dhwT);
html += "&#176;";
html += "<br>";

html += "<br></div>";

html += "</td><td width=\"34%\" valign=\"top\" bgcolor=\"#419527\">";


html += "<h3 align=\"center\">Pump Control & Status</h3><div style=\"font-size:14px\">";

html += "<form>";

html += "Lead";
html += "&ensp;";
html += "<INPUT type=button value=ON onClick=window.location='/?panelleadon'>";
html += "<INPUT type=button value=OFF onClick=window.location='/?panelleadoff'>";
html += "<INPUT type=button value=AUTO onClick=window.location='/?panelleadauto'>";
html += "&ensp;";
if (state_panel_lead == PUMP_AUTO) {
  html += "AUTO :";
} else {
  if (state_panel_lead == PUMP_OFF) {
    html += "OFF :";
  } else {
    if (state_panel_lead == PUMP_ON) {
      html += "ON :";
    } else {
      html += "????";
    }
  }
}
if (digitalRead(PANEL_LEAD_PUMP_RELAY))
  html += "<span style=\"color:black\"> STOPPED</span>";
else
  html += "<span style=\"color:blue; font-weight:bold;\"> POWERED</span>";
html += "<br>";
html += "<br>";
html += "Lag";
html += "&emsp;";
html += "<INPUT type=button value=ON onClick=window.location='/?panellagon'>";
html += "<INPUT type=button value=OFF onClick=window.location='/?panellagoff'>";
html += "<INPUT type=button value=AUTO onClick=window.location='/?panellagauto'>";
html += "&ensp;";
if (state_panel_lag == PUMP_AUTO) {
  html += "AUTO :";
} else {
  if (state_panel_lag == PUMP_OFF) {
    html += "OFF :";
  } else {
    if (state_panel_lag == PUMP_ON) {
      html += "ON :";
    } else {
      html += "????";
    }
  }
}
if (digitalRead(PANEL_LAG_PUMP_RELAY))
  html += "<span style=\"color:black\"> STOPPED</span>";
else
  html += "<span style=\"color:blue; font-weight:bold;\"> POWERED</span>";
html += "<br>";
html += "<br>";
html += "DHW";
html += "&ensp;";
html += "<INPUT type=button value=ON onClick=window.location='/?dhwon'>";
html += "<INPUT type=button value=OFF onClick=window.location='/?dhwoff'>";
html += "<INPUT type=button value=AUTO onClick=window.location='/?dhwauto'>";
html += "&ensp;";
if (state_dhw == PUMP_AUTO) {
  html += "AUTO :";
} else {
  if (state_dhw == PUMP_OFF) {
    html += "OFF :";
  } else {
    if (state_dhw == PUMP_ON) {
      html += "ON :";
    } else {
      html += "????";
    }
  }
}
if (digitalRead(DHW_PUMP_RELAY))
  html += "<span style=\"color:black\"> STOPPED</span>";
else
  html += "<span style=\"color:blue; font-weight:bold;\"> POWERED</span>";
html += "<br>";
html += "<br>";
html += "Heat";
html += "&ensp;";
html += "<INPUT type=button value=ON onClick=window.location='/?heaton'>";
html += "<INPUT type=button value=OFF onClick=window.location='/?heatoff'>";
html += "<INPUT type=button value=AUTO onClick=window.location='/?heatauto'>";
html += "&ensp;";
if (state_heat == PUMP_AUTO) {
  html += "AUTO :";
} else {
  if (state_heat == PUMP_OFF) {
    html += "OFF :";
  } else {
    if (state_heat == PUMP_ON) {
      html += "ON :";
    } else {
      html += "????";
    }
  }
}
if (digitalRead(STORAGE_HEAT_RELAY))
  html += "<span style=\"color:black\"> STOPPED</span>";
else
  html += "<span style=\"color:blue; font-weight:bold;\"> POWERED</span>";
html += "<br>";
html += "<br>";
html += "Circ";
html += "&ensp;";
html += "<INPUT type=button value=ON onClick=window.location='/?circon'>";
html += "<INPUT type=button value=OFF onClick=window.location='/?circoff'>";
html += "<INPUT type=button value=AUTO onClick=window.location='/?circauto'>";
html += "&ensp;";
if (state_circ == PUMP_AUTO) {
  html += "AUTO :";
} else {
  if (state_circ == PUMP_OFF) {
    html += "OFF :";
  } else {
    if (state_circ == PUMP_ON) {
      html += "ON :";
    } else {
      html += "????";
    }
  }
}
if (digitalRead(Circ_Pump_Relay))
  html += "<span style=\"color:black\"> STOPPED</span>";
else
  html += "<span style=\"color:blue; font-weight:bold;\"> POWERED</span>";
html += "<br>";
html += "<br>";

// Circulators
html += "Circulators <input type=button value=\"CLEAR\" onClick=window.location='/'>";
html += "&emsp;<input type=button value=\"ALL OFF\" onClick=window.location='/?alloff'>";
html += "&emsp;<input type=button value=\"ALL AUTO\" onClick=window.location='/?auto'>";

html += "</form></div>";

html += "</td><td valign=\"top\" width=\"33%\" valign=\"top\" bgcolor=\"#419527\">";

html += "<h3 align=\"center\">Heat Tape</h3><div style=\"font-size:14px\">";


html += "<form>";

html += "Heat Tape";
html += "&ensp;";
html += "<INPUT type=button value=ON onClick=window.location='/?heattapeon'>";
html += "<INPUT type=button value=OFF onClick=window.location='/?heattapeoff'>";
html += "<INPUT type=button value=AUTO onClick=window.location='/?heattapeauto'>";
html += "&ensp;";
/*
html += "<form action='?heattapeon' method='post'>";
html += "<input type='submit' name='heattapeon' value='ON'>";
html += "<input type='submit' name='heattapeoff' value='OFF'>";
html += "<input type='submit' name='heattapeauto' value='AUTO'>";
*/

if (state_heat_tape == PUMP_AUTO) {
  html += "AUTO :";
} else {
  if (state_heat_tape == PUMP_OFF) {
    html += "OFF :";
  } else {
    if (state_heat_tape == PUMP_ON) {
      html += "ON :";
    } else {
      html += "????";
    }
  }
}
if (digitalRead(HEAT_TAPE_RELAY))
  html += "<span style=\"color:black\"> STOPPED</span>";
else
  html += "<span style=\"color:blue; font-weight:bold;\"> POWERED</span>";
html += "<br>";
html += "<br>";

html += "</form></div>";

html += "</td>";


html += "<tr><td valign=\"top\" bgcolor=\"#419527\">";

html += "<h3 align=\"center\">Temp Sensors</h3><div style=\"font-size:14px\">";

html += "pt1000: ";
html += "&emsp;";
html += String(pt1000());
html += "&#176;";
html += "&emsp;";
html += "pt1000 Average:  ";
html += String(pt1000AverageValue());
html += "&#176;";
html += "<br>";
html += "DTemp1:";
html += "&emsp;";
html += String(DTemp1);
html += "&#176;";
html += "&emsp;";
html += "DTemp1Average:";
html += "&emsp;";
html += String(DTemp1Average);
html += "&#176;";
html += "<br>";
html += "DTemp2:";
html += "&emsp;";
html += String(DTemp2);
html += "&#176;";
html += "&emsp;";
html += "DTemp2Average:";
html += "&emsp;";
html += String(DTemp2Average);
html += "&#176;";
html += "<br>";
html += "DTemp3:";
html += "&emsp;";
html += String(DTemp3);
html += "&#176;";
html += "&emsp;";
html += "DTemp3Average:";
html += "&emsp;";
html += String(DTemp3Average);
html += "&#176;";
html += "<br>";
html += "DTemp4:";
html += "&emsp;";
html += String(DTemp4);
html += "&#176;";
html += "&emsp;";
html += "DTemp4Average:";
html += "&emsp;";
html += String(DTemp4Average);
html += "&#176;";
html += "<br>";
html += "DTemp5:";
html += "&emsp;";
html += String(DTemp5);
html += "&#176;";
html += "&emsp;";
html += "DTemp5Average:";
html += "&emsp;";
html += String(DTemp5Average);
html += "&#176;";
html += "<br>";
html += "DTemp6:";
html += "&emsp;";
html += String(DTemp6);
html += "&#176;";
html += "&emsp;";
html += "DTemp6Average:";
html += "&emsp;";
html += String(DTemp6Average);
html += "&#176;";
html += "<br>";
html += "DTemp7:";
html += "&emsp;";
html += String(DTemp7);
html += "&#176;";
html += "&emsp;";
html += "DTemp7Average:";
html += "&emsp;";
html += String(DTemp7Average);
html += "&#176;";
html += "<br>";
html += "DTemp8:";
html += "&emsp;";
html += String(DTemp8);
html += "&#176;";
html += "&emsp;";
html += "DTemp8Average:";
html += "&emsp;";
html += String(DTemp8Average);
html += "&#176;";
html += "<br>";
html += "DTemp9:";
html += "&emsp;";
html += String(DTemp9);
html += "&#176;";
html += "&emsp;";
html += "DTemp9Average:";
html += "&emsp;";
html += String(DTemp9Average);
html += "&#176;";
html += "<br>";
html += "DTemp10:";
html += "&ensp;";
html += String(DTemp10);
html += "&#176;";
html += "&emsp;";
html += "DTemp10Average:";
html += "&ensp;";
html += String(DTemp10Average);
html += "&#176;";
html += "<br>";
html += "DTemp11:";
html += "&ensp;";
html += String(DTemp11);
html += "&#176;";
html += "&emsp;";
html += "DTemp11Average:";
html += "&ensp;";
html += String(DTemp11Average);
html += "&#176;";
html += "<br>";
html += "DTemp12:";
html += "&ensp;";
html += String(DTemp12);
html += "&#176;";
html += "&emsp;";
html += "DTemp12Average:";
html += "&ensp;";
html += String(DTemp12Average);
html += "&#176;";
html += "<br>";
html += "DTemp13:";
html += "&ensp;";
html += String(DTemp13);
html += "&#176;";
html += "&emsp;";
html += "DTemp13Average:";
html += "&ensp;";
html += String(DTemp13Average);
html += "&#176;";

html += "<br>";
html += "<br></div>";

html += "</td><td valign=\"top\" bgcolor=\"#419527\">";


html += "<h3 align=\"center\">Information</h3><div style=\"font-size:14px\">";

html += "Min Lead Start Temp(PT1000):";
html += "&ensp;";
html += String(panelTminimum); // Manifold must achieve this temperature in addition to the supply vs panelT setting
html += "&#176;";
html += "<br>";
html += "Lead On Diff.(PT1000 vs DTemp5):";
html += "&ensp;";
html += String(PanelOnDifferential); // Panels must be this much warmer than sUpply to turn on pumps
html += "&#176;";
html += "<br>";
html += "Lag On Diff.(Dtemp6 vs DTemp11):";
html += "&ensp;";
html += String(PanelLowDifferential); // If Panels are only this much warmer, run slower
html += "&#176;";
html += "<br>";
html += "Lead Off Diff.(PT1000 vs DTemp5):";
html += "&ensp;";
html += String(PanelOffDifferential); // If Panels are only this much warmer, turn off
html += "&#176;";
html += "<br>";
html += "<br>";
html += "DHW On Diff. (DTemp2 vs DTemp1):";
html += "&ensp;";
html += String(HotWaterOnDifferential); // Storage must be this much warmer than dhw to turn on pump
html += "&#176;";
html += "<br>";
html += "DHW Off Diff. (DTemp2 vs DTemp1):";
html += "&ensp;";
html += String(HotWaterOffDifferential); // If Storage is only this much warmer than dhw, turn off pump
html += "&#176;";
html += "<br>";
html += "DHW Tank High Temp Limit:";
html += "&ensp;";
html += String(DHW_HIGHTEMP_LIMIT); // shut off when this temperature is reached
html += "&#176;";
html += "<br>";
html += "<br>";
html += "Heating Min Temp (DTemp2):";
html += "&ensp;";
html += String(StorageTooCold); // If Storage isn't hot enough, don't run heat exch pump
html += "&#176;";
html += "<br>";
html += "Storage Tank High Limit:";
html += "&ensp;";
html += String(HIGHTEMP_LIMIT); // shut off when this temperature is reached
html += "&#176;";
html += "<br>";
html += "<br>";
html += "Circ Loop On Diff.(DTemp5 vs DTemp6):";
html += "&ensp;";
html += String(Circ_Pump_On); // turn on circulation pump when deltaT hits this temperature
html += "&#176;";
html += "<br>";
html += "Circ Loop Off Diff.(DTemp5 vs DTemp6):";
html += "&ensp;";
html += String(Circ_Pump_Off); // turn off circulation pump when deltaT hits this temperature
html += "&#176;";
html += "<br>";
html += "<br>";
html += "Heat Tape on Temperature ";
html += String(Heat_Tape_On);
html += "&#176;";
html += "<br>";
html += "Heat Tape Off Temperature ";
html += String(Heat_Tape_Off);
html += "&#176;";
html += "<br>";

html += "<br></div>";

html += "</td><td valign=\"top\" width=\"33%\" valign=\"top\" bgcolor=\"#419527\">";

html += "<h3 align=\"center\">Energy</h3><div style=\"font-size:14px\">";

html += "<br>";
html += "<div>";
html += "Collectors Heat Gain:";
html += "&ensp;";

int ii = 0;
int iii = 1;

if (!digitalRead(PANEL_LEAD_PUMP_RELAY)) { //if the lead pump relay is energized
  if ((ii) <= (500 * ((meter1volume) / 3.785411784) * (CreturnT - CSupplyT))) { // if flow meter is working
    html += "<span style=\"color:blue\"></span>";
    html += String((500 * ((meter1volume) / 3.785411784) * (CreturnT - CSupplyT)) / 1000);
    html += "Kbtu ";
    html += String((500 * ((meter1volume) / 3.785411784) * (CreturnT - CSupplyT)) / (3412.141633));
    html += "KW";
  } else if ((ii) > (500 * ((meter1volume) / 3.785411784) * (CreturnT - CSupplyT))) {
    html += "<span style=\"color:red\"></span>";
    html += String((500 * ((meter1volume) / 3.785411784) * (CreturnT - CSupplyT)) / 1000);
    html += "Kbtu ";
    html += String((500 * ((meter1volume) / 3.785411784) * (CreturnT - CSupplyT)) / (3412.141633));
    html += "KW";
  }
} else {
  html += " 0 ";
  html += "Kbtu ";
  html += " 0 ";
  html += "KW";
}

html += "<br>";
html += "</div>";
html += "<div>";
html += "Collectors Temp Rise:";
html += "&ensp;";

if (!digitalRead(PANEL_LEAD_PUMP_RELAY)) { //if the lead pump relay is energized
  if ((ii) <= (500 * ((meter1volume) / 3.785411784) * (CreturnT - CSupplyT))) {
    html += "<span style=\"color:blue\"></span>";
    html += String(CreturnT - CSupplyT);
  } else if ((ii) > (500 * ((meter1volume) / 3.785411784) * (CreturnT - CSupplyT))) {
    html += "<span style=\"color:red\"></span>";
    html += String(CreturnT - CSupplyT);
  }
} else {
  html += "0";
  html += "&#176;";
}

html += "<br>";
html += "</div>";
html += "<div>";
html += "Collectors Flow Rate:";
html += "&ensp;";

if (!digitalRead(PANEL_LEAD_PUMP_RELAY)) { //if the lead pump relay is energized
  html += "<span style=\"color:blue\"></span>";
  html += String(meter1volume);
} else {
  html += " 0 ";
}

html += "L/m";
html += "&ensp;";

if (!digitalRead(PANEL_LEAD_PUMP_RELAY)) { //if the lead pump relay is energized
  html += String((meter1volume) / 3.785411784);
} else {
  html += " 0 ";
}

html += "G/m";
html += "</div>";
html += "<br>";
html += "<div>";
html += "DHW Heat Loss:";
html += "&ensp;";

if (!digitalRead(DHW_PUMP_RELAY)) { //if the DHW relay is energized
  if ((ii) <= (500 * ((meter2volume) / 3.785411784) * (DhwSupplyT - DhwReturnT))) {
    html += "<span style=\"color:blue\"></span>";
    html += String((500 * ((meter2volume) / 3.785411784) * (DhwSupplyT - DhwReturnT)) / 1000);
    html += "Kbtu ";
    html += String((500 * ((meter2volume) / 3.785411784) * (DhwSupplyT - DhwReturnT)) / (3412.141633));
    html += "KW";
  } else if ((ii) > (500 * ((meter2volume) / 3.785411784) * (DhwSupplyT - DhwReturnT))) {
    html += "<span style=\"color:red\"></span>";
    html += String((500 * ((meter2volume) / 3.785411784) * (DhwSupplyT - DhwReturnT)) / 1000);
    html += "Kbtu ";
    html += String((500 * ((meter2volume) / 3.785411784) * (DhwSupplyT - DhwReturnT)) / (3412.141633));
    html += "KW";
  }
} else {
  html += " 0 ";
  html += "Kbtu ";
  html += " 0 ";
  html += "KW";
}

html += "<br>";
html += "</div>";
html += "<div>";
html += "DHW Temp Drop:";
html += "&ensp;";

if (!digitalRead(DHW_PUMP_RELAY)) { //if the DHW relay is energized
  html += "<span style=\"color:blue\"></span>";
  html += String(DhwSupplyT - DhwReturnT);
} else {
  html += "0";
  html += "&#176;";
}

html += "<br>";
html += "</div>";
html += "<div>";
html += "DHW Loop Flow Rate: ";

if (!digitalRead(DHW_PUMP_RELAY)) { //if the DHW relay is energized
  html += "<span style=\"color:blue\"></span>";
  html += String(meter2volume);
} else {
  html += " 0 ";
}

html += "L/m";
html += "&ensp;";
if (!digitalRead(DHW_PUMP_RELAY)) { //if the DHW relay is energized
  html += String((meter2volume) / 3.785411784);
} else {
  html += " 0 ";
}

html += "G/m";
html += "</div>";
html += "<br>";
html += "<div>";
html += "Heating Heat Loss:";
html += "&ensp;";

if (!digitalRead(STORAGE_HEAT_RELAY)) { //if the Storage Heat Relay is energized
  if ((ii) <= (500 * ((meter3volume) / 3.785411784) * (HeatingSupplyT - HeatingReturnT))) {
    html += "<span style=\"color:blue\"></span>";
    html += String((500 * ((meter3volume) / 3.785411784) * (HeatingSupplyT - HeatingReturnT)) / 1000);
    html += "Kbtu ";
    html += String((500 * ((meter3volume) / 3.785411784) * (HeatingSupplyT - HeatingReturnT)) / (3412.141633));
    html += "KW";
  } else if ((ii) > (500 * ((meter3volume) / 3.785411784) * (HeatingSupplyT - HeatingReturnT))) {
    html += "<span style=\"color:red\"></span>";
    html += String((500 * ((meter3volume) / 3.785411784) * (HeatingSupplyT - HeatingReturnT)) / 1000);
    html += "Kbtu ";
    html += String((500 * ((meter3volume) / 3.785411784) * (HeatingSupplyT - HeatingReturnT)) / (3412.141633));
    html += "KW";
  }
} else {
  html += " 0 ";
  html += "Kbtu ";
  html += " 0 ";
  html += "KW";
}

html += "<br>";
html += "</div>";
html += "<div>";
html += "Heating Temp Drop:";
html += "&ensp;";

if (!digitalRead(STORAGE_HEAT_RELAY)) { 
  html += "<span style=\"color:blue\"></span>";
  html += String(HeatingSupplyT - HeatingReturnT);
} else {
  html += "0";
  html += "&#176;";
}

html += "<br>";
html += "</div>";
html += "<div>";
html += "Heating Loop Flow Rate: ";

if (!digitalRead(STORAGE_HEAT_RELAY)) { 
  html += "<span style=\"color:blue\"></span>";
  html += String(meter3volume);
} else {
  html += " 0 ";
}

html += "L/m";
html += "&ensp;";
if (!digitalRead(STORAGE_HEAT_RELAY)) { 
  html += String((meter3volume) / 3.785411784);
} else {
  html += " 0 ";
}

html += "G/m";
html += "</div>";
html += "<br>";
html += "<div>";
html += "Circ Loop Heat Loss:";
html += "&ensp;";

if (!digitalRead(Circ_Pump_Relay)) { 
  if ((ii) <= (500 * ((meter4volume) / 3.785411784) * (supplyT - CircReturnT))) {
    html += "<span style=\"color:blue\"></span>";
    html += String((500 * ((meter4volume) / 3.785411784) * (supplyT - CircReturnT)) / 1000);
    html += "Kbtu ";
    html += String((500 * ((meter4volume) / 3.785411784) * (supplyT - CircReturnT)) / (3412.141633));
    html += "KW";
  } else if ((ii) > (500 * ((meter4volume) / 3.785411784) * (supplyT - CircReturnT))) {
    html += "<span style=\"color:red\"></span>";
    html += String((500 * ((meter4volume) / 3.785411784) * (supplyT - CircReturnT)) / 1000);
    html += "Kbtu ";
    html += String((500 * ((meter4volume) / 3.785411784) * (supplyT - CircReturnT)) / (3412.141633));
    html += "KW";
  }
} else {
  html += " 0 ";
  html += "Kbtu ";
  html += " 0 ";
  html += "KW";
}

html += "<br>";
html += "</div>";
html += "<div>";
html += "Circ Loop Temp Drop:";
html += "&ensp;";

if (!digitalRead(Circ_Pump_Relay)) { 
  html += "<span style=\"color:blue\"></span>";
  html += String(supplyT - CircReturnT);
} else {
  html += "0";
  html += "&#176;";
}

html += "<br>";
html += "</div>";
html += "<div>";
html += "Circ Loop Flow Rate: ";

if (!digitalRead(Circ_Pump_Relay)) { 
  html += "<span style=\"color:blue\"></span>";
  html += String(meter4volume);
} else {
  html += " 0 ";
}

html += "L/m";
html += "&ensp;";
if (!digitalRead(Circ_Pump_Relay)) { 
  html += String((meter4volume) / 3.785411784);
} else {
  html += " 0 ";
}

html += "G/m";
html += "</div>";
html += "<br>";

html += "</td>";
html += "</tr>";
html += "</table>";
html += "</body></html>";


Serial.println(readString);
doreadStringAction();
//readString = "";

server.send(200, "text/html", html); // Send the HTML response
}




  /*
  String htmlContent = "<!DOCTYPE HTML>\r\n"
                       "<html>\r\n"
                       "<head>\r\n"
                       "<meta http-equiv=\"refresh\" content=\"5\">\r\n" // Automatic refresh every 5 seconds
                       "</head>\r\n"
                       "<body>\r\n"
                       "<h2>Hello World from ESP32-S3-DevKit!</h2>\r\n"
                       //"<p>Temperature: " + temperatureString + " C</p>\r\n"
                       "<p>Temperature: " + temperatureString + " C</p>"
                       "<p>Temperature: " + fahrenheitTemperatureString + " F</p>"
                       "<p>Version: " + String(VERSION_INFO) + "</p>\r\n" // Second line showing the version
                                  
                       "</body>\r\n"
                       "</html>\r\n";

  server.send(200, "text/html", htmlContent);
}
*/


// Task to run WebReporting() on core 0 with lower priority than system functions
void WebReportingTask(void* pvParameters) {
  const TickType_t xDelay = pdMS_TO_TICKS(1000); // 1 second delay
  for (;;) {
    handleRoot(); // Call the WebReporting function
    vTaskDelay(xDelay); // Delay for xDelay milliseconds
  }
}


// Function to read the temperature and RTD resistance from the PT1000 not taken from ESP32_S3_Solar_Controller20_4
void readTemperatureAndResistance(float& temperature, float& rtdResistance) {
  thermo.begin(MAX31865_4WIRE); // Use 4-wire RTD configuration
  float RTDnominal = 1000.0; // PT1000 nominal resistance
  float refResistor = 4300.0; // Reference resistor value
  temperature = thermo.temperature(RTDnominal, refResistor);
  //rtdResistance = thermo.resistance(RTDnominal, refResistor); // Use the resistance() function to get the RTD resistance
rtdResistance = thermo.readRTD();
}

float temperature;
float rtdResistance;
float PT1000rtdResistance = thermo.readRTD();


// Counter for the number of requests received
int reqCount = 0;



void setup() {
  Serial.begin(115200);
  
  while (!Serial && (millis() < 5000));

  Serial.print(F("\nStart WebServer on "));
  Serial.print(ARDUINO_BOARD);
  Serial.print(F(" with "));
  Serial.println(SHIELD_TYPE);
  Serial.println(WEBSERVER_ESP32_SC_W5500_VERSION);
  
  /*
  ET_LOGWARN(F("Default SPI pinout:"));
  ET_LOGWARN1(F("SPI_HOST:"), ETH_SPI_HOST);
  ET_LOGWARN1(F("MOSI:"), MOSI_GPIO);
  ET_LOGWARN1(F("MISO:"), MISO_GPIO);
  ET_LOGWARN1(F("SCK:"),  SCK_GPIO);
  ET_LOGWARN1(F("CS:"),   CS_GPIO);
  ET_LOGWARN1(F("INT:"),  INT_GPIO);
  ET_LOGWARN1(F("SPI Clock (MHz):"), SPI_CLOCK_MHZ);
  ET_LOGWARN(F("========================="));
*/
SPI.begin(LSBFIRST, SPI_MODE0, SPI_CLOCK_DIV2);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // MAC address of your device
IPAddress ip(10, 20, 90, 33);            // IP address of your device
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(10, 20, 90, 1);        // Gateway IP address

  // Begin the Ethernet connection
 // ETH.begin(W5500_MISO, W5500_MOSI, W5500_SCLK, W5500_CS, W5500_INT, 25, SPI2_HOST, mac);
  ETH.begin(W5500_MISO, W5500_MOSI, W5500_SCLK, W5500_CS, W5500_INT, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac);

//ESP32_W5500_waitForConnect();

  // Start the web server on port 80
  server.begin();

 // Print IP address
  Serial.print("IP Address: ");
  Serial.println(ETH.localIP());

  // Specify the HTTP handlers for the web server
  server.on("/", handleRoot);
  
   // Start the web server on port 80
    server.begin();
      sensors.setResolution(DSensor1, 12);
      sensors.setResolution(DSensor2, 12);
      sensors.setResolution(DSensor3, 12);
      sensors.setResolution(DSensor4, 12);
      sensors.setResolution(DSensor5, 12);
      sensors.setResolution(DSensor6, 12);
      sensors.setResolution(DSensor7, 12);
      sensors.setResolution(DSensor8, 12);
      sensors.setResolution(DSensor9, 12);
      sensors.setResolution(DSensor10, 12);
      sensors.setResolution(DSensor11, 12);
      sensors.setResolution(DSensor12, 12);
      sensors.setResolution(DSensor13, 12);
    
    // Initialize the PT1000 sensor
      thermo.begin(MAX31865_4WIRE);

    // prep the relay control pins
    // setup pins as outputs
      pinMode (PANEL_LEAD_PUMP_RELAY, OUTPUT);
      pinMode (PANEL_LAG_PUMP_RELAY, OUTPUT);
      pinMode (DHW_PUMP_RELAY, OUTPUT);
      pinMode (STORAGE_HEAT_RELAY, OUTPUT);
      pinMode (Circ_Pump_Relay, OUTPUT);
      pinMode (HEAT_TAPE_RELAY, OUTPUT);
      pinMode (BOILER_CIRC_RELAY, OUTPUT);
      pinMode (RECIRC_VALVE_RELAY, OUTPUT);

    // set all pumps to off to begin -
     digitalWrite (PANEL_LEAD_PUMP_RELAY, HIGH);
     digitalWrite (PANEL_LAG_PUMP_RELAY, HIGH);
     digitalWrite (DHW_PUMP_RELAY, HIGH);
     digitalWrite (STORAGE_HEAT_RELAY, HIGH);
     digitalWrite (Circ_Pump_Relay, HIGH);
     digitalWrite (BOILER_CIRC_RELAY, HIGH);
  
    // set valve closed to begin
     digitalWrite (RECIRC_VALVE_RELAY, HIGH);

    // set heat tape on to begin
     digitalWrite (HEAT_TAPE_RELAY, HIGH);

    // Call for Heat from Furnace
     pinMode (48, INPUT); //pulled low by external relay on call for heat
     pinMode (36, INPUT); //pulled low by external relay on call for heat
     pinMode (FURNACE_HEATING_PIN, INPUT_PULLUP);
     pinMode (DHW_HEATING_PIN, INPUT_PULLUP);  
  
    //Flow meter pins
     pinMode(8, INPUT); //It is necessary to declare the input pin 8
     pinMode(3, INPUT); //It is necessary to declare the input pin 3
     pinMode(46, INPUT); //It is necessary to declare the input pin 46
     pinMode(9, INPUT); //It is necessary to declare the input pin 9
  

    // get a new FlowMeter instance for an uncalibrated flow sensor and let them attach their 'interrupt service handler' (ISR) on every rising edge
     Meter1 = new FlowMeter(digitalPinToInterrupt(8), MySensor1, Meter1ISR, RISING);
     #define meter1volume  (Meter1->getCurrentFlowrate())
    
    // do this setup step for every  FlowMeter and ISR you have defined, depending on how many you need
     Meter2 = new FlowMeter(digitalPinToInterrupt(3), MySensor2, Meter2ISR, RISING);
     #define meter2volume  (Meter2->getCurrentFlowrate())
    
    // do this setup step for every  FlowMeter and ISR you have defined, depending on how many you need
     Meter3 = new FlowMeter(digitalPinToInterrupt(46), MySensor3, Meter3ISR, RISING);
     #define meter3volume  (Meter3->getCurrentFlowrate())
      
    // do this setup step for every  FlowMeter and ISR you have defined, depending on how many you need
     Meter4 = new FlowMeter(digitalPinToInterrupt(9), MySensor4, Meter4ISR, RISING);
     #define meter4volume  (Meter4->getCurrentFlowrate())
 
     #define CollectorTemperatureRise ((CreturnT) - (supplyT))
     #define DhwTemperatureDrop ((DhwSupplyT) - (DhwReturnT))
     #define HeatingTemperatureDrop ((HeatingSupplyT) - (HeatingReturnT))
     #define CirculationTemperatureDrop ((supplyT) - (CircReturnT)) 
   
    // Sync Time on Boot   
     timeClient.begin();
     timeClient.setPoolServerName("pool.ntp.org");  // Set the NTP server
     timeClient.forceUpdate();  // Sync the time immediately
     timeClient.setTimeOffset(3600 * TIME_ZONE_OFFSET_HRS);  // Set the time offset for MST
     Serial.println (String ("Date: ") + timeClient.getUTCMonthStr() + " " + timeClient.getUTCDay() + " " + timeClient.getUTCYear());
 
    

   xTaskCreatePinnedToCore(
    WebReportingTask,    /* Task function */
    "WebReportingTask",  /* Name of the task */
    10000,               /* Stack size in words */
    NULL,                /* Task input parameter */
    0,                   /* Priority of the task (lower value means lower priority) */
    NULL,                /* Task handle */
    0                    /* Core where the task should run (0 = core 0) */
  );
  
}

void loop() 
 {
   unsigned long currentMillis = millis();

   // Check if it's time for synchronization
   if (currentMillis - lastSyncTime >= syncInterval) 
   {
    timeClient.forceUpdate();  // Sync the time immediately
    lastSyncTime = currentMillis;  // Update the last synchronization time
   }

   // Print the current time
     Serial.print("Current time: ");
     Serial.println(timeClient.getFormattedTime());
     Serial.println (String ("Date: ") + timeClient.getUTCDoW() + " " + timeClient.getUTCMonthStr() + " " + timeClient.getUTCDay() + " " + timeClient.getUTCYear());

  /*
   //Temperature Probe Assignments
     panelT   = pt1000Average;
     CSupplyT = DTemp1Average;
     storageT= DTemp2Average;
     outsideT= DTemp3Average; 
     CreturnT = DTemp6Average;
     supplyT = DTemp5Average;  
     CircReturnT = DTemp4Average;
     DhwSupplyT = DTemp7Average;
     DhwReturnT = DTemp8Average;
     HeatingSupplyT = DTemp9Average;
     HeatingReturnT = DTemp10Average;
     dhwT    = DTemp11Average;
     PotHeatXinletT = DTemp12Average;
     PotHeatXoutletT = DTemp13Average;
     */
     panelT   = 157;
     CSupplyT = 110;
     storageT= 110;
     outsideT= -30; 
     CreturnT = 156;
     supplyT = 126;  
     CircReturnT = 80;
     DhwSupplyT = 110;
     DhwReturnT = 75;
     HeatingSupplyT = 110;
     HeatingReturnT = 75;
     dhwT    = 100;
     PotHeatXinletT = 80;
     PotHeatXoutletT = 70;


 // Read pt1000 temperature from MAX31865
  float pt1000Val = pt1000();

  // Check if the temperature reading is valid
  if (pt1000Val == -196.60) {
    // Replace with the previous valid temperature value in the array
    for (int i = pt1000NumReadings - 1; i >= 0; i--) {
      if (pt1000Values[i] != -196.60) {
        pt1000Val = pt1000Values[i];
        break;
      }
    }
  }


   
  // Store the reading in the array
  pt1000Values[pt1000Index] = pt1000Val;

  // Increment the index and wrap around if necessary
  pt1000Index = (pt1000Index + 1) % pt1000NumReadings;

  // Wait for a short time to allow other tasks to run
  //delay(10);
/*
  // Report the last temperature reading
  Serial.print("pt1000 Last Temperature Reading: ");
  Serial.print(pt1000Val);
  Serial.println(" F");
*/
// Report the average temperature for pt1000
  pt1000Average = pt1000AverageValue();
  /*
  Serial.print("pt1000 Average Temperature: ");
  Serial.print(pt1000Average);
  Serial.println(" F");
*/


 // Read the temperature from DTemp1
  float DTemp1Val = DTemp1;

  // Check if the temperature reading is valid
  if (DTemp1Val == -196.60) {
    // Replace with the previous valid temperature value in the array
    for (int i = DTemp1NumReadings - 1; i >= 0; i--) {
      if (DTemp1Values[i] != -196.60) {
        DTemp1Val = DTemp1Values[i];
        break;
      }
    }
  }

   
  // Store the reading in the array
  DTemp1Values[DTemp1Index] = DTemp1Val;

  // Increment the index and wrap around if necessary
  DTemp1Index = (DTemp1Index + 1) % DTemp1NumReadings;

  // Wait for a short time to allow other tasks to run
  //delay(10);
/*
  // Report the last temperature reading
  Serial.print("DTemp1 Last Temperature Reading: ");
  Serial.print(DTemp1Val);
  Serial.println(" F");
*/
// Report the average temperature for DTemp1
  DTemp1Average = DTemp1AverageValue();
  /*
  Serial.print("DTemp1 Average Temperature: ");
  Serial.print(DTemp1Average);
  Serial.println(" F");
*/

//debuging use 
//Serial.print("DTemp1Val: ");
//Serial.println(DTemp1Val);
//Serial.print("DTemp1NumReadings: ");
//Serial.println(DTemp1NumReadings);
//Serial.print("DTemp1Index: ");
//Serial.println(DTemp1Index);

/*
//Debugging use, prints values of  the DTemp1Values array
for (int i = 0; i < DTemp1NumReadings; i++) {
  Serial.print("DTemp1Values[");
  Serial.print(i);
  Serial.print("]: ");
  Serial.println(DTemp1Values[i]);
}
*/


// Read the temperature from DTemp2
  float DTemp2Val = DTemp2;

  // Check if the temperature reading is valid
  if (DTemp2Val == -196.60) {
    // Replace with the previous valid temperature value in the array
    for (int i = DTemp2NumReadings - 1; i >= 0; i--) {
      if (DTemp2Values[i] != -196.60) {
        DTemp2Val = DTemp2Values[i];
        break;
      }
    }
  }

  // Store the reading in the array
  DTemp2Values[DTemp2Index] = DTemp2Val;

  // Increment the index and wrap around if necessary
  DTemp2Index = (DTemp2Index + 1) % DTemp2NumReadings;

  // Wait for a short time to allow other tasks to run
  //delay(10);
/*
  // Report the last temperature reading
  Serial.print("DTemp2 Last Temperature Reading: ");
  Serial.print(DTemp2Val);
  Serial.println(" F");
*/
  // Report the average temperature
  DTemp2Average = DTemp2AverageValue();
  /*
  Serial.print("DTemp2 Average Temperature: ");
  Serial.print(DTemp2Average);
  Serial.println(" F");
*/
  // Read the temperature from DTemp3
  float DTemp3Val = DTemp3;

  // Check if the temperature reading is valid
  if (DTemp3Val == -196.60) {
    // Replace with the previous valid temperature value in the array
    for (int i = DTemp3NumReadings - 1; i >= 0; i--) {
      if (DTemp3Values[i] != -196.60) {
        DTemp3Val = DTemp3Values[i];
        break;
      }
    }
  }

  // Store the reading in the array
  DTemp3Values[DTemp3Index] = DTemp3Val;

  // Increment the index and wrap around if necessary
  DTemp3Index = (DTemp3Index + 1) % DTemp3NumReadings;

  // Wait for a short time to allow other tasks to run
  //delay(10);
/*
  // Report the last temperature reading
  Serial.print("DTemp3 Last Temperature Reading: ");
  Serial.print(DTemp3Val);
  Serial.println(" F");
*/
// Report the average temperature for DTemp3
  DTemp3Average = DTemp3AverageValue();
  /*
  Serial.print("DTemp3 Average Temperature: ");
  Serial.print(DTemp3Average);
  Serial.println(" F");
 */ 
// Read the temperature from DTemp4
  float DTemp4Val = DTemp4;

  // Check if the temperature reading is valid
  if (DTemp4Val == -196.60) {
    // Replace with the previous valid temperature value in the array
    for (int i = DTemp4NumReadings - 1; i >= 0; i--) {
      if (DTemp4Values[i] != -196.60) {
        DTemp4Val = DTemp4Values[i];
        break;
      }
    }
  }

  // Store the reading in the array
  DTemp4Values[DTemp4Index] = DTemp4Val;

  // Increment the index and wrap around if necessary
  DTemp4Index = (DTemp4Index + 1) % DTemp4NumReadings;

  // Wait for a short time to allow other tasks to run
  //delay(10);
/*
  // Report the last temperature reading
  Serial.print("DTemp4 Last Temperature Reading: ");
  Serial.print(DTemp4Val);
  Serial.println(" F");
*/
// Report the average temperature for DTemp4
  DTemp4Average = DTemp4AverageValue();
  /*
  Serial.print("DTemp4 Average Temperature: ");
  Serial.print(DTemp4Average);
  Serial.println(" F");
*/
// Read the temperature from DTemp5
  float DTemp5Val = DTemp5;

  // Check if the temperature reading is valid
  if (DTemp5Val == -196.60) {
    // Replace with the previous valid temperature value in the array
    for (int i = DTemp5NumReadings - 1; i >= 0; i--) {
      if (DTemp5Values[i] != -196.60) {
        DTemp5Val = DTemp5Values[i];
        break;
      }
    }
  }

  // Store the reading in the array
  DTemp5Values[DTemp5Index] = DTemp5Val;

  // Increment the index and wrap around if necessary
  DTemp5Index = (DTemp5Index + 1) % DTemp5NumReadings;

  // Wait for a short time to allow other tasks to run
  //delay(10);
/*
  // Report the last temperature reading
  Serial.print("DTemp5 Last Temperature Reading: ");
  Serial.print(DTemp5Val);
  Serial.println(" F");
*/
// Report the average temperature for DTemp5
  DTemp5Average = DTemp5AverageValue();
  /*
  Serial.print("DTemp5 Average Temperature: ");
  Serial.print(DTemp5Average);
  Serial.println(" F");
*/
// Read the temperature from DTemp6
  float DTemp6Val = DTemp6;

  // Check if the temperature reading is valid
  if (DTemp6Val == -196.60) {
    // Replace with the previous valid temperature value in the array
    for (int i = DTemp6NumReadings - 1; i >= 0; i--) {
      if (DTemp6Values[i] != -196.60) {
        DTemp6Val = DTemp6Values[i];
        break;
      }
    }
  }

  // Store the reading in the array
  DTemp6Values[DTemp6Index] = DTemp6Val;

  // Increment the index and wrap around if necessary
  DTemp6Index = (DTemp6Index + 1) % DTemp6NumReadings;

  // Wait for a short time to allow other tasks to run
  //delay(10);
/*
  // Report the last temperature reading
  Serial.print("DTemp6 Last Temperature Reading: ");
  Serial.print(DTemp6Val);
  Serial.println(" F");
*/
// Report the average temperature for DTemp6
  DTemp6Average = DTemp6AverageValue();
  /*
  Serial.print("DTemp6 Average Temperature: ");
  Serial.print(DTemp6Average);
  Serial.println(" F");
*/
// Read the temperature from DTemp7
  float DTemp7Val = DTemp7;

  // Check if the temperature reading is valid
  if (DTemp7Val == -196.60) {
    // Replace with the previous valid temperature value in the array
    for (int i = DTemp7NumReadings - 1; i >= 0; i--) {
      if (DTemp7Values[i] != -196.60) {
        DTemp7Val = DTemp7Values[i];
        break;
      }
    }
  }

  // Store the reading in the array
  DTemp7Values[DTemp7Index] = DTemp7Val;

  // Increment the index and wrap around if necessary
  DTemp7Index = (DTemp7Index + 7) % DTemp7NumReadings;

  // Wait for a short time to allow other tasks to run
  //delay(10);
/*
  // Report the last temperature reading
  Serial.print("DTemp7 Last Temperature Reading: ");
  Serial.print(DTemp7Val);
  Serial.println(" F");
*/
// Report the average temperature for DTemp7
  DTemp7Average = DTemp7AverageValue();
  /*
  Serial.print("DTemp7 Average Temperature: ");
  Serial.print(DTemp7Average);
  Serial.println(" F");
*/
// Read the temperature from DTemp8
  float DTemp8Val = DTemp8;

  // Check if the temperature reading is valid
  if (DTemp8Val == -196.60) {
    // Replace with the previous valid temperature value in the array
    for (int i = DTemp8NumReadings - 1; i >= 0; i--) {
      if (DTemp8Values[i] != -196.60) {
        DTemp8Val = DTemp8Values[i];
        break;
      }
    }
  }

  // Store the reading in the array
  DTemp8Values[DTemp8Index] = DTemp8Val;

  // Increment the index and wrap around if necessary
  DTemp8Index = (DTemp8Index + 1) % DTemp8NumReadings;

  // Wait for a short time to allow other tasks to run
  //delay(10);
/*
  // Report the last temperature reading
  Serial.print("DTemp8 Last Temperature Reading: ");
  Serial.print(DTemp8Val);
  Serial.println(" F");
*/
// Report the average temperature for DTemp8
  DTemp8Average = DTemp8AverageValue();
  /*
  Serial.print("DTemp8 Average Temperature: ");
  Serial.print(DTemp8Average);
  Serial.println(" F");
*/
// Read the temperature from DTemp9
  float DTemp9Val = DTemp9;

  // Check if the temperature reading is valid
  if (DTemp9Val == -196.60) {
    // Replace with the previous valid temperature value in the array
    for (int i = DTemp9NumReadings - 1; i >= 0; i--) {
      if (DTemp9Values[i] != -196.60) {
        DTemp9Val = DTemp9Values[i];
        break;
      }
    }
  }

  // Store the reading in the array
  DTemp9Values[DTemp9Index] = DTemp9Val;

  // Increment the index and wrap around if necessary
  DTemp9Index = (DTemp9Index + 1) % DTemp9NumReadings;

  // Wait for a short time to allow other tasks to run
  //delay(10);
/*
  // Report the last temperature reading
  Serial.print("DTemp9 Last Temperature Reading: ");
  Serial.print(DTemp9Val);
  Serial.println(" F");
*/
// Report the average temperature for DTemp9
  DTemp9Average = DTemp9AverageValue();
  /*
  Serial.print("DTemp9 Average Temperature: ");
  Serial.print(DTemp9Average);
  Serial.println(" F");
*/
// Read the temperature from DTemp10
  float DTemp10Val = DTemp10;

  // Check if the temperature reading is valid
  if (DTemp10Val == -196.60) {
    // Replace with the previous valid temperature value in the array
    for (int i = DTemp10NumReadings - 1; i >= 0; i--) {
      if (DTemp10Values[i] != -196.60) {
        DTemp10Val = DTemp10Values[i];
        break;
      }
    }
  }

  // Store the reading in the array
  DTemp10Values[DTemp10Index] = DTemp10Val;

  // Increment the index and wrap around if necessary
  DTemp10Index = (DTemp10Index + 1) % DTemp10NumReadings;

  // Wait for a short time to allow other tasks to run
  //delay(10);
/*
  // Report the last temperature reading
  Serial.print("DTemp10 Last Temperature Reading: ");
  Serial.print(DTemp10Val);
  Serial.println(" F");
*/
// Report the average temperature for DTemp10
  DTemp10Average = DTemp10AverageValue();
  /*
  Serial.print("DTemp10 Average Temperature: ");
  Serial.print(DTemp10Average);
  Serial.println(" F");
*/
// Read the temperature from DTemp11
  float DTemp11Val = DTemp11;

  // Check if the temperature reading is valid
  if (DTemp11Val == -196.60) {
    // Replace with the previous valid temperature value in the array
    for (int i = DTemp11NumReadings - 1; i >= 0; i--) {
      if (DTemp11Values[i] != -196.60) {
        DTemp11Val = DTemp11Values[i];
        break;
      }
    }
  }

  // Store the reading in the array
  DTemp11Values[DTemp11Index] = DTemp11Val;

  // Increment the index and wrap around if necessary
  DTemp11Index = (DTemp11Index + 1) % DTemp11NumReadings;

  // Wait for a short time to allow other tasks to run
  //delay(10);
/*  
  // Report the average temperature for DTemp11
  Serial.print("DTemp11 Average Temperature: ");
  Serial.print(DTemp11Average);
  Serial.println(" F");
*/
  // Report the average temperature for DTemp11
  DTemp11Average = DTemp11AverageValue();
/*  
  Serial.print("DTemp11 Average Temperature: ");
  Serial.print(DTemp11Average);
  Serial.println(" F");
*/
  
// Read the temperature from DTemp12
  float DTemp12Val = DTemp12;

  // Check if the temperature reading is valid
  if (DTemp12Val == -196.60) {
    // Replace with the previous valid temperature value in the array
    for (int i = DTemp12NumReadings - 1; i >= 0; i--) {
      if (DTemp12Values[i] != -196.60) {
        DTemp12Val = DTemp12Values[i];
        break;
      }
    }
  }

  // Store the reading in the array
  DTemp12Values[DTemp12Index] = DTemp12Val;

  // Increment the index and wrap around if necessary
  DTemp12Index = (DTemp12Index + 1) % DTemp12NumReadings;
  
  // Wait for a short time to allow other tasks to run
  //delay(10);
/*
  // Report the last temperature reading
  Serial.print("DTemp12 Last Temperature Reading: ");
  Serial.print(DTemp12Val);
  Serial.println(" F");
*/
   // Report the average temperature for DTemp12
  DTemp12Average = DTemp12AverageValue();
/*  
  Serial.print("DTemp12 Average Temperature: ");
  Serial.print(DTemp12Average);
  Serial.println(" F");
*/


  // Read the temperature from DTemp13
  float DTemp13Val = DTemp13;

  // Check if the temperature reading is valid
  if (DTemp13Val == -196.60) {
    // Replace with the previous valid temperature value in the array
    for (int i = DTemp13NumReadings - 1; i >= 0; i--) {
      if (DTemp13Values[i] != -196.60) {
        DTemp13Val = DTemp13Values[i];
        break;
      }
    }
  }

  // Store the reading in the array
  DTemp13Values[DTemp13Index] = DTemp13Val;

  // Increment the index and wrap around if necessary
  DTemp13Index = (DTemp13Index + 1) % DTemp13NumReadings;

  // Wait for a short time to allow other tasks to run
  //delay(10);
/*
  // Report the last temperature reading
  Serial.print("DTemp13 Last Temperature Reading: ");
  Serial.print(DTemp13Val);
  Serial.println(" F");
*/
   // Report the average temperature for DTemp13
     DTemp13Average = DTemp13AverageValue();
     /*
     Serial.print("DTemp13 Average Temperature: ");
     Serial.print(DTemp13Average);
     Serial.println(" F");
     */ 


  
   // process the (possibly) counted ticks
   // Meter1->tick(period);
   // Meter2->tick(period);
   // Meter3->tick(period);
   // Meter4->tick(period);

 
   // Read Digital Temps
     ReadDigitalTemperatures();

   
   // Alarm if needed
     Alarm();

   // PUMP CONTROL: Control Panel Circ, Heat Exchanger Circ and DHW Circ Pumps
     PumpControl();
 
   // Update Serial Monitor  
    UpdateSerialMonitor();
  
   // Handle incoming client requests
     server.handleClient();
     Serial.println(); // Add this line to print on a new line
     Serial.print("sketch = ");
     Serial.print(VERSION_INFO);
     Serial.println(); // Add this line to print on a new line
     Serial.print("IP Address: ");
     Serial.println(ETH.localIP());
 




// Check for reboot command from the serial monitor
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    if (command == "reboot") {
      Serial.println("Rebooting ESP32...");
      delay(1000);
      ESP.restart();
    }
  }


// Create a timer to delay for 5 seconds.
  static uint32_t timer = millis();
  while (millis() - timer < 2000) {
    // Do some stuff here.
  }
  // The timer has expired. Do something.
  // Reset the timer.
  timer = millis();

}



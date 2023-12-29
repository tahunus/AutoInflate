
An API to interact with AUTOINFLATE via a Wi-Fi connection using UDP packets.

# Content
[What it is](#what-it-is)  
[How to add it to AUTOINFLATE](#how-to-add-it-to-autoinflate)  
[How it works](#how-it-works)  

-----------------------------------------------------------------------------------------------------------------------

## What it is
An API using UDP messages (see [What is UDP](https://www.cloudflare.com/learning/ddos/glossary/user-datagram-protocol-udp/)) formatted in JSON (using the [ArduinoJson library](https://arduinojson.org/)) and managed by [Espressif's Async UDP ](https://github.com/espressif/arduino-esp32/tree/master/libraries/AsyncUDP)to interact with AUTOINFLATE via an outside microprocessor, single board computer, smartphone or PC/Mac. 

The API includes methods to retrieve the operating variables as currently defined & saved in AUTOINFLATE, to set & save new values for these variables and, to run defined inflation cycles.

To avoid possible operating conflicts, UDP requests received in the unit during inflation cycles or while the unit's menu is in CONFIG mode will return an error response.  The whole API is encapsulated in a Class to avoid present and future conflicts with variable or function names with the rest of AUTOINFLATE's code. 

-----------------------------------------------------------------------------------------------------------------------

## How to add it to AUTOINFLATE
**Step 1.** In the main .ino program, add 3 lines of code:
- `#include "WiFi_API.h"`  as the last include statement
- `wifiAPI.begin();` as the last statement in setup()
- `wifiAPI.chkProcess(); ` as the last statement in loop()

**Step 2.** add these 2 files to the folder where the main .ino file resides
- WiFi_API.h
- WiFi_API_config.h

**Step 3.** enter the required credentials in WiFI_API_config.h
```c++
const char* ssid = "yourNetworkID";
const char* password = "yourPassword";
const int UDPport = 12345;  // define a 4 or 5 digit port 

#define DEBUG_PRINT  //if commented out, all debug serial output statments become empty when compiled

#define STATIC_IP  //if commented out, IP & DNS will come from router's DHCP
#ifdef STATIC_IP
  IPAddress staticIP(192, 168, 1, 69); //if this IP is reserved in your router, connection can be faster
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(8, 8, 8, 8);
  IPAddress secondaryDNS(192, 168, 1, 1);
#endif
```

**Step 4.** Compile & load. 

-----------------------------------------------------------------------------------------------------------------------

## How it works
### Definition of API Messages 
note:  Methods, Groups and Parameter names are case sensitive and in JSON format

### Methods
- **run** : execute an defined inflation cycle
- **getConfig** : retrieve the parameters stored in the device for a particular group
- **setConfig** : send and save parameters for a particular group

### Groups
- **profileRUN** : inflation cycle using the profile option for multiple cycles based on total time
- **hugRUN** : one time inflation cycle
- **airSYS** : inflation operating limits
- **config** : pump operation limits
- **globals** : selected global variables

### Parameters

 group | parameter | min | max | units
--- | --- | --- | --- | ---
**profileRUN** | highPressure | 0 | MaxPressure | PSI x 10
| | onTime | 0 | 599 | seconds
| | offTime | 0 | 599 | seconds
| | cycletime | 0 | 5999 | seconds 
**hugRUN** | hugTime | 0 | 599 | seconds
| | hugPressure | 0 | MaxPressure | seconds
**airSYS** | maxPumpPWM | 0 | 51 | ?
| | maxPressure | 0 | MaxPressure | PSI x 10
| | thresholdPressure | 0 | 5 | ?
**config** | minPumpPWM | 200 | 1023 | ?
| | freq | 2 | 199 | ?
| | clearALL | 0 | 1 | boolean
**globals** | MaxPressure | 0 | 50 | PSI x 10
| | battery | | | Volts. Read-only

### Sample requests & responses

**Request**  
{  
&nbsp;  "method" :  "getConfig",  
&nbsp;  "group" :  "hugRUN"  
}  

 **Response**  
{  
&nbsp;  "method" :  "getConfig",  
&nbsp;  "group" :  "hugRUN",  
&nbsp;  "hugTime" :  30,  
&nbsp;  "hugPressure" :  30  
}  

  
  
**Request**  
{  
&nbsp;  "method" :  "setConfig",  
&nbsp;  "group" :  "hugRUN",  
&nbsp;  "hugTime" :  30,  
&nbsp;  "hugPressure" :  30  
} 

 **Response**  
{  
&nbsp;  "method" :  "getConfig",  
&nbsp;  "group" :  "hugRUN",  
&nbsp;  "response" :  "Success"  
} 

To test sending and receiving UDP messages prior to developing some program, you can use a simple packet sender. For windows I'm using [Network Tools Collection Desktop Edition](https://www.microsoft.com/store/productId/9NW6L8PLQMM4?ocid=pdpshare)

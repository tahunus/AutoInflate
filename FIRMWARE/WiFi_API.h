#ifndef INCLUDE_WIFI_API_H
#define INCLUDE_WIFI_API_H

#include <ArduinoJson.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include "WiFi_API_config.h" //credentials & config

#ifdef DEBUG_PRINT //if not defined, output statments become empty when compiled
  #define debug_print(x)            Serial.print(x)
  #define debug_println(...)        Serial.println(__VA_ARGS__)
  #define debug_printf(...)         Serial.printf(__VA_ARGS__)
  #define debug_write(data, length) Serial.write(data, length)
#else
  #define debug_print(x)
  #define debug_println(...)
  #define debug_printf(...)
  #define debug_write(data, length)
#endif

class WiFi_API {
  private:
    AsyncUDP udp;

    const char* api_methods[3] = {"run", "setConfig", "getConfig"};
    const char* api_groupNames[5] = {"profileRUN", "hugRUN", "airSYS", "config", "globals"};
    
    //parameters and their MIN/MAX used for SET CONFIG validation. Defined as array of apiGroups to simplify validation code
    struct apiParam {
      const char* name;
      unsigned short minVal;
      unsigned short maxVal;
    };
    const apiParam params_profileRUN[4] = {{"highPressure", 0, MaxPressure}, {"onTime", 0, 599}, {"offTime", 0, 599}, {"cycleTime", 0, 5999}};
    const apiParam params_hugRUN[2] =     {{"hugTime", 0, 599}, {"hugPressure", 0, MaxPressure}};
    const apiParam params_airSYS[3] =     {{"maxPumpPWM", 0, 51}, {"maxPressure", 0, MaxPressure}, {"thresholdPressure", 0, 5}};
    const apiParam params_config[3] =     {{"minPumpPWM", 200, 1023}, {"freq", 2, 199}, {"clearALL", 0, 1 }};
    const apiParam params_globals[1] =    {{"MaxPressure", 0, 50}};
    struct apiGroup {
      const char* name;
      const apiParam* params;
      size_t paramCount;
    };
    const apiGroup api_groups[5] = {
      { api_groupNames[0], params_profileRUN, sizeof(params_profileRUN) / sizeof(params_profileRUN[0]) },
      { api_groupNames[1], params_hugRUN, sizeof(params_hugRUN) / sizeof(params_hugRUN[0]) },
      { api_groupNames[2], params_airSYS, sizeof(params_airSYS) / sizeof(params_airSYS[0]) },
      { api_groupNames[3], params_config, sizeof(params_config) / sizeof(params_config[0]) },
      { api_groupNames[4], params_globals, sizeof(params_globals) / sizeof(params_globals[0]) },
    };

    //code that needs to be run outisde the UDP callback
    enum whatToRunOptions {nothing, profile, hug, newConfig}; 
    whatToRunOptions whatToRun = nothing;

    //aux function to search value in apiParam or apiGroup arrays
    int groupIndex = 99;
    template<size_t N>
    bool keyExists(const String& inputVar, const char* (&arr)[N]) { 
      for (size_t i = 0; i < N; ++i) {
        if (inputVar == arr[i]) {
          groupIndex = i;
          return true;
        }
      }
      return false;
    }

    StaticJsonDocument<200> docOUT;  //--------------------------------------------TO DO: confirm size
    String response = "";
    //catalogued error msgs of input JSON data in response packet
    void sendError(String msg) { 
      debug_println(msg);
      docOUT.clear();
      docOUT["method"] = "Error";
      docOUT["response"] = msg;
      response = "";
      serializeJson(docOUT, response);
    }

    //main UDP callback
    void validateUDP(AsyncUDPPacket packet) {  
      debug_print("UDP Packet Type: ");
      debug_print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
      debug_print(", From: ");
      debug_print(packet.remoteIP());
      debug_print(":");
      debug_print(packet.remotePort());
      debug_print(", Length: ");
      debug_print(packet.length());
      debug_print(", Data: ");
      debug_write(packet.data(), packet.length());
      debug_println("");

      debug_printf("pageNumber: %d\n", pageNumber);
      if (pageNumber > 1) { //on-device interface is in CONFIG mode. Can't operate it via API
        sendError("Device in CONFIG mode");
        return;
      }
      if (airSys.runType > 0) { //device is running inflation function. Can't operate it via API
        sendError("Device in Inflation cycle");
        return;
      }
      StaticJsonDocument<200> docIN;  //--------------------------------------------TO DO: confirm size
      DeserializationError error = deserializeJson(docIN, packet.data(), packet.length());
      if (error) {
        sendError("Invalid JSON string");
        return;
      }
      if (!docIN.containsKey("method") || !docIN.containsKey("group")) {
        sendError("Missing JSON key");
        return;
      }
      JsonVariant tempMethod = docIN["method"];
      JsonVariant tempGroup = docIN["group"];
      if (!tempMethod.is<const char*>() || !tempGroup.is<const char*>()) {
        sendError("Invalid key type");
        return;
      }
      const char* method = docIN["method"];
      if (!keyExists(method, api_methods)) {
        sendError("Method does not exist");
        return;
      }
      const char* group = docIN["group"];
      if (!keyExists(group, api_groupNames)) {
        sendError("Group does not exist");
        return;
      }

      //if flow reaches this point, then group exists & groupIndex != 99
      if (strcmp(method, "setConfig") == 0) {
        for (int i = 0; i < api_groups[groupIndex].paramCount; i++) {
          const apiParam& param = api_groups[groupIndex].params[i];
          if (!docIN.containsKey(param.name)) {
            sendError("Missing Parameter key");
            return;
          }
          JsonVariant tempParam = docIN[param.name];
          if (!tempParam.is<unsigned short>()) {
            sendError("Invalid Parameter type");
            return;
          }
          unsigned short value = tempParam.as<unsigned short>();
          if (value < param.minVal || value > param.maxVal) {
            sendError("Parameter out of range");
            return;
          }
        }

        //if flow reaches this point all data in JSON is valid
        debug_println("executing SET CONFIG...");
        int tempVar;
        switch (groupIndex) {
          case 0: tempVar = docIN["highPressure"];
                  profileVar.highPressure = tempVar * 689.4;
                  profileVar.onTime = docIN["onTime"];
                  profileVar.offTime = docIN["offTime"];
                  profileVar.cycleTime = docIN["cycleTime"];
                  storedData(3, true); // pageNumber(3) => PROFILE
                  break;
          case 1: hugVar.hugTime = docIN["hugTime"];
                  tempVar = docIN["hugPressure"];
                  hugVar.hugPressure = tempVar * 689.4;
                  storedData(4, true); // pageNumber(4) => HUG
                  break;
          case 2: airSys.maxPumpPWM = map(docIN["maxPumpPWM"], 0, 51, 0, 1023);
                  tempVar = docIN["maxPressure"];
                  airSys.maxPressure =  tempVar * 689.4;
                  tempVar = docIN["thresholdPressure"];
                  airSys.thresholdPressure = tempVar * 689.4;
                  storedData(5, true); // pagenumber(5) => airSYS
                  break;
          case 3: airSys.minPumpPWM = docIN["minPumpPWM"];
                  tempVar = docIN["freq"];
                  airSys.freq = tempVar * 50;
                  //storedData(7, true) is called in chkProcess(). This allows UDP callback to finish before ESP.restart
                  whatToRun = newConfig;
                  break; 
          case 4: MaxPressure = docIN["MaxPressure"];
                  //will be lost after next reboot since it's hard coded---------------------------
        }
        docOUT.clear();
        docOUT["method"] = docIN["method"];
        docOUT["group"] = docIN["group"];
        docOUT["response"] = "Success";
        response = "";
        serializeJson(docOUT, response);
        return;
      }

      debug_println("executing GET CONFIG");
      if (docIN["method"] == "getConfig") { //------------------testing vs (strcmp(method, "Set Config") == 0)
        docOUT.clear();
        docOUT["method"] = docIN["method"];
        docOUT["group"] = docIN["group"];
        switch (groupIndex) {
          case 0:  //profileRUN
            docOUT["highPressure"] = profileVar.highPressure;
            docOUT["onTime"] = profileVar.onTime;
            docOUT["offTime"] = profileVar.offTime;
            docOUT["cycleTime"] = profileVar.cycleTime;
            break;
          case 1:  //hugRUN
            docOUT["hugTime"] = hugVar.hugTime;
            docOUT["hugPressure"] = hugVar.hugPressure;
            break;
          case 2:  //airSYS
            docOUT["maxPumpPWM"] = airSys.maxPumpPWM;
            docOUT["maxPressure"] = airSys.maxPressure;
            docOUT["thresholdPressure"] = airSys.thresholdPressure;
            break;
          case 3:  //config
            docOUT["minPumpPWM"] = airSys.minPumpPWM;
            docOUT["freq"] = airSys.freq;
            break;
          case 4:  //globals
            docOUT["MaxPressure"] = MaxPressure;
            docOUT["battery"] = powerLevel * 0.00677;
        }
        response = "";
        serializeJson(docOUT, response);
        return;
      }

      debug_println("executing RUN");
      whatToRun = nothing;  //will be executed outside the UDP callback, in loop()
      if (docIN["method"] == "run" && docIN["group"] == "ProfileRUN")
          whatToRun = profile;
      if (docIN["method"] == "run" && docIN["group"] == "hugRUN") 
          whatToRun = hug;
      docOUT.clear();
      docOUT["method"] = docIN["method"];
      docOUT["group"] = docIN["group"];
      docOUT["response"] = "Success";
      response = "";
      serializeJson(docOUT, response);
      return;
    }    

    void connectToWiFi() {
      int connectionAttempts = 0;
      WiFi.disconnect(true);  //clear all credentials. Pass 'true' to also erase stored credentials
      while (WiFi.status() != WL_CONNECTED) {
        connectionAttempts++;
        if (connectionAttempts > 5) {
          debug_println("\nFailed to connect to WiFi after 3 attempts.");
          ESP.restart();
        }
        WiFi.mode(WIFI_STA);
        debug_printf("MAC Address:%s\n", WiFi.macAddress().c_str());
        #ifdef STATIC_IP
          if (!WiFi.config(staticIP, gateway, subnet)) { debug_println("STA Failed to configure"); }
        #endif
        debug_print("Connecting to WiFi.");
        WiFi.begin(ssid, password);
        int innerCounter = 0;
        while (WiFi.status() != WL_CONNECTED && innerCounter < 10) {  // wait for 2 seconds max
          delay(200);
          debug_print(".");
          innerCounter++;
        }
        if (WiFi.status() != WL_CONNECTED) {
          debug_println("\nConnection attempt failed. Trying again...");
          u8g2.drawStr(0, 60, ".....trying again.....");
          u8g2.sendBuffer(); 
          delay(3000);  // Wait before trying again
        }
      }
      debug_println("\nConnected to WiFi");
      debug_println(WiFi.localIP());
    }

  public:
    WiFi_API() {}

    void begin() {
      u8g2.clearBuffer();
      u8g2.setBitmapMode(1);
      u8g2.setDrawColor(1);
      u8g2.setFont(u8g2_font_helvB08_tr);
      u8g2.drawStr(0, 8,  "CONNECTING TO WIFI");
      u8g2.drawStr(0, 25, "....PLEASE WAIT...");
      u8g2.sendBuffer(); 

      connectToWiFi();
      if (udp.listen(UDPport)) {
        debug_print("UDP Listening on IP: ");
        debug_println(WiFi.localIP());

        //callback for UDP Server
        udp.onPacket([this](AsyncUDPPacket packet) {
          validateUDP(packet);
          packet.print(response);
        });
      }//-------------------------------------------------TO DO:else handle UDP listen not running
    }

    void chkProcess() {
      //------------------------------------------TO DO:add chk to confirm wifi connect. & decision if it can't
      if (whatToRun == newConfig) {
        whatToRun = nothing;
        storedData(7, true); //pagenumber is 7 => config. It calls ESP.restart
        return; //this will never be executed
      }
      if (whatToRun == hug) {
        if(airSys.runType > 0) STOP(); 
        else hugRun();
      }
      if (whatToRun == profile) {
        if(airSys.runType > 0) STOP(); 
        else profileRUN();
      }
      if (whatToRun != nothing) { //reset encoderConstrainVal varfiables as defined in encoderInteraction()
        whatToRun = nothing;
        encoderConstrainValMin = 0;
        encoderConstrainValMax = PageElements[pageNumber];
      }
    }
}; // end of class WiFi_API

WiFi_API wifiAPI; //instatiation of object of class WiFi_API referenced in setup() & loop()


#endif  //INCLUDE_WIFI_API_H

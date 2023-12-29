const char* ssid = "qENTERPRISE";
const char* password = "doctorhouse2020";
const int UDPport = 12345; 

#define DEBUG_PRINT  //if commented out, all debug serial output statments become empty when compiled

#define STATIC_IP  //if commented out, IP & DNS will come from router's DHCP
#ifdef STATIC_IP
  IPAddress staticIP(192, 168, 1, 69); //if this IP is reserved in router, connection can be faster
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(8, 8, 8, 8);
  IPAddress secondaryDNS(192, 168, 1, 1);
#endif
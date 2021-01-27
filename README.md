# ESPHTTPUpdateServer
A simple Arduino ESPHTTPUpdateServer for ESP32, forked from ESP8266HTTPUpdateServer.<br/>

Integrate this library and by typing \<hostname\>/update in your favourite Webbrowser a simple OTA Webupdater is for ESP32 Boards available. See at examples, there is a simple sketch and a sketch working for both ESP8266 and ESP32.<br/>
 
```yaml
setup() {
    [...]
    WebServer httpServer(80); 
    ESPHTTPUpdateServer httpUpdater;
    
    httpUpdater.setup(&httpServer);
    httpServer.begin();
    [...]
 }
 
 loop {
   // do some stuff here
 }
```

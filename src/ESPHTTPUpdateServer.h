
#ifndef ESP_HTTP_UPDATE_SERVER_H
#define ESP_HTTP_UPDATE_SERVER_H

#include <WebServer.h>
//#include <WiFiUdp.h>
//#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "StreamString.h"

class ESPHTTPUpdateServer {

  public:
    ESPHTTPUpdateServer(bool serial_debug);
    ESPHTTPUpdateServer();
    void    setup(WebServer *server);
    void    setup(WebServer *server, const String& path, const String& username, const String& password);

  private:
    WebServer *_server;

    String    _username;
    String    _password;
    bool      _authenticated;
    String    _updaterError;
    bool      _serial_output;

    void _setUpdaterError();
};


#endif

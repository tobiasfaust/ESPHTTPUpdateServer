#include "ESPHTTPUpdateServer.h"

const char UpdateIndex[] PROGMEM = R"=====(
     <!DOCTYPE html>
     <html lang='en'>
     <head>
         <meta charset='utf-8'>
         <meta name='viewport' content='width=device-width,initial-scale=1'/>
     </head>
     <body>
     <form method='POST' action='' enctype='multipart/form-data'>
         Firmware:<br>
         <input type='file' accept='.bin,.bin.gz' name='firmware'>
         <input type='submit' value='Update Firmware'>
     </form>
<!--
     <form method='POST' action='' enctype='multipart/form-data'>
         FileSystem:<br>
         <input type='file' accept='.bin,.bin.gz' name='filesystem'>
         <input type='submit' value='Update FileSystem'>
     </form>
-->
     </body>
     </html>
)=====";
     
const char UpdateSuccessResponse[] PROGMEM = R"=====(
  <META http-equiv=\"refresh\" content=\"15;URL=/\">Update Success! Rebooting...
)=====";

ESPHTTPUpdateServer::ESPHTTPUpdateServer() {
  ESPHTTPUpdateServer(false);
}

ESPHTTPUpdateServer::ESPHTTPUpdateServer(bool serial_debug) {
  _serial_output = serial_debug;
  _server = NULL;
  _username = emptyString;
  _password = emptyString;
  _authenticated = false;

  if (this->_serial_output) {
    Serial.printf("Serial Debug Output enabled \n");
    Serial.setDebugOutput(true);
  } else {
    Serial.printf("Serial Debug Output disabled \n");
    Serial.setDebugOutput(false);
  }
  
}

void ESPHTTPUpdateServer::setup(WebServer *server) {
  this->setup(server, "/update", this->_username, this->_password);
}

void ESPHTTPUpdateServer::setup(WebServer *server, const String& path, const String& username, const String& password) {
    this->_server = server;
    this->_username = username;
    this->_password = password;

    // handler for the /update form page
    this->_server->on(path.c_str(), HTTP_GET, [&](){
      if(this->_username != emptyString && this->_password != emptyString && !this->_server->authenticate(this->_username.c_str(), this->_password.c_str()))
        return _server->requestAuthentication();
        
      this->_server->send_P(200, PSTR("text/html"), UpdateIndex);
    });

    // handler for the /update form POST (once file upload finishes)
    this->_server->on(path.c_str(), HTTP_POST, [&](){
      if(!this->_authenticated)
        return _server->requestAuthentication();
      
      if (Update.hasError()) {
        this->_server->send(200, F("text/html"), String(F("Update error: ")) + this->_updaterError);
      }
       
      else {
        this->_server->client().setNoDelay(true);
        this->_server->send_P(200, PSTR("text/html"), UpdateSuccessResponse);
        delay(100);
        this->_server->client().stop();
        ESP.restart();
      }
    },[&](){
      // handler for the file upload, get's the sketch bytes, and writes
      // them through the Update object
      HTTPUpload& upload = this->_server->upload();

      if(upload.status == UPLOAD_FILE_START) {
        this->_updaterError.clear();

        this->_authenticated = (this->_username == emptyString || this->_password == emptyString || this->_server->authenticate(this->_username.c_str(), this->_password.c_str()));
        
        if(!this->_authenticated) {
          if (this->_serial_output)  Serial.printf("Unauthenticated Update\n");
          return;
        }
        else {
          if (this->_serial_output) Serial.printf("Authenticated Update\n");
        }

        //WiFiUDP::stopAll();
        if (this->_serial_output)
          Serial.printf("Update: %s\n", upload.filename.c_str());
          
        if (upload.name == "filesystem") {
//          size_t fsSize = ((size_t) &_FS_end - (size_t) &_FS_start);
//          close_all_fs();
//          if (!Update.begin(fsSize, U_FS)){//start with max available size
//            if (_serial_output) Update.printError(Serial);
//          }
        } 
        
        else if (upload.name == "firmware") {
          if (this->_serial_output) Serial.printf("Processing firmware ...\n");
          
          uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
          if (!Update.begin(maxSketchSpace, U_FLASH)) { //start with max available size
            this->_setUpdaterError();
          }
        } 
      }

      else if(this->_authenticated && upload.status == UPLOAD_FILE_WRITE && !this->_updaterError.length()){
        if (this->_serial_output) Serial.printf(".");
          
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          this->_setUpdaterError();
        }
      } 
      
      else if(this->_authenticated && upload.status == UPLOAD_FILE_END && !this->_updaterError.length()){
        if(Update.end(true)){ //true to set the size to the current progress
          if (this->_serial_output) Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          this->_setUpdaterError();
        }
        if (this->_serial_output) Serial.setDebugOutput(false);
      } 
      
      else if(this->_authenticated && upload.status == UPLOAD_FILE_ABORTED) {
        Update.end();
        if (this->_serial_output) Serial.println("Update was aborted");
      }
      delay(0);     
   });
}

void ESPHTTPUpdateServer::_setUpdaterError() {
  if (this->_serial_output) Update.printError(Serial);
  StreamString str;
  Update.printError(str);
  this->_updaterError = str.c_str();
}

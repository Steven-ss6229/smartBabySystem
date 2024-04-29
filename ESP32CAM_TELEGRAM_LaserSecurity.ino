
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

const char* ssid = "VARMA"; //--> Enter your SSID / your WiFi network name.
const char* password = "abhi1234"; //--> Enter your WiFi password.

String BOTtoken = "6374288023:AAGEVFVOhEhts2RQIyt_Co4daiTZ4RhadIo";  //--> your Bot Token (Get from Botfather).

String CHAT_ID = "1141566451";
WiFiClientSecure clientTCP;

UniversalTelegramBot bot(BOTtoken, clientTCP);

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define ON HIGH
#define OFF LOW

#define FLASH_LED_PIN   4           //--> LED Flash PIN (GPIO 4)
#define ldr_SENSOR_PIN  12          //--> ldr SENSOR PIN (GPIO 12)

#define EEPROM_SIZE     2           //--> Define the number of bytes you want to access

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

int countdown_interval_to_stabilize_ldr_Sensor = 1000;
unsigned long lastTime_countdown_Ran;
byte countdown_to_stabilize_ldr_Sensor = 30;
/* ======================================== */

bool sendPhoto = false;             //--> Variables for photo sending triggers.

bool ldr_Sensor_is_stable = false;  //--> Variable to state that the ldr sensor stabilization time has been completed.

bool boolldrState = false;

String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void FB_MSG_is_photo_send_successfully (bool state) {
  String send_feedback_message = "";
  if(state == false) {
    send_feedback_message += "From the ESP32-CAM :\n\n";
    send_feedback_message += "ESP32-CAM failed to send photo.\n";
    send_feedback_message += "Suggestion :\n";
    send_feedback_message += "- Please try again.\n";
    send_feedback_message += "- Reset ESP32-CAM.\n";
    send_feedback_message += "- Change FRAMESIZE (see Drop down frame size in void configInitCamera).\n";
    Serial.print(send_feedback_message);
    send_feedback_message += "\n\n";
    send_feedback_message += "/start : to see all commands.";
    bot.sendMessage(CHAT_ID, send_feedback_message, "");
  } else {
    if(boolldrState == true) {
      Serial.println("Successfully sent photo.");
      send_feedback_message += "From the ESP32-CAM :\n\n";
      send_feedback_message += "The ldr sensor detects objects and movements.\n";
      send_feedback_message += "Photo sent successfully.\n\n";
      send_feedback_message += "/start : to see all commands.";
      bot.sendMessage(CHAT_ID, send_feedback_message, ""); 
    }
    if(sendPhoto == true) {
      Serial.println("Successfully sent photo.");
      send_feedback_message += "From the ESP32-CAM :\n\n";
      send_feedback_message += "Photo sent successfully.\n\n";
      send_feedback_message += "/start : to see all commands.";
      bot.sendMessage(CHAT_ID, send_feedback_message, ""); 
    }
  }
}

bool ldr_State() {
  bool PRS = digitalRead(ldr_SENSOR_PIN);
  return PRS;
}


void LEDFlash_State(bool ledState) {
  digitalWrite(FLASH_LED_PIN, ledState);
}

void enable_capture_Photo_With_Flash(bool state) {
  EEPROM.write(0, state);
  EEPROM.commit();
  delay(50);
}
bool capture_Photo_With_Flash_state() {
  bool capture_Photo_With_Flash = EEPROM.read(0);
  return capture_Photo_With_Flash;
}
void enable_capture_Photo_with_ldr(bool state) {
  EEPROM.write(1, state);
  EEPROM.commit();
  delay(50);
}
bool capture_Photo_with_ldr_state() {
  bool capture_Photo_with_ldr = EEPROM.read(1);
  return capture_Photo_with_ldr;
}

void configInitCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; //--> FRAMESIZE_ + UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
    config.jpeg_quality = 10;  
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  
    config.fb_count = 1;
  }
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    Serial.println();
    Serial.println("Restart ESP32 Cam");
    delay(1000);
    ESP.restart();
  }
  
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_SXGA);  //--> FRAMESIZE_ + UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
  /* ---------------------------------------- */
}

void handleNewMessages(int numNewMessages) {
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  /* ---------------------------------------- "For Loop" to check the contents of the newly received message. */
  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      Serial.println("Unauthorized user");
      Serial.println("------------");
      continue;
    }
    /* ::::::::::::::::: */

    /* ::::::::::::::::: Print the received message. */
    String text = bot.messages[i].text;
    Serial.println(text);
   
   String send_feedback_message = "";
    String from_name = bot.messages[i].from_name;
    if (text == "/start") {
      send_feedback_message += "From the ESP32-CAM :\n\n";
      send_feedback_message += "Welcome , " + from_name + "\n";
      send_feedback_message += "Use the following commands to interact with the ESP32-CAM.\n\n";
      send_feedback_message += "/capture_photo : takes a new photo\n\n";
      send_feedback_message += "Settings :\n";
      send_feedback_message += "/enable_capture_Photo_With_Flash : takes a new photo with LED FLash\n";
      send_feedback_message += "/disable_capture_Photo_With_Flash : takes a new photo without LED FLash\n";
      send_feedback_message += "/enable_capture_Photo_with_ldr : takes a new photo with ldr Sensor\n";
      send_feedback_message += "/disable_capture_Photo_with_ldr : takes a new photo without ldr Sensor\n\n";
      send_feedback_message += "Setting status :\n";
      if(capture_Photo_With_Flash_state() == ON) {
        send_feedback_message += "- Capture Photo With Flash = ON\n";
      }
      if(capture_Photo_With_Flash_state() == OFF) {
        send_feedback_message += "- Capture Photo With Flash = OFF\n";
      }
      if(capture_Photo_with_ldr_state() == ON) {
        send_feedback_message += "- Capture Photo With ldr = ON\n";
      }
      if(capture_Photo_with_ldr_state() == OFF) {
        send_feedback_message += "- Capture Photo With ldr = OFF\n";
      }
      if(ldr_Sensor_is_stable == false) {
        send_feedback_message += "\nldr Sensor Status:\n";
        send_feedback_message += "The ldr sensor is being stabilized.\n";
        send_feedback_message += "Stabilization time is " + String(countdown_to_stabilize_ldr_Sensor) + " seconds away. Please wait.\n";
      }
      bot.sendMessage(CHAT_ID, send_feedback_message, "");
      Serial.println("------------");
    }
    
    // The condition if the command received is "/capture_photo".
    if (text == "/capture_photo") {
      sendPhoto = true;
      Serial.println("New photo request");
    }
    
    // The condition if the command received is "/enable_capture_Photo_With_Flash".
    if (text == "/enable_capture_Photo_With_Flash") {
      enable_capture_Photo_With_Flash(ON);
      send_feedback_message += "From the ESP32-CAM :\n\n";
      if(capture_Photo_With_Flash_state() == ON) {
        Serial.println("Capture Photo With Flash = ON");
        send_feedback_message += "Capture Photo With Flash = ON\n\n";
      } else {
        Serial.println("Failed to set. Try again.");
        send_feedback_message += "Failed to set. Try again.\n\n"; 
      }
      Serial.println("------------");
      send_feedback_message += "/start : to see all commands.";
      bot.sendMessage(CHAT_ID, send_feedback_message, "");
    }
    
    // The condition if the command received is "/disable_capture_Photo_With_Flash".
    if (text == "/disable_capture_Photo_With_Flash") {
      enable_capture_Photo_With_Flash(OFF);
      send_feedback_message += "From the ESP32-CAM :\n\n";
      if(capture_Photo_With_Flash_state() == OFF) {
        Serial.println("Capture Photo With Flash = OFF");
        send_feedback_message += "Capture Photo With Flash = OFF\n\n";
      } else {
        Serial.println("Failed to set. Try again.");
        send_feedback_message += "Failed to set. Try again.\n\n"; 
      }
      Serial.println("------------");
      send_feedback_message += "/start : to see all commands.";
      bot.sendMessage(CHAT_ID, send_feedback_message, "");
    }

    // The condition if the command received is "/enable_capture_Photo_with_ldr".
    if (text == "/enable_capture_Photo_with_ldr") {
      enable_capture_Photo_with_ldr(ON);
      send_feedback_message += "From the ESP32-CAM :\n\n";
      if(capture_Photo_with_ldr_state() == ON) {
        Serial.println("Capture Photo With ldr = ON");
        send_feedback_message += "Capture Photo With ldr = ON\n\n";
        botRequestDelay = 20000;
      } else {
        Serial.println("Failed to set. Try again.");
        send_feedback_message += "Failed to set. Try again.\n\n"; 
      }
      Serial.println("------------");
      send_feedback_message += "/start : to see all commands.";
      bot.sendMessage(CHAT_ID, send_feedback_message, "");
    }

    // The condition if the command received is "/disable_capture_Photo_with_ldr".
    if (text == "/disable_capture_Photo_with_ldr") {
      enable_capture_Photo_with_ldr(OFF);
      send_feedback_message += "From the ESP32-CAM :\n\n";
      if(capture_Photo_with_ldr_state() == OFF) {
        Serial.println("Capture Photo With ldr = OFF");
        send_feedback_message += "Capture Photo With ldr = OFF\n\n";
        botRequestDelay = 1000;
      } else {
        Serial.println("Failed to set. Try again.");
        send_feedback_message += "Failed to set. Try again.\n\n"; 
      }
      Serial.println("------------");
      send_feedback_message += "/start : to see all commands.";
      bot.sendMessage(CHAT_ID, send_feedback_message, "");
    }
    /* ::::::::::::::::: */
  }
  /* ---------------------------------------- */
}

String sendPhotoTelegram() {
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  /* ---------------------------------------- The process of taking photos. */
  Serial.println("Taking a photo...");

  if(capture_Photo_With_Flash_state() == ON) {
    LEDFlash_State(ON);
  }
  delay(1500);
  
  /* ::::::::::::::::: Taking a photo. */ 
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    Serial.println("Restart ESP32 Cam");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }  
  /* ::::::::::::::::: */

  if(capture_Photo_With_Flash_state() == ON) {
    LEDFlash_State(OFF);
  }
  /* ::::::::::::::::: */
  Serial.println("Successful photo taking.");
  /* ---------------------------------------- */
  

  /* ---------------------------------------- The process of sending photos. */
  Serial.println("Connect to " + String(myDomain));

  if (clientTCP.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    Serial.print("Send photos");
    
    String head = "--Esp32Cam\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n";
    head += CHAT_ID; 
    head += "\r\n--Esp32Cam\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--Esp32Cam--\r\n";


    uint32_t imageLen = fb->len;
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;
  
    clientTCP.println("POST /bot"+BOTtoken+"/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=Esp32Cam");
    clientTCP.println();
    clientTCP.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        clientTCP.write(fbBuf, remainder);
      }
    }  
    
    clientTCP.print(tail);
    
    esp_camera_fb_return(fb);
    
    int waitTime = 10000;   //--> timeout 10 seconds (To send photos.)
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + waitTime) > millis()){
      Serial.print(".");
      delay(100);      
      while (clientTCP.available()) {
        char c = clientTCP.read();
        if (state==true) getBody += String(c);        
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length()>0) break;
    }
    clientTCP.stop();
    Serial.println(getBody);

    /* ::::::::::::::::: The condition to check if the photo was sent successfully or failed. */
    // If the photo is successful or failed to send, a feedback message will be sent to Telegram.
    if(getBody.length() > 0) {
      String send_status = "";
      send_status = getValue(getBody, ',', 0);
      send_status = send_status.substring(6);
      
      if(send_status == "true") {
        FB_MSG_is_photo_send_successfully(true);  //--> The photo was successfully sent and sent an information message that the photo was successfully sent to telegram.
      }
      if(send_status == "false") {
        FB_MSG_is_photo_send_successfully(false); //--> The photo failed to send and sends an information message that the photo failed to send to telegram.
      }
    }
    if(getBody.length() == 0) FB_MSG_is_photo_send_successfully(false); //--> The photo failed to send and sends an information message that the photo failed to send to telegram.
    /* ::::::::::::::::: */
  }
  else {
    getBody="Connected to api.telegram.org failed.";
    Serial.println("Connected to api.telegram.org failed.");
  }
  Serial.println("------------");
  return getBody;
  /* ---------------------------------------- */
}
/* ________________________________________________________________________________ */

/* ________________________________________________________________________________ VOID SETTUP() */
void setup(){
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //--> Disable brownout detector.

   /* ---------------------------------------- Init serial communication speed (baud rate). */
  Serial.begin(115200);
  delay(1000);
  /* ---------------------------------------- */

  Serial.println();
  Serial.println();
  Serial.println("------------");

  /* ---------------------------------------- Starts the EEPROM, writes and reads the settings stored in the EEPROM. */
  EEPROM.begin(EEPROM_SIZE);

  enable_capture_Photo_With_Flash(OFF);
  enable_capture_Photo_with_ldr(OFF);
  delay(500);
  /* ::::::::::::::::: */

  Serial.println("Setting status :");
  if(capture_Photo_With_Flash_state() == ON) {
    Serial.println("- Capture Photo With Flash = ON");
  }
  if(capture_Photo_With_Flash_state() == OFF) {
    Serial.println("- Capture Photo With Flash = OFF");
  }
  if(capture_Photo_with_ldr_state() == ON) {
    Serial.println("- Capture Photo With ldr = ON");
    botRequestDelay = 20000;
  }
  if(capture_Photo_with_ldr_state() == OFF) {
    Serial.println("- Capture Photo With ldr = OFF");
    botRequestDelay = 1000;
  }

  pinMode(FLASH_LED_PIN, OUTPUT);
  LEDFlash_State(OFF);

  Serial.println();
  Serial.println("Start configuring and initializing the camera...");
  configInitCamera();
  Serial.println("Successfully configure and initialize the camera.");
  Serial.println();

  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); //--> Add root certificate for api.telegram.org

 
  int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    LEDFlash_State(ON);
    delay(250);
    LEDFlash_State(OFF);
    delay(250);
    if(connecting_process_timed_out > 0) connecting_process_timed_out--;
    if(connecting_process_timed_out == 0) {
      delay(1000);
      ESP.restart();
    }
  }
  
  LEDFlash_State(OFF);
  Serial.println();
  Serial.print("Successfully connected to ");
  Serial.println(ssid);
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.println("The ldr sensor is being stabilized.");
  Serial.printf("Stabilization time is %d seconds away. Please wait.\n", countdown_to_stabilize_ldr_Sensor);
  
  Serial.println("------------");
  Serial.println();
  /* ---------------------------------------- */
}

void loop() {
  /* ---------------------------------------- Conditions for taking and sending photos. */
  if(sendPhoto) {
    Serial.println("Preparing photo...");
    sendPhotoTelegram(); 
    sendPhoto = false; 
  }

  if(millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println();
      Serial.println("------------");
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

  if(ldr_Sensor_is_stable == false) {
    if(millis() > lastTime_countdown_Ran + countdown_interval_to_stabilize_ldr_Sensor) {
      if(countdown_to_stabilize_ldr_Sensor > 0) countdown_to_stabilize_ldr_Sensor--;
      if(countdown_to_stabilize_ldr_Sensor == 0) {
        ldr_Sensor_is_stable = true;
        Serial.println();
        Serial.println("------------");
        Serial.println("The ldr Sensor stabilization time is complete.");
        Serial.println("The ldr sensor can already work.");
        Serial.println("------------");
        String send_Status_ldr_Sensor = "";
        send_Status_ldr_Sensor += "From the ESP32-CAM :\n\n";
        send_Status_ldr_Sensor += "The ldr Sensor stabilization time is complete.\n";
        send_Status_ldr_Sensor += "The ldr sensor can already work.";
        bot.sendMessage(CHAT_ID, send_Status_ldr_Sensor, "");
      }
      lastTime_countdown_Ran = millis();
    }
  }
  

  if(capture_Photo_with_ldr_state() == ON) {
    if(ldr_State() == true && ldr_Sensor_is_stable == true) {
      Serial.println("------------");
      Serial.println("The ldr sensor detects objects and movements.");
      boolldrState = true;
      sendPhotoTelegram();
      boolldrState = false;
    }
  }
  /* ---------------------------------------- */
}
 
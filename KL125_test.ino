// #############################################################################
// #############################################################################
// #############################################################################

#include "Device_Controller.h"
#include <WiFi.h>

const char* ssid = "Troy And Abed In A Bubble";
const char* password = "Community-19";

IPAddress bulbIP = { 192, 168, 2, 141 };
BulbController bulb(bulbIP, 9999);

String response;

int lastButtonState;
int currentButtonState = digitalRead(17);
int a = 0;
// #############################################################################

void setup()
{
    Serial.begin(115200);
    Serial.println("Booting");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    pinMode(17, INPUT_PULLUP);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }

    Serial.println("Plug may stop reacting and responding when sending several consecutive commands");



    response = bulb.off();
    if (response == "")
        Serial.println("No response from plug or not connected");
    else
        Serial.println(response);
    delay(1000);

       
    response = bulb.on();
    if (response == "")
        Serial.println("No response from plug or not connected");
    else
        Serial.println(response);
    delay(1000);

    response = bulb.setcolour(100, 120);
    if (response == "")
        Serial.println("No response from plug or not connected");
    else
        Serial.println(response);
    delay(1000);

    response = bulb.settemp(4555);
    if (response == "")
        Serial.println("No response from plug or not connected");
    else
        Serial.println(response);
    delay(1000);
    
    response = bulb.getInfo();
    if (response == "")
        Serial.println("No response from plug or not connected");
    else
        Serial.println(response);
    delay(1000);

    
    Serial.println("Testing done");

    
}

// #############################################################################

void loop()
{

    lastButtonState = currentButtonState;
    currentButtonState = digitalRead(17);
    //Serial.println(currentButtonState);
    if(lastButtonState == LOW && currentButtonState == HIGH){
        if (a == 1) {
           response = bulb.off();
           a = 0;
        }
        else{
           response = bulb.on();
           a = 1;
        }
    }
}
// #############################################################################
// #############################################################################
// #############################################################################

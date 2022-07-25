// #############################################################################
// #############################################################################
// #############################################################################

// TP-Link HS100/HS110 SmartBulb Controller for ESP8266/ESP32
// by Miraculix200 (not affiliated with TP-Link)
// License: MIT

// Code may stop working any time if TP-Link changes their firmware

// List of other commands: 
// https://www.briandorey.com/post/tp-link-lb130-smart-wi-fi-led-bulb-python-control

// #############################################################################
// #############################################################################
// #############################################################################

#include "Device_Controller.h"
// #############################################################################

BulbController::BulbController(IPAddress ip, uint16_t port)
{
    targetIP = ip;
    targetPort = port;
}

// #############################################################################

void BulbController::serializeUint32(char (&buf)[4], uint32_t val)
{
    buf[0] = (val >> 24) & 0xff;
    buf[1] = (val >> 16) & 0xff;
    buf[2] = (val >> 8) & 0xff;
    buf[3] = val & 0xff;
}

// #############################################################################

void BulbController::decrypt(char* input, uint16_t length)
{
    uint8_t key = 171;
    uint8_t next_key;
    for (uint16_t i = 0; i < length; i++) {
        next_key = input[i];
        input[i] = key ^ input[i];
        key = next_key;
    }
}

// #############################################################################

void BulbController::encrypt(char* data, uint16_t length)
{
    uint8_t key = 171;
    for (uint16_t i = 0; i < length + 1; i++) {
        data[i] = key ^ data[i];
        key = data[i];
    }
}

// #############################################################################

void BulbController::encryptWithHeader(char* out, char* data, uint16_t length)
{
    char serialized[4];
    serializeUint32(serialized, length);
    encrypt(data, length);
    memcpy(out, &serialized, 4);
    memcpy(out + 4, data, length);
}

// #############################################################################

String BulbController::getInfo()
{
    const String cmd = "{\"system\":{\"get_sysinfo\":{}}}";
    return sendCmd(cmd);
}

// #############################################################################

String BulbController::on()
{
    const String cmd = "{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"on_off\":1,\"transition_period\":0}}}";
    return sendCmd(cmd);
}

// #############################################################################

String BulbController::off()
{
    const String cmd = "{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"on_off\":0,\"transition_period\":0}}}";
    return sendCmd(cmd);
}

// #############################################################################

// #############################################################################

String BulbController::setcolour(uint16_t saturation, uint16_t hue)
{
    const String cmd = "{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"saturation\":" + String(saturation) + ",\"hue\":" + String(hue) + ",\"color_temp\":0,\"transition_period\":0}}}";
    return sendCmd(cmd);
}

// #############################################################################

String BulbController::settemp(uint16_t temp)
{
    const String cmd = "{\"smartlife.iot.smartbulb.lightingservice\":{\"transition_light_state\":{\"color_temp\":" + String(temp) + ",\"transition_period\":0}}}";
    return sendCmd(cmd);
}


// #############################################################################

String BulbController::sendCmd(String cmd)
{
    char encrypted[cmd.length() + 4];
    encryptWithHeader(encrypted, const_cast<char*>(cmd.c_str()), cmd.length());
    char response[2048] = { 0 };
    uint16_t length = this->tcpConnect(response, encrypted, cmd.length() + 4, 5000);
    if (length > 0)
        decrypt(response, length - 4);
    else
        return String("");
    return String(response);
}

// #############################################################################

uint16_t BulbController::tcpConnect(char* out, const char* cmd, uint16_t length, unsigned long timeout_millis)
{
    WiFiClient Bulb_client;
    if (Bulb_client.connect(this->targetIP, this->targetPort))
    {
        delay(10);
        Bulb_client.write(cmd, length);

        // give the Bulb some time to think about its response ^^ 
        // otherwise there may be no response
        // while waiting, pat the dog for ESP8266
        unsigned long start = millis();
        while(millis() - start < 500)
        {
            delay(10); // this includes yield() to pat the dog
        }
        
        start = millis();
        char buf[2048] = { 0 };
        while (Bulb_client.connected()) {
            if (Bulb_client.available()) {
                delay(10);
                int len = Bulb_client.read((uint8_t*)buf, 2048);

                // necessary for decryption
                // buf + 4 strips 4 byte header
                // valread - 3 leaves 1 byte for terminating null character
                strncpy(out, buf + 4, len - 3);

                delay(10);
                Bulb_client.flush();
                return len;
            }

            if (millis() - start >= timeout_millis) {
                // connection timeout
                break;
            }
            delay(0);
        }
        // timeout/not connected
        delay(10);
        Bulb_client.flush();
    }
    return 0;
}

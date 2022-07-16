#include "configuration.h"
#include "TriggerPlugin.h"
#include "NodeDB.h"
#include "PowerFSM.h"
#include "CannedMessagePlugin.h"
#include "driver/uart.h"

TriggerPlugin* triggerPlugin;

#define servo_pin 2
#define arm_pin 13

TriggerPlugin::TriggerPlugin() : concurrency::OSThread("trigger"){
    triggerServo.setPeriodHertz(50);
    triggerServo.attach(servo_pin);
    //GoToInitPos();
    GoToOffPos();
}

void TriggerPlugin::GoToInitPos(){
    pinMode(arm_pin, OUTPUT);
    digitalWrite(arm_pin, HIGH);
    triggerServo.writeMicroseconds(1300);
    delay(1000);
    digitalWrite(arm_pin, LOW);
}

void TriggerPlugin::GoToOffPos(){
    pinMode(arm_pin, OUTPUT);
    digitalWrite(arm_pin, HIGH);
    triggerServo.writeMicroseconds(2000);
    delay(1000);
    digitalWrite(arm_pin, LOW);
}

void TriggerPlugin::AttemptLink()
{
        if(!linkReqd){
            cannedMessagePlugin->sendText(NODENUM_BROADCAST, "linkReq", false);
        }else{
            cannedMessagePlugin->sendText(reqd_from, "linkAck", false);
        }
}

void TriggerPlugin::SendTrigger(){
    if(devicestate.is_linked){
        cannedMessagePlugin->sendText(devicestate.linked_id, "fire", false);
    }
}

void TriggerPlugin::SendFullTriggerSequence(){
    if(devicestate.is_linked){
        cannedMessagePlugin->sendText(devicestate.linked_id, "enable", false);
        delay(500);
        cannedMessagePlugin->sendText(devicestate.linked_id, "arm", false);
        delay(500);
        cannedMessagePlugin->sendText(devicestate.linked_id, "fire", false);
        delay(500);
    }
}

void TriggerPlugin::SendLinkTerm(){
        cannedMessagePlugin->sendText(NODENUM_BROADCAST, "linkTerm", false);
}

void TriggerPlugin::TriggerServo(){
    if(isEnabled && isArmed){
        timeAtArming = millis();
        digitalWrite(arm_pin, HIGH);
        triggerServo.writeMicroseconds(1000);
        delay(10000);
        triggerServo.writeMicroseconds(2000);
        digitalWrite(arm_pin, LOW);
    }
}

void TriggerPlugin::TriggerServo(int timeMillis){
    digitalWrite(arm_pin, HIGH);
    triggerServo.writeMicroseconds(1000);
    delay(timeMillis);
    triggerServo.writeMicroseconds(2000);
    delay(1000);
    triggerServo.writeMicroseconds(0);
    digitalWrite(arm_pin, LOW);
}

void TriggerPlugin::TriggerServo(int microWrite, int timeMillis){
    digitalWrite(arm_pin, HIGH);
    triggerServo.writeMicroseconds(microWrite);
    delay(timeMillis);
    triggerServo.writeMicroseconds(2000);
    delay(1000);
    triggerServo.writeMicroseconds(0);
    digitalWrite(arm_pin, LOW);
}

void TriggerPlugin::TriggerRelay(){
    if(isEnabled && isArmed){
        digitalWrite(arm_pin, HIGH);
        delay(1000);
        digitalWrite(arm_pin, LOW);
    }
}

void TriggerPlugin::TriggerSerial(){
    if(isEnabled && isArmed){
        char msg[50] = "trigger enable\n\r";
        int len = strlen(msg);
        uart_write_bytes(UART_NUM_2, msg, len);

        delay(200);

        strcpy(msg, "trigger arm\n\r");
        len = strlen(msg);
        uart_write_bytes(UART_NUM_2, msg, len);

        delay(200);

        strcpy(msg, "trigger fire\n\r");
        len = strlen(msg);
        uart_write_bytes(UART_NUM_2, msg, len);
    }
}

void TriggerPlugin::Arm(){
    digitalWrite(arm_pin, HIGH);
    isArmed = true;
    timeAtArming = millis();
}

int32_t TriggerPlugin::runOnce(){
    // if(isAttemptingLink && millis() - timeAtLastReq > 800 && !isLinked){
    //     num_reqs_sent++;
    //     char buffer[20] = "";
    //     char tempbuff[5];
    //     strcat(buffer, "linkReq ");
    //     strcat(buffer, itoa(num_reqs_sent, tempbuff, 10));
    //     Serial.println(buffer);
    //     cannedMessagePlugin->sendText(NODENUM_BROADCAST, buffer, false);
    //     timeAtLastReq = millis();
    // }
    // if(num_reqs_sent >= 20 || isLinked){
    //     isAttemptingLink = false;
    // }
    // return 100;

    char radio_json[3000];
    char intbuf[16];
    strcpy(radio_json, "[");
    for(int i = 0; i < nodeDB.getNumNodes(); i++){
        NodeInfo node = *nodeDB.getNodeByIndex(i);
        strcat(radio_json, "{\"id\":\"");
        strcat(radio_json, node.user.id);
        strcat(radio_json, "\",\"name\":\"");
        strcat(radio_json, node.user.long_name);
        strcat(radio_json, "\",\"num\":");
        strcat(radio_json, utoa(node.num, intbuf, 10));
        strcat(radio_json, ",\"lat\":");
        strcat(radio_json, itoa(node.position.latitude_i, intbuf, 10));
        strcat(radio_json, ",\"lon\":");
        strcat(radio_json, itoa(node.position.longitude_i, intbuf, 10));
        strcat(radio_json, ",\"bat\":");
        strcat(radio_json, itoa(node.position.battery_level, intbuf, 10));
        strcat(radio_json, ",\"last_heard\":");
        strcat(radio_json, utoa(node.last_heard, intbuf, 10));
        strcat(radio_json, "},");
    }
    radio_json[strlen(radio_json)-1] = 0;
    strcat(radio_json, "]");

    Serial.println(radio_json);

    if(isArmed && millis()-timeAtArming > 16000){
        Serial.println("disarming");
        triggerServo.writeMicroseconds(2000);
        cannedMessagePlugin->sendText(NODENUM_BROADCAST, "wrote micro", false);
        delay(3000);

        isArmed = false;
        digitalWrite(arm_pin, LOW);
        timeAtArming = 0;
    }

    return 500;
}



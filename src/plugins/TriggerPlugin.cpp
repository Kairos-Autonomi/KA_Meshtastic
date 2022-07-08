#include "configuration.h"
#include "TriggerPlugin.h"
#include "NodeDB.h"
#include "PowerFSM.h"
#include "CannedMessagePlugin.h"
#include "driver/uart.h"

TriggerPlugin* triggerPlugin;

TriggerPlugin::TriggerPlugin() : concurrency::OSThread("trigger"){
    triggerServo.setPeriodHertz(50);
    triggerServo.attach(15);
    triggerServo.writeMicroseconds(2000);
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
        cannedMessagePlugin->sendText(devicestate.linked_id, "trigger", false);
    }
}

void TriggerPlugin::SendLinkTerm(){
        cannedMessagePlugin->sendText(NODENUM_BROADCAST, "linkTerm", false);
}

void TriggerPlugin::TriggerServo(){
    triggerServo.writeMicroseconds(1000);
    delay(1000);
    triggerServo.writeMicroseconds(2000);
}

void TriggerPlugin::TriggerRelay(){
    digitalWrite(13, HIGH);
    delay(1000);
    digitalWrite(13, LOW);
}

void TriggerPlugin::TriggerSerial(){
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

    char pos_json[1000];
    char intbuf[16];
    strcpy(pos_json, "[");
    for(int i = 0; i < nodeDB.getNumNodes(); i++){
        NodeInfo node = *nodeDB.getNodeByIndex(i);
        strcat(pos_json, "{\"id\":\"");
        strcat(pos_json, node.user.id);
        strcat(pos_json, "\",\"name\":\"");
        strcat(pos_json, node.user.long_name);
        strcat(pos_json, "\",\"lat\":");
        strcat(pos_json, itoa(node.position.latitude_i, intbuf, 10));
        strcat(pos_json, ",\"lon\":");
        strcat(pos_json, itoa(node.position.longitude_i, intbuf, 10));
        strcat(pos_json, "},");
    }
    pos_json[strlen(pos_json)-1] = 0;
    strcat(pos_json, "]");

    Serial.println(pos_json);
    return 500;
}



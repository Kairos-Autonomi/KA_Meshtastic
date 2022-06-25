#include "configuration.h"
#include "TextMessagePlugin.h"
#include "NodeDB.h"
#include "PowerFSM.h"
#include "CannedMessagePlugin.h"
#include "TriggerPlugin.h"
#include "driver/uart.h"

TextMessagePlugin *textMessagePlugin;

ProcessMessage TextMessagePlugin::handleReceived(const MeshPacket &mp)
{
    bool ignore = false;
    auto &p = mp.decoded;
    DEBUG_MSG("Received text msg from=0x%0x, id=0x%x, msg=%.*s\n", mp.from, mp.id, p.payload.size, p.payload.bytes);

    char* cmd = strtok((char*)p.payload.bytes, " ");
    char* sarg1 = strtok(NULL, " ");    

    if(strcmp(cmd, "linkReq") == 0){
        triggerPlugin->linkReqd = true;
        triggerPlugin->reqd_from = mp.from;
        // strcpy((char*)mp.decoded.payload.bytes, sarg1);
        // Serial.println(sarg1);
        // if(triggerPlugin->isAttemptingLink){
        //     if(triggerPlugin->isLinked){
        //         ignore = true;
        //     }
        //     else{
        //         // char buffer [256];
        //         // char buffer2 [12];
        //         const unsigned int from = mp.from;
                
        //         // strcat(buffer, "linkConf ");
        //         // strcat(buffer, utoa(from,buffer2,10));
        //         // strcat(buffer, " ");
        //         // strcat(buffer, utoa(nodeDB.getNodeNum(),buffer2,10));
        //         // DEBUG_MSG("Sending msg=%.*s\n", buffer);
        //         cannedMessagePlugin->sendText(from, "linkConf", true);
        //     }
        // }
    }
    else if(strcmp(cmd, "linkAck") == 0){
        triggerPlugin->isLinked = true;
        triggerPlugin->linked_id = mp.from;
        cannedMessagePlugin->sendText(mp.from, "linkConf", false);
        cannedMessagePlugin->sendText(NODENUM_BROADCAST, "reqComp", false);
        // if(!triggerPlugin->isLinked){
        //     triggerPlugin->isLinked = true;
        //     triggerPlugin->linked_id = mp.from;
        //     cannedMessagePlugin->sendText(mp.from, "linkConf", true);
        // }
    }
    else if(strcmp(cmd, "linkConf") == 0){
        triggerPlugin->isLinked = true;
        triggerPlugin->linked_id = mp.from;
    }
    else if(strcmp(cmd, "reqComp") == 0){
        triggerPlugin->linkReqd = false;
    }
    else if(strcmp(cmd, "trigger")){
        const int len = strlen((char*)p.payload.bytes);
        const int txBytes = uart_write_bytes(UART_NUM_2, "trigger fire\n\r", len);
    }
    // else{
    //     char buffer[12];
    //     char* cmd = strtok((char*)p.payload.bytes, " ");
    //     char* responded_to = strtok((char*)p.payload.bytes, " ");
    //     char* response_from = strtok((char*)p.payload.bytes, " ");
    //     char* my_number = utoa(nodeDB.getNodeNum(),buffer,10);
    //     if(strcmp(cmd, "linkConf") == 0 && strcmp(responded_to, my_number) == 0){
    //         triggerPlugin->hasReceivedLinkConf = true;
    //         triggerPlugin->isLinked = true;
    //         triggerPlugin->linked_id=atoi(response_from);
    //         DEBUG_MSG("Recieved conf. Linked.");
    //     }else{
    //         DEBUG_MSG("Recieved conf. Not Linked.");
    //     }
    //}

    // We only store/display messages destined for us.
    // Keep a copy of the most recent text message.
    devicestate.rx_text_message = mp;
    devicestate.has_rx_text_message = !ignore;

    powerFSM.trigger(EVENT_RECEIVED_TEXT_MSG);
    notifyObservers(&mp);

    return ProcessMessage::CONTINUE; // Let others look at this message also if they want
}

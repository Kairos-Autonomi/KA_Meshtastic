#include "configuration.h"
#include "TextMessagePlugin.h"
#include "NodeDB.h"
#include "PowerFSM.h"
#include "CannedMessagePlugin.h"
#include "TriggerPlugin.h"

TextMessagePlugin *textMessagePlugin;

ProcessMessage TextMessagePlugin::handleReceived(const MeshPacket &mp)
{
    auto &p = mp.decoded;
    DEBUG_MSG("Received text msg from=0x%0x, id=0x%x, msg=%.*s\n", mp.from, mp.id, p.payload.size, p.payload.bytes);

    

    if(strcmp((char*)p.payload.bytes, "linkReq") == 0){
        if(triggerPlugin->isAttemptingLink){
            char buffer [256];
            char buffer2 [12];
            const unsigned int from = mp.from;
            
            strcat(buffer, "linkConf ");
            strcat(buffer, utoa(from,buffer2,10));
            strcat(buffer, " ");
            strcat(buffer, utoa(nodeDB.getNodeNum(),buffer2,10));
            DEBUG_MSG("Sending msg=%.*s\n", buffer);
            cannedMessagePlugin->sendText(from, buffer, true);
        }
    }
    else{
        char buffer[12];
        char* cmd = strtok((char*)p.payload.bytes, " ");
        char* responded_to = strtok((char*)p.payload.bytes, " ");
        char* response_from = strtok((char*)p.payload.bytes, " ");
        char* my_number = utoa(nodeDB.getNodeNum(),buffer,10);
        if(strcmp(cmd, "linkConf") == 0 && strcmp(responded_to, my_number) == 0){
            triggerPlugin->hasReceivedLinkConf = true;
            triggerPlugin->isLinked = true;
            triggerPlugin->linked_id=atoi(response_from);
            DEBUG_MSG("Recieved conf. Linked.");
        }else{
            DEBUG_MSG("Recieved conf. Not Linked.");
        }
    }

    // We only store/display messages destined for us.
    // Keep a copy of the most recent text message.
    devicestate.rx_text_message = mp;
    devicestate.has_rx_text_message = true;

    powerFSM.trigger(EVENT_RECEIVED_TEXT_MSG);
    notifyObservers(&mp);

    return ProcessMessage::CONTINUE; // Let others look at this message also if they want
}

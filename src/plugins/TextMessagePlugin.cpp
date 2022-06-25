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
    }
    else if(strcmp(cmd, "linkAck") == 0){
        triggerPlugin->isLinked = true;
        triggerPlugin->linked_id = mp.from;
        cannedMessagePlugin->sendText(mp.from, "linkConf", false);
        cannedMessagePlugin->sendText(NODENUM_BROADCAST, "reqComp", false);
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

    // We only store/display messages destined for us.
    // Keep a copy of the most recent text message.
    devicestate.rx_text_message = mp;
    devicestate.has_rx_text_message = !ignore;

    powerFSM.trigger(EVENT_RECEIVED_TEXT_MSG);
    notifyObservers(&mp);

    return ProcessMessage::CONTINUE; // Let others look at this message also if they want
}

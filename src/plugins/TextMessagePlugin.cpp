#include "TextMessagePlugin.h"
#include "CannedMessagePlugin.h"
#include "NodeDB.h"
#include "PowerFSM.h"
#include "TriggerPlugin.h"
#include "configuration.h"
#include "driver/uart.h"
#include "graphics/Screen.h"

TextMessagePlugin* textMessagePlugin;

// extern graphics::Screen *screen;

ProcessMessage TextMessagePlugin::handleReceived(const MeshPacket& mp)
{
    bool ignore = false;
    auto& p = mp.decoded;
    DEBUG_MSG("Received text msg from=0x%0x, id=0x%x, msg=%.*s\n", mp.from, mp.id, p.payload.size, p.payload.bytes);

    char* cmd = strtok((char*)p.payload.bytes, " ");
    char* sarg1 = strtok(NULL, " ");

    if (strcmp(cmd, "linkReq") == 0) {
        if (devicestate.is_linked && mp.from == devicestate.linked_id) {
            devicestate.is_linked = false;
            devicestate.linked_id = INT32_MAX;
            strcpy(devicestate.linked_name, "");
            nodeDB.saveToDisk();
        }
        else {
            triggerPlugin->linkReqd = true;
            triggerPlugin->reqd_from = mp.from;
        }
    }
    else if (strcmp(cmd, "linkAck") == 0) {
        if (devicestate.is_linked && mp.from == devicestate.linked_id) {
            devicestate.is_linked = false;
            devicestate.linked_id = INT32_MAX;
            strcpy(devicestate.linked_name, "");
            nodeDB.saveToDisk();
        }
        else {
            NodeInfo* node = nodeDB.getNode(getFrom(&mp));
            devicestate.is_linked = true;
            devicestate.linked_id = mp.from;
            strcpy(devicestate.linked_name, node->user.long_name);
            nodeDB.saveToDisk();
            // screen->goToStatusScreen();
            cannedMessagePlugin->sendText(mp.from, "linkConf", false);
            cannedMessagePlugin->sendText(NODENUM_BROADCAST, "reqComp", false);
        }
    }
    else if (strcmp(cmd, "linkConf") == 0) {
        NodeInfo* node = nodeDB.getNode(getFrom(&mp));
        devicestate.is_linked = true;
        devicestate.linked_id = mp.from;
        strcpy(devicestate.linked_name, node->user.long_name);
        nodeDB.saveToDisk();

        // screen->goToStatusScreen();
    }
    else if (strcmp(cmd, "reqComp") == 0) {
        triggerPlugin->linkReqd = false;
    }
    else if (strcmp(cmd, "disable") == 0) {
        triggerPlugin->isEnabled = false;
    }
    else if (strcmp(cmd, "enable") == 0) {
        triggerPlugin->isEnabled = true;
    }
    else if (strcmp(cmd, "arm") == 0) {
        triggerPlugin->Arm();
    }
    else if (strcmp(cmd, "disarm") == 0) {
        triggerPlugin->isArmed = false;
    }
    else if (strcmp(cmd, "fire") == 0) {
        Serial.println("got trigger");
        if(devicestate.is_linked && mp.from == devicestate.linked_id){
            triggerPlugin->TriggerServo();
        }
        else {
            ignore = true;
            Serial.println("ignored trigger");
        }
    }
    else if(strcmp(cmd, "fireoverride") == 0){
        Serial.println("is override");
        triggerPlugin->TriggerServo();

        Serial.println("acted on trigger");
    }
    else if (strcmp(cmd, "linkTerm") == 0) {
        if (mp.from == devicestate.linked_id) {
            triggerPlugin->linkReqd = false;
            devicestate.is_linked = false;
            devicestate.linked_id = INT32_MAX;
            strcpy(devicestate.linked_name, "");
            nodeDB.saveToDisk();
            triggerPlugin->SendLinkTerm();
        }
    }
    else {
        Serial.println("unknown Text:");
        Serial.println(cmd);
    }

    // We only store/display messages destined for us.
    // Keep a copy of the most recent text message.
    if (!ignore) {
        Serial.println("Not ignored!");
        devicestate.rx_text_message = mp;
        devicestate.has_rx_text_message = !ignore;

        powerFSM.trigger(EVENT_RECEIVED_TEXT_MSG);
        notifyObservers(&mp);
    }

    return ProcessMessage::CONTINUE; // Let others look at this message also if they want
}

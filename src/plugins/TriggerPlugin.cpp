#include "configuration.h"
#include "TriggerPlugin.h"
#include "NodeDB.h"
#include "PowerFSM.h"
#include "CannedMessagePlugin.h"

TriggerPlugin* triggerPlugin;

TriggerPlugin::TriggerPlugin(){

}

void TriggerPlugin::AttemptLink()
{
    isAttemptingLink = true;
    for (int i = 0; i < 20; i++) {
        cannedMessagePlugin->sendText(NODENUM_BROADCAST, "linkReq", true);
        delay(800);
        // if(hasReceivedLinkConf){
        //     break;
        // }
    }
}

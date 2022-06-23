#include "configuration.h"
#include "TextMessageModule.h"
#include "NodeDB.h"
#include "PowerFSM.h"
#include "graphics/Screen.h"
#include "driver/uart.h"

TextMessageModule *textMessageModule;

ProcessMessage TextMessageModule::handleReceived(const MeshPacket &mp)
{
    auto &p = mp.decoded;
    DEBUG_MSG("Received text msg from=0x%0x, id=0x%x, msg=%.*s\n", mp.from, mp.id, p.payload.size, p.payload.bytes);

    if(strcmp((char*)p.payload.bytes, "hi") == 0 ){
        strcpy((char*)mp.decoded.payload.bytes, "hi intercepted");
    }
    else if(strcmp((char*)p.payload.bytes, "trigger fire") == 0){

        const int len = strlen((char*)p.payload.bytes);
        const int txBytes = uart_write_bytes(UART_NUM_2, (char*)p.payload.bytes, len);
    }

    

    // We only store/display messages destined for us.
    // Keep a copy of the most recent text message.
    devicestate.rx_text_message = mp;
    devicestate.has_rx_text_message = true;

    powerFSM.trigger(EVENT_RECEIVED_TEXT_MSG);
    notifyObservers(&mp);

    return ProcessMessage::CONTINUE; // Let others look at this message also if they want
}

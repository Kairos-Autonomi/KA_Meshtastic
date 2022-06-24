#pragma once

/**
 * Text message handling for meshtastic - draws on the OLED display the most recent received message
 */
class TriggerPlugin
{
  public:
    bool isAttemptingLink = false;
    bool isLinked = false;
    bool hasReceivedLinkConf = false;
    unsigned int linked_id = __UINT32_MAX__;

    /** Constructor
     * name is for debugging output
     */
    TriggerPlugin();
    void AttemptLink();
};

extern TriggerPlugin *triggerPlugin;

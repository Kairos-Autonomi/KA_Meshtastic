#pragma once
#include "ESP32Servo/src/ESP32Servo.h"

/**
 * Text message handling for meshtastic - draws on the OLED display the most recent received message
 */
class TriggerPlugin:private concurrency::OSThread
{
  public:
    bool isAttemptingLink = false;
    bool hasReceivedLinkConf = false;

    bool isEnabled = false;
    bool isArmed = false;

    unsigned int reqd_from = __UINT32_MAX__;
    bool linkReqd = false;

    /** Constructor
     * name is for debugging output
     */
    TriggerPlugin();
    void AttemptLink();
    void SendTrigger();
    void SendFullTriggerSequence();
    void SendLinkTerm();
    void Arm();

    void TriggerServo();
    void TriggerRelay();
    void TriggerSerial();

  protected:
    bool lastArmedState = false;
    Servo triggerServo;
    int32_t timeAtLastReq = 0;
    int num_reqs_sent = 0;
    int timeAtArming = 0;

    /**
     * Periodically check to see if we are attempting link and sends linksreq.
     * 
     * The method that will be called each time our thread gets a chance to run
     *
     * Returns desired period for next invocation (or RUN_SAME for no change)
     */
    virtual int32_t runOnce() override;
};

extern TriggerPlugin *triggerPlugin;

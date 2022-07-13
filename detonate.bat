meshtastic --port %1 --dest %2 --sendtext enable
timeout /t 1
meshtastic --port %1 --dest %2 --sendtext arm
timeout /t 1
meshtastic --port %1 --dest %2 --sendtext fireoverride
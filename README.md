# ESP8266_Xiaomi_YI_WIFI_RemotePWM
Software for ESP8266 to trigger by wifi Xiaomi Yi action camera.

This program was made to trigger original Xiaomi Yi action camera via wifi according to a PWM signal (from a drone flight controller or RC receiver for example), regarding the PWM pulse, it can take a photo or start/stop to record a video.

To ensure that the received PWM wasn't made by soft/hardware problem, it must last small time (80ms) before to request camera trigger.
The code test the answer form the camera, if the request is denied, the code ask again.
Start the camera first and turn on wifi, when ready, start the ESP8266 board or reset it.

This program was made for the original Xiaomi Yi action camera, which don't have display on the back and is white or blue/green, I don't ensure any compatibility with newer Yi camera.

This code is open source, use and modify as you want!
This is a modification of code found via a youtube video (original code trigger via switch).

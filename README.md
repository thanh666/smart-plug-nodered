# Smart socket

Firmware for a smart AC socket control through the Internet. The plug can collect the power usage and set the time to turn it off.

## Technology used

- **Programming language:** C++
- **Hardware:** ESP32, ACS712 sensor, normal AC socket, 5V relay

## Detail

Using an MQTT broker to transfer the collected data from the socket to the AWS AC2 server.
The ESP32 board is create an MQTT connection and listens to the specific setup Topic.
When the MQTT topic is published data (on/off signal, set timer signal) the ESP32 will process it by the logic embedded in the ESP32. If the socket is ON, ESP32 will send the current usage of the socket to the MQTT broker 12 times/minute.  

# This is a Wemos D1 Mini sketch that reads DS18D20 and uploads realtime energy consumption to a MQTT server
 
1) Assemble hardware according to https://create.arduino.cc/projecthub/TheGadgetBoy/ds18b20-digital-temperature-sensor-and-arduino-9cc806
2) Copy config.h.sample to config.h
3) Edit your newly created config.h to add your wifi, MQTT, input pin and sleep time info (in minutes)
4) Upload to your board and wait for the information to be published every sleep_time

Obs: The board goes to deep sleep between runs.


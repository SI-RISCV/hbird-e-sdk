# README #


* This demo is to show how to use the WIZnet W5100 Ethernet device for Hbird Development
* Fuction:  The W5100 module will actively connect to the server and send the data it resceives.

W5100  is client by default                local IP : 192.168.0.177                  
W5100  connects  remote  server by default  . Server IP : 192.168.0.100 Port : 5000

Pin correspondence:
HBird-FPGA    Arduino Ethernet Module(W5100) 
  
mcu_gpio29    sck(13,ICSP)
mcu_gpio28    MOSI(12,ICSP)
mcu_gpio27    MISO(11,ICSP)
mcu_gpio26    SS(10)
mcu_gpio25    reset 

VCC3V3        3V3
GND           GND


Please check the 13,12,11 pins on the expansion board first.If a problem occurs, the SPI pin of the ICSP interface can be used instead.


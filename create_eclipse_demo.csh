make software_clean       PROGRAM=demo_gpio
make software_clean       PROGRAM=dhrystone
make software_clean       PROGRAM=hello_world
rm eclipse_demo -rf
mkdir eclipse_demo
mkdir eclipse_demo/software
cp bsp eclipse_demo -rf
rm eclipse_demo/bsp/nuclei-n200 -rf
cp software/dhrystone eclipse_demo/software -rf
cp software/demo_gpio eclipse_demo/software -rf
cp software/hello_world eclipse_demo/software -rf
rm eclipse_demo/bsp/hbird-e200/env/openocd_ftdi.cfg -rf
tar -czvf eclipse_demo.tar.gz eclipse_demo
rm eclipse_demo -rf

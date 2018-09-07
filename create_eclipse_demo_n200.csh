make software_clean       PROGRAM=demo_pic
make software_clean       PROGRAM=dhrystone
make software_clean       PROGRAM=hello_world
rm n200_eclipse_demo -rf
mkdir n200_eclipse_demo
mkdir n200_eclipse_demo/software
cp bsp n200_eclipse_demo -rf
rm n200_eclipse_demo/bsp/hbird-e200 -rf
cp software/dhrystone n200_eclipse_demo/software -rf
cp software/demo_pic n200_eclipse_demo/software -rf
cp software/hello_world n200_eclipse_demo/software -rf
tar -czvf n200_eclipse_demo.tar.gz n200_eclipse_demo
rm n200_eclipse_demo -rf

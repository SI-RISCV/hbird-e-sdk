make dasm      PROGRAM=demo_gpio4sim BOARD=hbird-e200 CORE=e203   DOWNLOAD=itcm
make dasm      PROGRAM=dhrystone4sim BOARD=hbird-e200 CORE=e203   DOWNLOAD=itcm
make dasm      PROGRAM=dhrystone4sim_best BOARD=hbird-e200 CORE=e203   DOWNLOAD=itcm
make dasm      PROGRAM=coremark4sim  BOARD=hbird-e200 CORE=e203   DOWNLOAD=itcm
rm fpga_test4sim -rf
mkdir fpga_test4sim
cp software/demo_gpio4sim fpga_test4sim/ -rf
cp software/dhrystone4sim fpga_test4sim/ -rf
cp software/dhrystone4sim_best fpga_test4sim/ -rf
cp software/coremark4sim  fpga_test4sim/ -rf

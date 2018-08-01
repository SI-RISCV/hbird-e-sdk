
# May need to hack dhrystone/dhry_1.c number of runs from 16000 to 2, and hack the too small times
# May need to hack coremark/core_main.c number of runs and hack the too small times
make dasm      PROGRAM=demo_gpio4sim BOARD=hbird-e200 CORE=e203 OCDCFG=ftdi  DOWNLOAD=iecm
make dasm      PROGRAM=dhrystone4sim BOARD=hbird-e200 CORE=e203 OCDCFG=ftdi  DOWNLOAD=iecm
make dasm      PROGRAM=dhrystone4sim_best BOARD=hbird-e200 CORE=e203 OCDCFG=ftdi  DOWNLOAD=iecm
make dasm      PROGRAM=coremark4sim  BOARD=hbird-e200 CORE=e203 OCDCFG=ftdi  DOWNLOAD=iecm
rm fpga_test4sim -rf
mkdir fpga_test4sim
cp software/demo_gpio4sim fpga_test4sim/ -rf
cp software/dhrystone4sim fpga_test4sim/ -rf
cp software/dhrystone4sim_best fpga_test4sim/ -rf
cp software/coremark4sim  fpga_test4sim/ -rf

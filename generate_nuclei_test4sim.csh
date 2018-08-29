
# May need to hack dhrystone/dhry_1.c number of runs from 16000 to 2, and hack the too small times
# May need to hack coremark/core_main.c number of runs and hack the too small times
make dasm      PROGRAM=demo_pic4sim BOARD=nuclei-n200 CORE=n203   DOWNLOAD=iecm
make dasm      PROGRAM=dhrystone4sim BOARD=nuclei-n200 CORE=n203   DOWNLOAD=iecm
make dasm      PROGRAM=dhrystone4sim_best BOARD=nuclei-n200 CORE=n203   DOWNLOAD=iecm
make dasm      PROGRAM=coremark4sim  BOARD=nuclei-n200 CORE=n203   DOWNLOAD=iecm
rm fpga_nuclei_test4sim -rf
mkdir fpga_nuclei_test4sim
cp software/demo_pic4sim fpga_nuclei_test4sim/ -rf
cp software/dhrystone4sim fpga_nuclei_test4sim/ -rf
cp software/dhrystone4sim_best fpga_nuclei_test4sim/ -rf
cp software/coremark4sim  fpga_nuclei_test4sim/ -rf

cd ./lib
make clean
make 
cd ../bootloader
make clean
make 
make image
cd ../os
make clean
make
echo done
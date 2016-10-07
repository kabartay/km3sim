How To Install
==============

Install Geant4.10 in a directory of your choice.

Run cmake in a separate build dir (recommended):
    
    cd PATH/TO/KM3SIM
    mkdir build
    cd build
    cmake -DGeant4_DIR=/PATH/TO/GEANT/INSTALL/lib64/Geant4-10.2.2 ../
    make

    km3sim --help

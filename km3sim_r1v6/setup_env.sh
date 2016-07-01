export G4DIR=/state/partition1/geant4.9.6.p02
export G4INSTALL=$G4DIR/install
export G4DATADIR=$G4INSTALL/share/Geant4-10.2.0/data
export G4NEUTRONHPDATA=$G4DATADIR/G4NDL4.5
export G4LEDATA=$G4DATADIR/G4EMLOW6.48
export G4LEVELGAMMADATA=$G4DATADIR/PhotonEvaporation3.2
export G4RADIOACTIVEDATA=$G4DATADIR/RadioactiveDecay4.3
export G4NEUTRONXSDATA=$G4DATADIR/G4NEUTRONXS1.4
export G4PIIDATA=$G4DATADIR/G4PII1.3
export G4REALSURFACEDATA=$G4DATADIR/RealSurface1.0
export G4SAIDXSDATA=$G4DATADIR/G4SAIDDATA1.1
export G4ABLADATA=$G4DATADIR/G4ABLA3.0
export G4ENSDFSTATEDATA=$G4DATADIR/G4ENSDFSTATE1.2

export LD_LIBRARY_PATH=$G4INSTALL/lib64:$LD_LIBRARY_PATH
export PATH=$G4INSTALL/bin:$PATH

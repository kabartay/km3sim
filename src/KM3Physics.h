#ifndef KM3Physics_h
#define KM3Physics_h 1

#include "G4VUserPhysicsList.hh"
#include "G4ParticleTable.hh"
#include "G4NuclideTable.hh"
#include "globals.hh"
#include "KM3Detector.h"
#include "KM3Cherenkov.h"
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>

class KM3Physics : public G4VUserPhysicsList {
 public:
  KM3Physics();
  ~KM3Physics();

 protected:
  // Construct particle and physics
  void ConstructParticle();
  void ConstructProcess();
  void SetCuts();

 protected:
  // these methods Construct physics processes and register them
  //void AddParameterisation();
  void ConstructGeneral();
  void ConstructEM();
  void ConstructOP();

 protected:
  void ConstructHA();

 public:
  KM3Detector *aDetector;
  KM3Cherenkov *theCerenkovProcess;

 protected:
  G4double defaultCutEnergyValueForGamma;
  G4double defaultCutEnergyValueForElectron;
  G4double defaultCutEnergyValueForMuon;
  G4double defaultCutEnergyValueForHadron;

#ifndef theParticleIterator
  bool ownIterator = True;
  G4ParticleTable::G4PTblDicIterator* theParticleIterator;
#endif
  
};

#endif

#ifndef KM3Detector_h
#define KM3Detector_h 1

#include <stdio.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>

#include "KM3Definitions.h"
#include "KM3Cathods.h"
#include "KM3PrimaryGeneratorAction.h"
#include "KM3EvtIO.h"

#include "G4Material.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VSensitiveDetector.hh"
#include <CLHEP/Units/SystemOfUnits.h>
// newgeant #include "Saxana/SAXProcessor.h"
// newgeant #include "Saxana/ProcessingConfigurator.h"


class G4VPhysicalVolume;

class KM3Detector : public G4VUserDetectorConstruction {
 public:
  KM3Detector();
  ~KM3Detector();

 public:
  FILE *outfile;
  KM3EvtIO *TheEVTtoWrite;

  G4VPhysicalVolume *Construct();
  G4double Quantum_Efficiency;
  G4double bottomPosition;
  G4ThreeVector detectorCenter;
  G4double lowestStorey;
  G4double highestStorey;
  G4double outerStorey;
  G4double detectorRadius;

  G4VPhysicalVolume *ConstructWorldVolume(const std::string &detxFile);
  G4VPhysicalVolume *SetSensitiveCathods(G4VPhysicalVolume *pvol, G4VSensitiveDetector *sd);

  // this is the maximum vertical distance of the storeys
  // from the center plus a number of absorpion lengths
  G4double detectorMaxz;

  // this is the maximum orizontal distance of the
  // storeys plus a number of absorption lengths
  G4double detectorMaxRho;

  KM3Cathods *allCathods;
  G4double MaxAbsDist;
  G4bool vrmlhits;
  G4bool DrawDetector;
  //char *Geometry_File;
  //char *Parameter_File;
  std::string Geometry_File;
  std::string Parameter_File;
  G4double TotCathodArea;
  KM3PrimaryGeneratorAction *MyGenerator;

 private:
  void FindDetectorRadius(void);
  void ConstructMaterials(void);
  G4int TotalPMTEntities(const G4VPhysicalVolume *);
  void SetUpVariables(void);
  // newgeant  void sxpInitialize(void);

  std::vector<StoreysPositions *> *allStoreys;
  std::vector<OMPositions *> *allOMs;
  std::vector<TowersPositions *> *allTowers;  // new towers

 private:
  // newgeant  SAXProcessor sxp;
  // newgeant ProcessingConfigurator config;
  G4VPhysicalVolume *fWorld;
  G4Material *Water;
  G4Material *Crust;
  G4Material *Cathod;

 private:
  G4double detectorDepth;
  G4int NUMENTRIES;
  G4int NUMENTRIES_ANGLEACC;
  G4double PPCKOV[100];
  G4double RINDEX_WATER[100];
  G4double Water_Transparency;
  G4double ABSORPTION_WATER[100];
  G4double ABSORPTION_GLASS[100];
  G4double RINDEX_GLASS[100];
  G4double ABSORPTION_GELL[100];
  G4double RINDEX_GELL[100];
  G4double ABSORPTION_AIR[100];
  G4double RINDEX_AIR[100];
  G4double ABSORPTION_CATH[100];
  G4double RINDEX_CATH[100];
  G4double Q_EFF[100];
  G4double SCATTER_WATER[100];
  G4double MieModel;
  G4double COSANGLES[100];
  G4double ACCEPTANCE[100];

  int global_det_id_;
  int n_doms_;
  int numCathods;

  std::vector<double> z_all;
  std::vector<double> r_all;
};
#endif  // KM3Detector_h

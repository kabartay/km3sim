#ifndef KM3SD_h
#define KM3SD_h 1

#include "G4VSensitiveDetector.hh"
#include "KM3Hit.h"
#include <stdio.h>
#include <vector>
#include "KM3Detector.h"
#include "Randomize.hh"
#include "G4MaterialPropertiesTable.hh"
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>

class G4Step;
class G4HCofThisEvent;

class KM3SD : public G4VSensitiveDetector {
 public:
  FILE *outfile;
  KM3SD(G4String);
  ~KM3SD();

  void Initialize(G4HCofThisEvent *);
  G4bool ProcessHits(G4Step *, G4TouchableHistory *);
  void EndOfEvent(G4HCofThisEvent *);
  KM3Detector *myStDetector;
  // short  void InsertExternalHit(G4int ic,G4double time,G4int
  // originalInfo,G4int angleDirection,G4int angleIncident);
  void InsertExternalHit(G4int ic, const G4ThreeVector &OMPosition,
                         G4double time, G4int originalInfo,
                         const G4ThreeVector &photonDirection);

  void PrintAll();
  void clear();

 private:
  KM3HitsCollection *HitsCollection;
  G4int ProcessHitsCollection(KM3HitsCollection *aCollection);
  G4double TResidual(G4double, const G4ThreeVector &, const G4ThreeVector &,
                     const G4ThreeVector &);
  void QuickSort(G4int shorttype, std::vector<KM3Hit *> *theCollectionVector,
                 G4int top, G4int bottom);
  G4int partition_CathodId(std::vector<KM3Hit *> *theCollectionVector,
                           G4int top, G4int bottom);
  G4int partition_Time(std::vector<KM3Hit *> *theCollectionVector, G4int top,
                       G4int bottom);
  G4bool AcceptAngle(G4double cosangle, G4double CathodRadius,
                     G4double CathodHeight, bool);
  void MergeHits(G4int nfirst, G4int nlast, G4double MergeWindow);
  G4double thespeedmaxQE;
};

#endif

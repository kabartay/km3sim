#include "KM3StackingAction.h"
#include "G4ios.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4Track.hh"
#include "G4UnitsTable.hh"
#include "G4VProcess.hh"
#include <math.h>
#include "G4StackManager.hh"
// the following was added to see what initial hadrons can give muons
//#include "KM3TrackInformation.h"
//#ifdef G4MYHAMUONS_PARAMETERIZATION
#include "G4RunManager.hh"
#include "KM3PrimaryGeneratorAction.h"
//#endif

using CLHEP::keV;
using CLHEP::GeV;
using CLHEP::TeV;
using CLHEP::ns;
using CLHEP::m;

KM3StackingAction::KM3StackingAction() { ; }

KM3StackingAction::~KM3StackingAction() { ; }

G4ClassificationOfNewTrack KM3StackingAction::ClassifyNewTrack(
    const G4Track *aTrack) {
  static KM3PrimaryGeneratorAction *aGeneAction =
      (KM3PrimaryGeneratorAction *)(G4RunManager::GetRunManager()
                                        ->GetUserPrimaryGeneratorAction());
  G4double kineticEnergy;
  G4ThreeVector x0;
  G4ThreeVector p0;
  G4ThreeVector distanceV;
  G4double direction, distanceRho2;
  static G4double detectorMaxRho2 =
      MyStDetector->detectorMaxRho * MyStDetector->detectorMaxRho;

  // here kill tracks that have already killed by other classes
  if ((aTrack->GetTrackStatus() == fStopAndKill) ||
      (aTrack->GetTrackStatus() == fKillTrackAndSecondaries))
    return fKill;

  // first check that is not a photon to save time
  if (aTrack->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()) {
    // ok, this is reached as expected. moving on.
    //std::cout << "not a photon!" << std::endl;
    kineticEnergy = aTrack->GetKineticEnergy();

    // threshold for electron cerenkov production (not applicable for
    // positron due to anihhilation
    if ((aTrack->GetDefinition()->GetParticleName() == "e-") &&
        (kineticEnergy < 240 * keV))
      return fKill;
    // threshold for gamma potentially absorbed by an atomic electron
    if ((aTrack->GetDefinition()->GetParticleName() == "gamma") &&
        (kineticEnergy < 240 * keV))
      return fKill;
    // kill produced neutrinos
    if ((aTrack->GetDefinition()->GetParticleType() == "lepton") &&
        (aTrack->GetDefinition()->GetPDGMass() == 0.0))
      return fKill;

    //std::cout << aTrack->GetDefinition()->GetParticleType() << std::endl;

    x0 = aTrack->GetPosition();
    distanceV = x0 - MyStDetector->detectorCenter;
    distanceRho2 = distanceV[0] * distanceV[0] + distanceV[1] * distanceV[1];
    //std::cout << "dist v, center: " << distanceV << distanceRho2 << std::endl;

    // if the particle is not muon and is created outside the can kill it
    if (aTrack->GetDefinition() != G4MuonPlus::MuonPlusDefinition() &&
        aTrack->GetDefinition() != G4MuonMinus::MuonMinusDefinition()) {
      if ((x0[2] < MyStDetector->bottomPosition) ||
          (distanceRho2 > detectorMaxRho2) ||
          (x0[2] > MyStDetector->detectorMaxz)){
        return fKill;
        std::cout << "not a muon + outside, kill it" << std::endl;
      }
      return fUrgent;
    } else {
      // if it is a muon kill it only if is not going to cross the can
      p0 = aTrack->GetMomentumDirection();
      if ((x0[2] < MyStDetector->bottomPosition) && (p0[2] < 0)) {
        // goes down while below the can
        std::cout << "muon, goes down while below -> kill it" << std::endl;
        return fKill;
      }
      if ((x0[2] > MyStDetector->detectorMaxz) && (p0[2] > 0)) {
        // goes up while above the can
        std::cout << "muon, goes up while above -> kill it" << std::endl;
        return fKill;
      }
      direction = p0[0] * distanceV[0] + p0[1] * distanceV[1];
      if ((distanceRho2 > detectorMaxRho2) && (direction > 0)){
        // goes away while outside the can
        std::cout << "muon, goes away while outside -> kill it" << std::endl;
        return fKill;
      }

      // first check if it is inside the can
      G4double rxy2 = x0[0] * x0[0] + x0[1] * x0[1];
      if ((rxy2 < detectorMaxRho2) && (x0[2] > MyStDetector->bottomPosition) &&
          (x0[2] < MyStDetector->detectorMaxz))
        return fUrgent;
      G4double Ttop = (MyStDetector->detectorMaxz - x0[2]) / p0[2];
      if (Ttop > 0) {
        G4double Xtop = x0[0] + Ttop * p0[0] - MyStDetector->detectorCenter[0];
        G4double Ytop = x0[1] + Ttop * p0[1] - MyStDetector->detectorCenter[1];
        G4double dRhoTop = Xtop * Xtop + Ytop * Ytop;
        if (dRhoTop < detectorMaxRho2) return fUrgent;
      }
      G4double Tbottom = (MyStDetector->bottomPosition - x0[2]) / p0[2];
      if (Tbottom > 0) {
        G4double Xbottom =
            x0[0] + Tbottom * p0[0] - MyStDetector->detectorCenter[0];
        G4double Ybottom =
            x0[1] + Tbottom * p0[1] - MyStDetector->detectorCenter[1];
        G4double dRhoBottom = Xbottom * Xbottom + Ybottom * Ybottom;
        if (dRhoBottom < detectorMaxRho2) return fUrgent;
      }
      G4double a = p0[0] * p0[0] + p0[1] * p0[1];
      G4double b = x0[0] * p0[0] + x0[1] * p0[1];
      G4double c = x0[0] * x0[0] + x0[1] * x0[1] - detectorMaxRho2;
      G4double dia = b * b - a * c;
      if (dia > 0) {
        dia = sqrt(dia);
        G4double SideDist1 = (-b - dia) / a;
        G4double sidez1 = x0[2] + SideDist1 * p0[2];
        if ((sidez1 > MyStDetector->bottomPosition) &&
            (sidez1 < MyStDetector->detectorMaxz) && (SideDist1 > 0))
          return fUrgent;
        G4double SideDist2 = (-b + dia) / a;
        G4double sidez2 = x0[2] + SideDist2 * p0[2];
        if ((sidez2 > MyStDetector->bottomPosition) &&
            (sidez2 < MyStDetector->detectorMaxz) && (SideDist2 > 0))
          return fUrgent;
      }
      return fKill;
    }
  }
  return fUrgent;
}

void KM3StackingAction::NewStage() {}

void KM3StackingAction::PrepareNewEvent() {}

void KM3StackingAction::SetDetector(KM3Detector *adet) { MyStDetector = adet; }

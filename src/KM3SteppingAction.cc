#include "KM3SteppingAction.h"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4Step.hh"

using CLHEP::TeV;
using CLHEP::GeV;
using CLHEP::meter;
using CLHEP::m;

KM3SteppingAction::KM3SteppingAction() {
  P7[0] = 0.14709;
  P7[1] = 2.2948;
  P7[2] = -1.4446;
  P7[3] = 0.82128;
  P7[4] = -0.25202;
  P7[5] = 0.40399e-01;
  P7[6] = -0.32452e-02;
  P7[7] = 0.10360e-03;
  P1LOW[0] = 0.63416;
  P1LOW[1] = 0.97777;
  P1HIGH[0] = 3.9298;
  P1HIGH[1] = 0.85567e-01;
}

void KM3SteppingAction::UserSteppingAction(const G4Step *aStep) {
  G4ThreeVector x0;
  G4ThreeVector p0;
  G4double distanceRho, direction;

  if (aStep->GetTrack()->GetParentID() == 0) {  // only the primary particle
    if (aStep->GetTrack()->GetDefinition() ==
        G4MuonPlus::MuonPlusDefinition() ||
        aStep->GetTrack()->GetDefinition() ==
        G4MuonMinus::MuonMinusDefinition()) {
      p0 = aStep->GetTrack()->GetMomentumDirection();
      x0 = aStep->GetTrack()->GetPosition();

      // here we keep the energy of the muon every 10 meters (approximately)
      // only if the first primary is a muon
      if (aStep->GetTrack()->GetTrackID() == 1) {
        G4ThreeVector StartPosition = aStep->GetTrack()->GetVertexPosition();
        G4ThreeVector StartDirection =
            aStep->GetTrack()->GetVertexMomentumDirection();
        G4double DistanceCovered = (x0 - StartPosition).dot(StartDirection);
        G4int iDist = int(DistanceCovered / (10.0 * m));
        G4int iSlots = event_action->EnergyAtPosition.size();
        if (iDist == iSlots)
          event_action->EnergyAtPosition.push_back(
              aStep->GetTrack()->GetKineticEnergy());
        else if (iDist > iSlots) {
          G4double LastEnergy = event_action->EnergyAtPosition[iSlots - 1];
          for (int ir = 0; ir < iDist - iSlots; ir++)
            event_action->EnergyAtPosition.push_back(LastEnergy);
          event_action->EnergyAtPosition.push_back(
              aStep->GetTrack()->GetKineticEnergy());
        }
      }

      // here report the position of the muon when it stops
      if (aStep->GetTrack()->GetKineticEnergy() == 0.0 &&
          aStep->GetTrack()->GetTrackStatus() == fStopButAlive) {
        G4int MuonSlot = event_action->GetSlot(aStep->GetTrack()->GetTrackID());
        if (MuonSlot >= 0) {
          event_action->stopPosition[MuonSlot] = x0;
          event_action->stopTime[MuonSlot] = aStep->GetTrack()->GetGlobalTime();
        }
      }

      // here kill muons that have not enough energy to reach the can
      // first check if it is inside the can
      G4double RRR2 =
          myStDetector->detectorMaxRho * myStDetector->detectorMaxRho;
      G4cout << "Detector Radius: " <<RRR2 << G4endl;

      G4double rxy2 = x0[0] * x0[0] + x0[1] * x0[1];
      if ((rxy2 > RRR2) || (x0[2] < myStDetector->bottomPosition) ||
          (x0[2] >
           myStDetector->detectorMaxz)) {  // if it is not inside the can
        G4cout << "Not inside Can!!!: " <<RRR2 << G4endl;
        G4double Tbottom = (myStDetector->bottomPosition - x0[2]) / p0[2];
        G4double Xbottom =
            x0[0] + Tbottom * p0[0] - myStDetector->detectorCenter[0];
        G4double Ybottom =
            x0[1] + Tbottom * p0[1] - myStDetector->detectorCenter[1];
        G4double dRhoBottom = Xbottom * Xbottom + Ybottom * Ybottom;
        if ((dRhoBottom > RRR2) || (Tbottom < 0)) Tbottom = 1.0e21;

        G4double Ttop = (myStDetector->detectorMaxz - x0[2]) / p0[2];
        G4double Xtop = x0[0] + Ttop * p0[0] - myStDetector->detectorCenter[0];
        G4double Ytop = x0[1] + Ttop * p0[1] - myStDetector->detectorCenter[1];
        G4double dRhoTop = Xtop * Xtop + Ytop * Ytop;
        if ((dRhoTop > RRR2) || (Ttop < 0)) Ttop = 1.0e21;

        G4double a = p0[0] * p0[0] + p0[1] * p0[1];
        G4double b = x0[0] * p0[0] + x0[1] * p0[1];
        G4double c = x0[0] * x0[0] + x0[1] * x0[1] - RRR2;
        G4double dia = b * b - a * c;
        G4double SideDistIn = 1.0e21;
        if (dia > 0) {
          dia = sqrt(dia);
          G4double SideDistIn1 = (-b - dia) / a;
          G4double SideDistIn2 = (-b + dia) / a;
          G4double sidezIn1 = x0[2] + SideDistIn1 * p0[2];
          G4double sidezIn2 = x0[2] + SideDistIn2 * p0[2];
          if (SideDistIn1 > 0 && sidezIn1 > myStDetector->bottomPosition &&
              sidezIn1 < myStDetector->detectorMaxz)
            SideDistIn = SideDistIn1;
          if (SideDistIn2 > 0 && sidezIn2 > myStDetector->bottomPosition &&
              sidezIn2 < myStDetector->detectorMaxz && SideDistIn2 < SideDistIn)
            SideDistIn = SideDistIn2;
        }
        G4double TMin = 1.0e30;
        if (TMin > Tbottom) TMin = Tbottom;
        if (TMin > Ttop) TMin = Ttop;
        if (TMin > SideDistIn) TMin = SideDistIn;

        if (TMin < 1.0e20) {
          if (TMin > MuonRange(aStep->GetTrack()->GetKineticEnergy())) {
            aStep->GetTrack()->SetTrackStatus(fStopAndKill);
            return;
          }
        } else {
          aStep->GetTrack()->SetTrackStatus(fStopAndKill);
          return;
        }
      }  // if not inside detector

      // new here we report the points when the muon goes in the
      // detector (smaller can by 50 meters on the bottom and 150m on
      // top and sides)
      // first be sure that this muon is registered in Event Action
      G4int MuonSlot = event_action->GetSlot(aStep->GetTrack()->GetTrackID());
      if (MuonSlot >= 0) {
        G4double SideRadiusSmallCan = myStDetector->detectorMaxRho - 150.0 * m;
        G4double TopPosSmallCan = myStDetector->detectorMaxz - 150.0 * m;
        G4double BottomPosSmallCan = myStDetector->bottomPosition + 50.0 * m;
        RRR2 = SideRadiusSmallCan * SideRadiusSmallCan;

        G4double DistToTop = (TopPosSmallCan - x0[2]) / p0[2];
        G4double DistToBottom = (BottomPosSmallCan - x0[2]) / p0[2];
        G4ThreeVector PointOnTop = x0 + DistToTop * p0;
        G4ThreeVector PointOnBottom = x0 + DistToBottom * p0;
        G4double HorDistTop2 =
            PointOnTop[0] * PointOnTop[0] + PointOnTop[1] * PointOnTop[1];
        G4double HorDistBottom2 = PointOnBottom[0] * PointOnBottom[0] +
            PointOnBottom[1] * PointOnBottom[1];

        G4bool conti = true;
        if ((HorDistTop2 > RRR2) || (HorDistBottom2 > RRR2)) {
          G4double a = p0[0] * p0[0] + p0[1] * p0[1];
          G4double b = x0[0] * p0[0] + x0[1] * p0[1];
          G4double c = x0[0] * x0[0] + x0[1] * x0[1] - RRR2;
          G4double dia = b * b - a * c;
          if (dia < 0) {
            conti = false;
          } else {
            dia = sqrt(dia);
            G4double SideDist1 = (-b - dia) / a;
            G4double SideDist2 = (-b + dia) / a;
            G4double sidez1 = x0[2] + SideDist1 * p0[2];
            G4double sidez2 = x0[2] + SideDist2 * p0[2];
            if (((sidez1 < PointOnBottom[2]) && (sidez2 < PointOnBottom[2])) ||
                ((sidez1 > PointOnTop[2]) && (sidez2 > PointOnTop[2]))) {
              conti = false;
            } else if (((sidez1 > PointOnBottom[2]) &&
                        (sidez2 > PointOnBottom[2])) &&
                       ((sidez1 < PointOnTop[2]) && (sidez2 < PointOnTop[2]))) {
              if (sidez1 < sidez2) {
                DistToTop = SideDist2;
                PointOnTop = x0 + DistToTop * p0;
                DistToBottom = SideDist1;
                PointOnBottom = x0 + DistToBottom * p0;
              } else {
                DistToTop = SideDist1;
                PointOnTop = x0 + DistToTop * p0;
                DistToBottom = SideDist2;
                PointOnBottom = x0 + DistToBottom * p0;
              }
            } else if ((PointOnBottom[2] < sidez1) &&
                       (sidez1 < PointOnTop[2])) {
              if (HorDistTop2 > RRR2) {
                DistToTop = SideDist1;
                PointOnTop = x0 + DistToTop * p0;
              } else {
                DistToBottom = SideDist1;
                PointOnBottom = x0 + DistToBottom * p0;
              }

            } else {
              if (HorDistTop2 > RRR2) {
                DistToTop = SideDist2;
                PointOnTop = x0 + DistToTop * p0;
              } else {
                DistToBottom = SideDist2;
                PointOnBottom = x0 + DistToBottom * p0;
              }
            }
          }
        }

        if (conti) {
          G4ThreeVector PointOnMiddle = 0.5 * (PointOnTop + PointOnBottom);
          // before G4ThreeVector PointOnMiddle = PointOnTop -
          // (PointOnTop.dot(PointOnBottom-PointOnTop)/(PointOnBottom-PointOnTop).mag2())*(PointOnBottom-PointOnTop);
          G4double DistToEnter, DistToCenter, DistToLeave;
          DistToCenter = (PointOnMiddle[2] - x0[2]) / p0[2];
          if (p0[2] < 0) {
            DistToEnter = DistToTop;
            DistToLeave = DistToBottom;
          } else {
            DistToEnter = DistToBottom;
            DistToLeave = DistToTop;
          }

          if ((DistToCenter < -20.0 * m) && (DistToCenter > -30.0 * m)) {
            event_action->centerPost[MuonSlot] = x0;
          }
          if ((DistToCenter < 30.0 * m) && (DistToCenter > 20.0 * m)) {
            event_action->centerPre[MuonSlot] = x0;
          }
          if ((DistToCenter < 5.0 * m) && (DistToCenter > -5.0 * m)) {
            event_action->centerMomentum[MuonSlot] =
                aStep->GetTrack()->GetMomentum().mag();
            event_action->centerPosition[MuonSlot] = x0;
            event_action->centerTime[MuonSlot] =
                aStep->GetTrack()->GetGlobalTime();
          }

          if ((DistToEnter < -20.0 * m) && (DistToEnter > -30.0 * m)) {
            event_action->enterPost[MuonSlot] = x0;
          }
          if ((DistToEnter < 30.0 * m) && (DistToEnter > 20.0 * m)) {
            event_action->enterPre[MuonSlot] = x0;
          }
          if ((DistToEnter < 5.0 * m) && (DistToEnter > -5.0 * m)) {
            event_action->enterMomentum[MuonSlot] =
                aStep->GetTrack()->GetMomentum().mag();
            event_action->enterPosition[MuonSlot] = x0;
            event_action->enterTime[MuonSlot] =
                aStep->GetTrack()->GetGlobalTime();
          }

          if ((DistToLeave < -20.0 * m) && (DistToLeave > -30.0 * m)) {
            event_action->leavePost[MuonSlot] = x0;
          }
          if ((DistToLeave < 30.0 * m) && (DistToLeave > 20.0 * m)) {
            event_action->leavePre[MuonSlot] = x0;
          }
          if ((DistToLeave < 5.0 * m) && (DistToLeave > -5.0 * m)) {
            event_action->leaveMomentum[MuonSlot] =
                aStep->GetTrack()->GetMomentum().mag();
            event_action->leavePosition[MuonSlot] = x0;
            event_action->leaveTime[MuonSlot] =
                aStep->GetTrack()->GetGlobalTime();
          }
        }
      }

      // here kill only muons that are leaving the can. All other
      // particles are already killed if outside the can
      G4ThreeVector distanceV;
      distanceV = x0 - myStDetector->detectorCenter;
      if ((x0[2] < myStDetector->bottomPosition) &&
          (p0[2] < 0)) {  // goes down while below the can
        aStep->GetTrack()->SetTrackStatus(fStopAndKill);
        return;
      }
      if ((x0[2] > myStDetector->detectorMaxz) &&
          (p0[2] > 0)) {  // goes up while above the can
        aStep->GetTrack()->SetTrackStatus(fStopAndKill);
        return;
      }
      direction = p0[0] * distanceV[0] + p0[1] * distanceV[1];
      distanceRho =
          sqrt(distanceV[0] * distanceV[0] + distanceV[1] * distanceV[1]);
      if ((distanceRho > myStDetector->detectorMaxRho) &&
          (direction > 0)) {  // goes away while outside the can
        aStep->GetTrack()->SetTrackStatus(fStopAndKill);
        return;
      }

    }  // if muon
  }    // if initial particle
}

G4double KM3SteppingAction::MuonRange(G4double KineticEnergy) {
  G4double ENERGYLOG = log10(KineticEnergy / GeV);
  G4double RANGELOG;
  if (ENERGYLOG < 1.0) {
    RANGELOG = P1LOW[0] + ENERGYLOG * P1LOW[1];
  } else if (ENERGYLOG > 8.0) {
    RANGELOG = P1HIGH[0] + ENERGYLOG * P1HIGH[1];
  } else {
    RANGELOG = P7[6] + ENERGYLOG * P7[7];
    RANGELOG = P7[5] + ENERGYLOG * RANGELOG;
    RANGELOG = P7[4] + ENERGYLOG * RANGELOG;
    RANGELOG = P7[3] + ENERGYLOG * RANGELOG;
    RANGELOG = P7[2] + ENERGYLOG * RANGELOG;
    RANGELOG = P7[1] + ENERGYLOG * RANGELOG;
    RANGELOG = P7[0] + ENERGYLOG * RANGELOG;
  }
  return meter * pow(10.0, RANGELOG);
}

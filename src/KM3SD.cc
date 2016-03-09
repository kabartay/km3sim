//
#include "KM3SD.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#ifdef G4TRACK_INFORMATION
#include "KM3TrackInformation.hh"
#endif


KM3SD::KM3SD(G4String name) : G4VSensitiveDetector(name) {
  G4String HCname;
  collectionName.insert(HCname = "MyCollection");
}

KM3SD::~KM3SD() {}


void KM3SD::Initialize(G4HCofThisEvent *HCE) {
  MyCollection =
    new KM3HitsCollection(SensitiveDetectorName, collectionName[0]);
#ifdef G4MYFIT_PARAMETERIZATION
  G4int TotalNumberOfCathods = myStDetector->allCathods->GetNumberOfCathods();
  if (TotalNumberOfCathods > 20000)
    G4Exception(
        "KM3SD::Initialize Number of cathods for energy fit is greater "
        "than 20000. Change the corresponding dimension in KM3SD.hh",
        "", FatalException, "");
  for (G4int i = 0; i < TotalNumberOfCathods; i++) ArrayParam[i] = 0;
  for (G4int i = 0; i < TotalNumberOfCathods; i++) ArrayParamAll[i] = 0;
#endif
}


G4bool KM3SD::ProcessHits(G4Step *aStep, G4TouchableHistory *ROhist) {
  //  G4cout<< aStep->GetTrack()->GetDefinition()->GetParticleName()<<G4endl;
  if (aStep->GetTrack()->GetDefinition()->GetParticleName() !=
      "opticalphoton")  // this may have to change (do not kiil every particle
      // on
      // the photocathod)
  {
    //    aStep->GetTrack()->SetTrackStatus(fStopAndKill);   //kill every
    //    particle except photons that hit the cathod or dead part of OM
    //    G4cout<<"STOPPED"<<" particle "<<
    //    aStep->GetTrack()->GetDefinition()->GetParticleName() <<G4endl;
    return false;
  }

  // next kill photons that are incident on the back end of the pmt. The
  // definition of deadvolume is obsolete. so the following never happens
  //  if
  //  (aStep->GetPreStepPoint()->GetTouchable()->GetVolume()->GetName()=="DeadVolume_PV")
  //  //for newgeant  add "_PV" at the end of physical volume name
  //  {
  //    aStep->GetTrack()->SetTrackStatus(fStopAndKill); //kill a photon that
  //    hits the dead part of the OM
  //    //G4cout<<"STOPPED"<<G4endl;
  //    return false;
  //  }

  G4double edep = aStep->GetTrack()->GetTotalEnergy();
  if (edep == 0.) {
    aStep->GetTrack()->SetTrackStatus(
        fStopAndKill);  // kill a photon with zero energy (?)
    return false;
  }

  // comment about the fit,em,ha parametrizations
  // In case that the Cathod is composed by water, there is a possibility that
  // the photon is scatered inside the Cathod. In this case the hit is double in
  // this Cathod. When it enters and when it leaves. In the setup with ~105000
  // cathods this increases the hit count by 0.2% when we have 60m scaterin
  // length and 0.6% when we have 20m scattering length. This is negligible and I
  // dont take this into account.
  if (MyCollection->entries() < 10000000) {
    G4ThreeVector photonDirection = aStep->GetTrack()->GetMomentumDirection();

    // newmie
    // here we disgard photons that have been scattered and are created with
    // parametrization also in KM3Cherenkov this for parametrization running. Not
    // anymore, since these are killed in the G4OpMie scattering process
#ifdef G4TRACK_INFORMATION
    KM3TrackInformation *info = NULL;
#endif

    /////next is new cathod id finding mode/////
    G4int Depth = aStep->GetPreStepPoint()->GetTouchable()->GetHistoryDepth();
    G4int History[10];
    for (G4int idep = 0; idep < Depth; idep++) {
      History[idep] =
        aStep->GetPreStepPoint()->GetTouchable()->GetReplicaNumber(Depth - 1 -
            idep);
    }
    G4int id = myStDetector->allCathods->GetCathodId(Depth, History);
    //////check if this photon passes after the angular acceptance//////
    G4ThreeVector PMTDirection = myStDetector->allCathods->GetDirection();
    G4double CathodRadius = myStDetector->allCathods->GetCathodRadius();
    G4double CathodHeight = myStDetector->allCathods->GetCathodHeight();
    if (!AcceptAngle(photonDirection.dot(PMTDirection), CathodRadius,
          CathodHeight, false)) {
      // at this point we dont kill the track if it is not accepted due to
      // anglular acceptance this has an observable effect a few percent only
      // when running simulation with parametrization turned off and sparce
      // detectors. However with dense detectors ~0.04 OMs/m^3 there is a 30%
      // effect, and 0.005OMs/m^3 a 3.5% effect, based on the covered solid
      // angle
      return false;
    }
    //    G4cout<<"Accepted"<<G4endl;
    //    G4cout<<"----------------------------------------------------------------"<<G4endl;

    KM3Hit *newHit = new KM3Hit();
    newHit->SetCathodId(id);
    newHit->SetTime(aStep->GetPostStepPoint()->GetGlobalTime());

    G4int originalTrackCreatorProcess;
    G4int originalParentID;
#ifdef G4TRACK_INFORMATION
    if (info == NULL)
      info = (KM3TrackInformation *)(aStep->GetTrack()->GetUserInformation());
    originalParentID = info->GetOriginalParentID();
    G4String creator = info->GetOriginalTrackCreatorProcess();
    if (creator == "KM3Cherenkov")
      originalTrackCreatorProcess = 0;
    else if (creator == "muPairProd")
      originalTrackCreatorProcess = 1;
    else if (creator == "muIoni")
      originalTrackCreatorProcess = 2;
    else if (creator == "muBrems")
      originalTrackCreatorProcess = 3;
    else if (creator == "muonNuclear")
      originalTrackCreatorProcess = 4;
    else if (creator == "Decay")
      originalTrackCreatorProcess = 8;
    else if (creator == "muMinusCaptureAtRest")
      originalTrackCreatorProcess = 9;
    else
      originalTrackCreatorProcess = 5;
#ifdef G4MYLASER_PARAMETERIZATION
    info->KeepScatteringPosition(aStep->GetPostStepPoint()->GetPosition(), 1.0);
    G4int NumberOfScatters = info->GetNumberOfScatters();
    newHit->SetIManyScatters(NumberOfScatters - 1);
    for (G4int is = 0; is < NumberOfScatters - 1; is++) {
      newHit->SetScatteringSteps(is, (info->GetScatteringPosition(is + 1) -
            info->GetScatteringPosition(is))
          .mag());
      newHit->SetScatteringAngles(is, info->GetScatteringAngle(is + 1));
    }
#endif
#else
    originalParentID = 1;
    originalTrackCreatorProcess = 0;
#endif
    G4int originalInfo;
    originalInfo = (originalParentID - 1) * 10 + originalTrackCreatorProcess;
    //    newHit->SetoriginalInfo(int(1.e6*h_Planck*c_light/aStep->GetTrack()->GetTotalEnergy()));
    newHit->SetoriginalInfo(originalInfo);
    newHit->SetMany(1);

    // short    G4ThreeVector posHit=aStep->GetPostStepPoint()->GetPosition();
    // short    G4ThreeVector posPMT=myStDetector->allCathods->GetPosition();
    // short    G4ThreeVector posRel=posHit-posPMT;

    // short    G4double angleThetaIncident,anglePhiIncident;
    // short    G4double angleThetaDirection,anglePhiDirection;

    // short    angleThetaIncident=posRel.theta();
    // short    anglePhiIncident=posRel.phi();
    // short    angleThetaDirection=photonDirection.theta();
    // short    anglePhiDirection=photonDirection.phi();
    // convert to degrees
    // short    angleThetaIncident *= 180./M_PI;
    // short    anglePhiIncident *= 180./M_PI;
    // short    angleThetaDirection *= 180./M_PI;
    // short    anglePhiDirection *= 180./M_PI;
    // short    if(anglePhiIncident < 0.0)anglePhiIncident += 360.0;
    // short    if(anglePhiDirection < 0.0)anglePhiDirection += 360.0;

    // short    G4int angleIncident,angleDirection;
    // short    angleIncident = (G4int)(nearbyint(angleThetaIncident)*1000.0 +
    // nearbyint(anglePhiIncident));
    // short    angleDirection = (G4int)(nearbyint(angleThetaDirection)*1000.0 +
    // nearbyint(anglePhiDirection));

    // short    newHit->SetangleIncident(angleIncident);
    // short    newHit->SetangleDirection(angleDirection);

    MyCollection->insert(newHit);
  }

  // killing must not been done, when we have EM or HA or FIT parametrizations
  // but it must be done for normal run, especially K40, SN and laser since
  // coincidences play big role there
  aStep->GetTrack()->SetTrackStatus(fStopAndKill);  // kill the detected photon

  return true;
}
// this method is used to add hits from the EM shower model
// short void KM3SD::InsertExternalHit(G4int id,G4double time,G4int
// originalInfo,G4int angleDirection,G4int angleIncident)
void KM3SD::InsertExternalHit(G4int id, const G4ThreeVector &OMPosition,
    G4double time, G4int originalInfo,
    const G4ThreeVector &photonDirection) {
  /////////calculate the photon speed at max QE to correct time
  static G4int ooo = 0;
  if (ooo == 0) {
    ooo = 1;
    G4Material *aMaterial = G4Material::GetMaterial("Cathod");
    G4double MaxQE = -1;
    G4double PhEneAtMaxQE;
    G4MaterialPropertyVector *aPropertyVector =
      aMaterial->GetMaterialPropertiesTable()->GetProperty("Q_EFF");
    for (size_t i = 0; i < aPropertyVector->GetVectorLength(); i++) {
      G4double ThisQE = (*aPropertyVector)[i];
      G4double ThisPhEne = aPropertyVector->Energy(i);
      if (ThisQE > MaxQE) {
        MaxQE = ThisQE;
        PhEneAtMaxQE = ThisPhEne;
      }
    }
    aMaterial = G4Material::GetMaterial("Water");
    G4MaterialPropertyVector *GroupVel =
      aMaterial->GetMaterialPropertiesTable()->GetProperty("GROUPVEL");
    // coresponds to the maximum qe each time. This is the right one
    thespeedmaxQE = GroupVel->Value(PhEneAtMaxQE);
  }
  //////////////////////////////////////////////////////////////
#ifndef G4MYFIT_PARAMETERIZATION
  if (MyCollection->entries() < 10000000) {
    G4ThreeVector PMTDirection = myStDetector->allCathods->GetDirection(id);
    //    G4cout << "OutFromParam "<<id<<"
    //    "<<photonDirection.dot(PMTDirection)<<G4endl;
    // the two 1.0 are the cathod radius and height that do not play any role
    // in parametrization it is not accepted
    if (!AcceptAngle(photonDirection.dot(PMTDirection), 1.0, 1.0,
          true)) {
      return;
    }
    // correct the time to correspond to the cathod positions and not the OM
    // position
    G4ThreeVector PMTPosition = myStDetector->allCathods->GetPosition(id);
    G4double Tcorr =
      (photonDirection.dot(PMTPosition - OMPosition)) / thespeedmaxQE;
    KM3Hit *newHit = new KM3Hit();
    newHit->SetCathodId(id);
    newHit->SetTime(time + Tcorr);
    // short    newHit->SetangleIncident(angleIncident);
    // short    newHit->SetangleDirection(angleDirection);
    newHit->SetoriginalInfo(originalInfo);
    newHit->SetMany(1);
    MyCollection->insert(newHit);
  }
#else
  G4ThreeVector dirPARAM(0.0, 0.0, 1.0);
  G4double startz = -600.0 * meter;
  G4ThreeVector vertexPARAM(0.0, 0.0, startz);
  G4ThreeVector posPMT = myStDetector->allCathods->GetPosition(id);
  G4double TRes = TResidual(time, posPMT, vertexPARAM, dirPARAM);
  if (fabs(TRes) < 20.0 * ns) ArrayParam[id]++;
  if ((TRes > -20.0 * ns) && (TRes < 100.0 * ns)) ArrayParamAll[id]++;
  //  G4cout<<"ForFit "<<posPMT[0]/m<<" "<<posPMT[1]/m<<" "<<posPMT[2]/m<<"
  //  "<<time<<" "<<TRes<<G4endl;
#endif
}

void KM3SD::EndOfEvent(G4HCofThisEvent *HCE) {
  if (verboseLevel > 0) {
    G4int TotalNumberOfCathods = myStDetector->allCathods->GetNumberOfCathods();
    outfile = myStDetector->outfile;
    G4int NbHits = MyCollection->entries();
    static G4int ooo = 0;                               // count total
    ooo += NbHits;                                      // count total
    G4cout << "Total Hits: " << ooo << G4endl;          // count total
    G4cout << "This Event Hits: " << NbHits << G4endl;  // count for this event
    int i;

    // here we sort MyCollection according to ascending pmt number
    std::vector<KM3Hit *> *theCollectionVector = MyCollection->GetVector();
    QuickSort(0, theCollectionVector, 0, NbHits - 1);
    //-------//from now on the hits are sorted in cathod id
    // next sort according to ascending time for each cathod id
    if (NbHits > 1) {
      G4int prevCathod, currentCathod;
      G4int istart, istop;
      istart = 0;
      prevCathod = (*MyCollection)[istart]->GetCathodId();
      for (i = 1; i < NbHits; i++) {
        currentCathod = (*MyCollection)[i]->GetCathodId();
        if (currentCathod != prevCathod) {
          istop = i - 1;
          QuickSort(1, theCollectionVector, istart, istop);
          G4double MergeWindow = 0.5 * ns;
          MergeHits(istart, istop + 1, MergeWindow);
#ifdef G4MYLASER_PARAMETERIZATION
          G4double MergeWindow = 0.1 * ns;
          MergeHits(istart, istop + 1, MergeWindow);
#endif
          prevCathod = currentCathod;
          istart = i;
        } else if ((currentCathod == prevCathod) && (i == NbHits - 1)) {
          istop = i;
          QuickSort(1, theCollectionVector, istart, istop);
          G4double MergeWindow = 0.5 * ns;
          MergeHits(istart, istop + 1, MergeWindow);
#ifdef G4MYLASER_PARAMETERIZATION
          G4double MergeWindow = 0.1 * ns;
          MergeHits(istart, istop + 1, MergeWindow);
#endif
        }
      }
    }

    // find the number of hit entries to write
    G4int NbHitsWrite = 0;
    for (i = 0; i < NbHits; i++)
      if ((*MyCollection)[i]->GetMany() > 0) NbHitsWrite++;

    // find earliest hit time
    G4double timefirst = 1E20;
    for (i = 0; i < NbHits; i++) {
      if ((*MyCollection)[i]->GetTime() < timefirst &&
          (*MyCollection)[i]->GetMany() > 0)
        timefirst = (*MyCollection)[i]->GetTime();
    }

    // find how many cathods are hitted
    int allhit = 0;
    int prevcathod = -1;
    for (i = 0; i < NbHits; i++) {
      if (prevcathod != (*MyCollection)[i]->GetCathodId()) allhit++;
      prevcathod = (*MyCollection)[i]->GetCathodId();
    }
    if (!myStDetector->useANTARESformat)
      fprintf(outfile, "%.7e %d\n", timefirst * 1E-9, allhit);
    else
      myStDetector->TheEVTtoWrite->AddNumberOfHits(NbHitsWrite);

    // find the last pmt how many hits has
    G4int LastPmtNumber;
    G4int LastHitNumber;
    for (i = NbHits - 1; i >= 0; i--)
      if ((*MyCollection)[i]->GetMany() > 0) {
        LastPmtNumber = (*MyCollection)[i]->GetCathodId();
        LastHitNumber = i;
        break;
      }
    G4int LastPmtNumHits = 0;
    for (i = NbHits - 1; i >= 0; i--)
      if ((*MyCollection)[i]->GetMany() > 0) {
        if ((*MyCollection)[i]->GetCathodId() == LastPmtNumber)
          LastPmtNumHits++;
        else
          break;
      }

    // new
    int numphotons = 0;
    G4double firstphoton = 1.E50;
    int numpes = 0;
    if (NbHits > 0) prevcathod = (*MyCollection)[0]->GetCathodId();
    int prevstart = 0;
    int numhit = 0;
    for (i = 0; i < NbHits; i++) {
      if ((*MyCollection)[i]->GetMany() > 0) {
        if (prevcathod == (*MyCollection)[i]->GetCathodId()) {
          numphotons++;
          numpes += (*MyCollection)[i]->GetMany();
          if ((*MyCollection)[i]->GetTime() - timefirst < firstphoton)
            firstphoton = (*MyCollection)[i]->GetTime() - timefirst;
        } else {
          if (myStDetector->vrmlhits) {  // draw hits
            G4ThreeVector Cposition =
              myStDetector->allCathods->GetPosition(prevcathod);
            DrawCathodHit(numpes, Cposition);
          }
          if (!myStDetector->useANTARESformat)
            fprintf(outfile, "%d %d %.7e\n", prevcathod, numphotons,
                firstphoton * 1E-9);
          for (int j = prevstart; j < i; j++) {
            if ((*MyCollection)[j]->GetMany() > 0) {
              numhit++;
              if (!myStDetector->useANTARESformat) {
                fprintf(outfile, "%.7e %d %d\n",
                    ((*MyCollection)[j]->GetTime() - timefirst) * 1E-9,
                    (*MyCollection)[j]->GetoriginalInfo(),
                    (*MyCollection)[j]->GetMany());
#if defined(G4MYLASER_PARAMETERIZATION) && defined(G4TRACK_INFORMATION)
                G4int IManyScatters = (*MyCollection)[j]->GetIManyScatters();
                fprintf(outfile, "%d\n", IManyScatters);
                for (G4int is = 0; is < IManyScatters; is++)
                  fprintf(outfile, "%.3e %.6e\n",
                      (*MyCollection)[j]->GetScatteringSteps(is) / m,
                      (*MyCollection)[j]->GetScatteringAngles(is));
#endif
              } else {
                // here write antares format info
                G4int originalInfo = (*MyCollection)[j]->GetoriginalInfo();
                G4int originalParticleNumber = originalInfo / 10 + 1;
                G4int originalTrackCreatorProcess =
                  originalInfo - (originalParticleNumber - 1) * 10;
                myStDetector->TheEVTtoWrite->AddHit(
                    numhit, prevcathod, double((*MyCollection)[j]->GetMany()),
                    (*MyCollection)[j]->GetTime(), originalParticleNumber,
                    (*MyCollection)[j]->GetMany(),
                    (*MyCollection)[j]->GetTime(), originalTrackCreatorProcess);
              }
            }
          }
          prevstart = i;
          numphotons = 1;
          firstphoton = (*MyCollection)[i]->GetTime() - timefirst;
          numpes = (*MyCollection)[i]->GetMany();
        }
        prevcathod = (*MyCollection)[i]->GetCathodId();
        //  if(i == (NbHits-1) ){
        //  if(numhit == (NbHitsWrite-1) ){
        if (numhit == (NbHitsWrite - LastPmtNumHits) && i == LastHitNumber) {
          if (myStDetector->vrmlhits) {  // draw hits
            G4ThreeVector Cposition = myStDetector->allCathods->GetPosition(
                (*MyCollection)[i]->GetCathodId());
            DrawCathodHit(numpes, Cposition);
          }
          if (!myStDetector->useANTARESformat)
            fprintf(outfile, "%d %d %.7e\n", (*MyCollection)[i]->GetCathodId(),
                numphotons, firstphoton * 1E-9);
          for (int j = prevstart; j < NbHits; j++) {
            if ((*MyCollection)[j]->GetMany() > 0) {
              numhit++;
              if (!myStDetector->useANTARESformat) {
                fprintf(outfile, "%.7e %d %d\n",
                    ((*MyCollection)[j]->GetTime() - timefirst) * 1E-9,
                    (*MyCollection)[j]->GetoriginalInfo(),
                    (*MyCollection)[j]->GetMany());
#if defined(G4MYLASER_PARAMETERIZATION) && defined(G4TRACK_INFORMATION)
                G4int IManyScatters = (*MyCollection)[j]->GetIManyScatters();
                fprintf(outfile, "%d\n", IManyScatters);
                for (G4int is = 0; is < IManyScatters; is++)
                  fprintf(outfile, "%.3e %.6e\n",
                      (*MyCollection)[j]->GetScatteringSteps(is) / m,
                      (*MyCollection)[j]->GetScatteringAngles(is));
#endif
              } else {
                // here write antares format info
                G4int originalInfo = (*MyCollection)[j]->GetoriginalInfo();
                G4int originalParticleNumber = originalInfo / 10 + 1;
                G4int originalTrackCreatorProcess =
                  originalInfo - (originalParticleNumber - 1) * 10;
                myStDetector->TheEVTtoWrite->AddHit(
                    numhit, (*MyCollection)[i]->GetCathodId(),
                    double((*MyCollection)[j]->GetMany()),
                    (*MyCollection)[j]->GetTime(), originalParticleNumber,
                    (*MyCollection)[j]->GetMany(),
                    (*MyCollection)[j]->GetTime(), originalTrackCreatorProcess);
              }
            }
          }
        }
      }
      }

      // old
      // int numphotons;G4double firstphoton;
      // for(int ica=0;ica<TotalNumberOfCathods;ica++){ //for all benthos
      //   numphotons=0;firstphoton=1.E50;
      //   for(i=0;i<NbHits;i++){
      //   if((*MyCollection)[i]->GetCathodId()==ica){
      //     numphotons++;  // hit number for this benthos
      //     if((*MyCollection)[i]->GetTime()-timefirst<firstphoton)firstphoton=(*MyCollection)[i]->GetTime()-timefirst;
      //   }
      //   }
      //   if(numphotons>0){
      //   if(myStDetector->vrmlhits){  //draw hits
      //     G4ThreeVector Cposition=myStDetector->allCathods->GetPosition(ica);
      //     DrawCathodHit(numphotons,Cposition);
      //   }
      //   fprintf(outfile,"%d %d %.7e
      // %d\n",ica,numphotons,firstphoton*1E-9,numphotons);
      //   for (i=0;i<NbHits;i++){
      //     if((*MyCollection)[i]->GetCathodId()==ica)
      //       fprintf(outfile,"%.7e %d %d %d\n",
      //         ((*MyCollection)[i]->GetTime()-timefirst)*1E-9,
      //         (*MyCollection)[i]->GetangleIncident(),
      //         (*MyCollection)[i]->GetangleDirection(),
      //         (*MyCollection)[i]->GetoriginalInfo());
      //   }
      //   }
      // }
      // old
#endif
      if (myStDetector->vrmlhits) {
        static G4int HCID = -1;
        if (HCID < 0) {
          HCID = GetCollectionID(0);
        }
        HCE->AddHitsCollection(HCID, MyCollection);
      } else
        delete MyCollection;

      // G4cout << "\n-------->Hits Collection: in this event they are " <<
      // MyCollection->entries() << G4endl;
      // for (G4int i=0;i<NbHits;i++) (*MyCollection)[i]->Print();
      }
    }
#else
    void KM3SD::EndOfEvent(G4HCofThisEvent *HCE) { delete MyCollection; }
#endif

    void KM3SD::MergeHits(G4int nfirst, G4int nlast, G4double MergeWindow) {
      if (nlast - nfirst < 2) return;
      G4int iuu, imany;
      G4int istart = nfirst;
      G4int istop = nfirst;
go77:
      istart = istop + 1;
      for (iuu = istart + 1; iuu <= nlast; iuu++) {
        if (((*MyCollection)[iuu - 1]->GetTime() -
              (*MyCollection)[istart - 1]->GetTime()) > MergeWindow) {
          istop = iuu - 1;
          goto go78;
        }
      }
      istop = iuu - 1;
go78:
      if (istart > nlast) return;
      imany = istop - istart + 1;
      if (imany > 1) {
        G4double MeanTime = 0.0;
        for (iuu = istart; iuu <= istop; iuu++)
          MeanTime += (*MyCollection)[iuu - 1]->GetTime();
        MeanTime /= imany;
        (*MyCollection)[istart - 1]->SetTime(MeanTime);
        (*MyCollection)[istart - 1]->SetMany(imany);
        for (iuu = istart + 1; iuu <= istop; iuu++)
          (*MyCollection)[iuu - 1]->SetMany(0);
      }
      goto go77;
    }

#include "G4VisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"

    // Hit Draw Method (colours are descibing the number of photons, blue->red)
    void KM3SD::DrawCathodHit(G4int NumberOfPhotons, G4ThreeVector pos) {
      G4VVisManager *pVVisManager = G4VVisManager::GetConcreteInstance();
      if (pVVisManager) {
        G4Circle circle(pos);
        circle.SetWorldRadius(220.0);
        circle.SetFillStyle(G4Circle::filled);
        G4double nphotons = G4double(NumberOfPhotons);
        if (nphotons > 100.0) nphotons = 100.0;
        G4double redcol = log10(G4double(nphotons)) / 2.0;
        G4Colour colour(redcol, 0., 1.0 - redcol);
        G4VisAttributes attribs(colour);
        circle.SetVisAttributes(attribs);
        pVVisManager->Draw(circle);
      }
    }

    G4int KM3SD::ProcessMyCollection(KM3HitsCollection *aCollection) { return (0); }

    void KM3SD::clear() {}

    void KM3SD::PrintAll() {}

    G4double KM3SD::TResidual(G4double time, const G4ThreeVector &position,
        const G4ThreeVector &vertex,
        const G4ThreeVector &dir) {
      G4double tnthc = 0.961;  // this value depends on qe and water properties
      G4double ag, bg;
      G4ThreeVector Hit = position - vertex;
      ag = dir.dot(Hit);
      Hit -= ag * dir;
      bg = Hit.mag();
      return time - (ag + bg * tnthc) / c_light;
    }

    // Quick Sort Functions for Ascending Order
    void KM3SD::QuickSort(G4int shorttype,
        std::vector<KM3Hit *> *theCollectionVector, G4int top,
        G4int bottom) {
      // top = subscript of beginning of array
      // bottom = subscript of end of array

      G4int middle;
      if (top < bottom) {
        if (shorttype == 0)
          middle = partition_CathodId(theCollectionVector, top, bottom);
        else
          middle = partition_Time(theCollectionVector, top, bottom);
        QuickSort(shorttype, theCollectionVector, top,
            middle);  // sort first section
        QuickSort(shorttype, theCollectionVector, middle + 1,
            bottom);  // sort second section
      }
      return;
    }

    // Function to determine the partitions
    // partitions the array and returns the middle subscript
    G4int KM3SD::partition_CathodId(std::vector<KM3Hit *> *theCollectionVector,
        G4int top, G4int bottom) {
      G4int x = (*theCollectionVector)[top]->GetCathodId();
      G4int i = top - 1;
      G4int j = bottom + 1;
      KM3Hit *temp;
      do {
        do {
          j--;
        } while (x < (*theCollectionVector)[j]->GetCathodId());

        do {
          i++;
        } while (x > (*theCollectionVector)[i]->GetCathodId());

        if (i < j) {
          temp = (*theCollectionVector)[i];
          (*theCollectionVector)[i] = (*theCollectionVector)[j];
          (*theCollectionVector)[j] = temp;
        }
      } while (i < j);
      return j;  // returns middle subscript
    }

    // Function to determine the partitions
    // partitions the array and returns the middle subscript
    G4int KM3SD::partition_Time(std::vector<KM3Hit *> *theCollectionVector,
        G4int top, G4int bottom) {
      G4double x = (*theCollectionVector)[top]->GetTime();
      G4int i = top - 1;
      G4int j = bottom + 1;
      KM3Hit *temp;
      do {
        do {
          j--;
        } while (x < (*theCollectionVector)[j]->GetTime());

        do {
          i++;
        } while (x > (*theCollectionVector)[i]->GetTime());

        if (i < j) {
          temp = (*theCollectionVector)[i];
          (*theCollectionVector)[i] = (*theCollectionVector)[j];
          (*theCollectionVector)[j] = temp;
        }
      } while (i < j);
      return j;  // returns middle subscript
    }

    // the angular acceptance is according to the MultiPMT OM (WPD Document January
    // 2011)
    // doing linear interpolation
    // if shapespherical==true then it is from parametrization and take into account
    // only the <<experimental>> angular acceptance
    // if shapespherical==false then it is not from param and take also the
    // simulated angular acceptance of the cathod shape
    G4bool KM3SD::AcceptAngle(G4double cosangle, G4double CathodRadius,
        G4double CathodHeight, bool shapespherical) {
      static G4MaterialPropertyVector *Ang_Acc = NULL;
      static G4double MinCos_Acc = -1.0;
      static G4double MaxCos_Acc = 0.25;
      if (Ang_Acc == NULL) {
        G4Material *aMaterial = G4Material::GetMaterial("Cathod");
        Ang_Acc = aMaterial->GetMaterialPropertiesTable()->GetProperty(
            "ANGULAR_ACCEPTANCE");
        MinCos_Acc = Ang_Acc->GetMinLowEdgeEnergy();
        MaxCos_Acc = Ang_Acc->GetMaxLowEdgeEnergy();
      }

      if (cosangle > MaxCos_Acc) return false;
      if (cosangle < MinCos_Acc) return true;
      G4double AngAcc = Ang_Acc->Value(cosangle);

      // the following is the simulated angular acceptance (from the shape of the
      // photocathod)
      // A cylinder of radius R and height d has angular acceptance vs costh as
      // a(x)=abs(x)+(2*d/(pi*R))*sqrt(1-x*x)
      G4double AngularAccSim;

      if (!shapespherical) {
        AngularAccSim = fabs(cosangle) +
          (2.0 * CathodHeight / (pi * CathodRadius)) *
          sqrt(1 - cosangle * cosangle);
        AngAcc /= AngularAccSim;
      }

      if (G4UniformRand() <= AngAcc) return true;
      return false;
    }

    //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

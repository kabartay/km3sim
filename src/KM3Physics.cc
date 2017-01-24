#include "globals.hh"
#include "KM3Physics.h"

#include "G4UserSpecialCuts.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleWithCuts.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4ParticleTypes.hh"
#include "G4ParticleTable.hh"
#include "G4Material.hh"
#include "G4ios.hh"

#include "G4LeptonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4ShortLivedConstructor.hh"
#include "G4IonConstructor.hh"

#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"
//#include "G4GammaConversionToMuons.hh"
#include "G4PhotoElectricEffect.hh"

// newgeant #include "G4MultipleScattering.h"
// in version 4.9.3.p02 geant4 threatens than G4MultipleScattering class
// will be removed in next releases, so the next ones are assigned for
// e+-, mu+-, hadrons and ions, respectively
#include "G4eMultipleScattering.hh"
#include "G4MuMultipleScattering.hh"
#include "G4hMultipleScattering.hh"

#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eplusAnnihilation.hh"
#include "G4MuIonisation.hh"
#include "G4MuBremsstrahlung.hh"
#include "G4MuPairProduction.hh"

// this is apparently renamed/merged in 4.9.10
// see http://geant4.cern.ch/support/ReleaseNotes4.10.0.html
//#include "G4MuonMinusCaptureAtRest.hh"
#include "G4MuonMinusCapture.hh"
//#include "G4LEAlphaInelastic.hh"
//#include "G4AlphaInelastic.hh"
#include "G4AlphaInelasticProcess.hh"
//#include "G4LEDeuteronInelastic.hh"
#include "G4DeuteronInelasticProcess.hh"
//#include "G4LETritonInelastic.hh"
#include "G4TritonInelasticProcess.hh"
//#include "G4PionMinusAbsorptionAtRest.hh"
#include "G4PiMinusAbsorptionBertini.hh"
//#include "G4KaonMinusAbsorption.hh"
#include "G4KaonMinusAbsorptionBertini.hh"

#include "G4MuonNuclearProcess.hh"  //transition to 4.9.6
#include "G4MuonVDNuclearModel.hh"  //transition to 4.9.6
#include "G4hBremsstrahlung.hh"
#include "G4hPairProduction.hh"
#include "G4HadronicProcessStore.hh"
#include "G4HadronElasticPhysics.hh"
#include "G4HadronPhysicsQGSP_FTFP_BERT.hh"
//#include "G4AnnihiToMuPair.hh"
#include "G4hIonisation.hh"

#include "G4OpAbsorption.hh"
#include "G4OpMie.h"
#include "G4Decay.hh"
#include "G4RadioactiveDecay.hh"
#include "G4IonTable.hh"
#include "G4Ions.hh"

#include "globals.hh"
#include "KM3Physics.h"

#include "G4UserSpecialCuts.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleWithCuts.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4ParticleTypes.hh"
#include "G4ParticleTable.hh"
#include "G4Material.hh"
#include "G4ios.hh"

#include "G4LeptonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4ShortLivedConstructor.hh"
#include "G4IonConstructor.hh"

#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"
//#include "G4GammaConversionToMuons.hh"
#include "G4PhotoElectricEffect.hh"

// newgeant #include "G4MultipleScattering.h" //in version 4.9.3.p02 geant4
// threatens than G4MultipleScattering class will be removed in next releases,
// so the next ones are assigned for e+-, mu+-, hadrons and ions, respectively
#include "G4eMultipleScattering.hh"
#include "G4MuMultipleScattering.hh"
#include "G4hMultipleScattering.hh"

#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eplusAnnihilation.hh"
#include "G4MuIonisation.hh"
#include "G4MuBremsstrahlung.hh"
#include "G4MuPairProduction.hh"
#include "G4MuonNuclearProcess.hh"  //transition to 4.9.6
#include "G4MuonVDNuclearModel.hh"  //transition to 4.9.6
#include "G4hBremsstrahlung.hh"
#include "G4hPairProduction.hh"
#include "G4HadronicProcessStore.hh"
#include "G4HadronElasticPhysics.hh"
#include "G4HadronPhysicsQGSP_FTFP_BERT.hh"
#include "G4AlphaInelasticProcess.hh"
//#include "G4LEAlphaInelastic.hh"
#include "G4DeuteronInelasticProcess.hh"
//#include "G4LEDeuteronInelastic.hh"
#include "G4TritonInelasticProcess.hh"
//#include "G4LETritonInelastic.hh"
//#include "G4PionMinusAbsorptionAtRest.hh"
//#include "G4KaonMinusAbsorption.hh"
//#include "G4AnnihiToMuPair.hh"
#include "G4hIonisation.hh"

#include "G4OpAbsorption.hh"
#include "G4OpMie.h"
#include "G4Decay.hh"
#include "G4RadioactiveDecay.hh"
#include "G4IonTable.hh"
#include "G4Ions.hh"

using CLHEP::electron_mass_c2;
using CLHEP::GeV;
using CLHEP::keV;
using CLHEP::MeV;
using CLHEP::mm;
using CLHEP::PeV;
using CLHEP::TeV;

/*
 *
 * OK... nbins is always 220
 * also, you cant set the binning at individual processes
 * => set binning globally?
 *
 */


KM3Physics::KM3Physics() : G4VUserPhysicsList() {
  defaultCutValue = 0.5 * mm;
  SetVerboseLevel(2);
}

KM3Physics::~KM3Physics() { delete theCerenkovProcess; }

void KM3Physics::ConstructParticle() {
  // In this method, static member functions should be called
  // for all particles which you want to use.
  // This ensures that objects of these particle types will be
  // created in the program.

  G4BosonConstructor aCBoson;
  G4LeptonConstructor aCLepton;
  G4BaryonConstructor aCBaryon;
  G4MesonConstructor aCMeson;
  G4ShortLivedConstructor aCShort;
  G4IonConstructor aCIon;
  aCBoson.ConstructParticle();
  aCLepton.ConstructParticle();
  aCBaryon.ConstructParticle();
  aCMeson.ConstructParticle();
  aCShort.ConstructParticle();
  aCIon.ConstructParticle();
}

void KM3Physics::ConstructProcess() {
  AddTransportation();
  //AddParameterisation();
  ConstructEM();
  // construct hadronic processes only in case of Pythia input
  // (for apparent reasons)
  ConstructHA();
  ConstructGeneral();
  ConstructOP();

  // Geant 4.10.3
#ifndef theParticleIterator
  theParticleIterator = GetParticleIterator();
#endif

  // tempo list all processes for all particles defined up to the end

  // theParticleIterator->reset();
  // while( (*theParticleIterator)() ){
  //   G4ParticleDefinition* particle = theParticleIterator->value();
  //   G4ProcessManager* pmanager = particle->GetProcessManager();
  //   G4String particleName = particle->GetParticleName();
  //   G4ProcessVector* aVector=pmanager->GetProcessList();
  //   G4int isi=aVector->size();
  //   if(!particle->IsShortLived())
  //     G4cout << "------------------ "<<particleName<<" "<<isi<<"
  //     ------------------"<<G4endl;
  //   else
  //     G4cout << "----SHORTSHORT---- "<<particleName<<" "<<isi<<"
  //     ----SHORTSHORT----"<<G4endl;
  //   for( G4int iii=0 ; iii<isi ; iii++){
  //     G4VProcess* app=(*aVector)[iii];
  //     G4String ass=app->GetProcessName();
  //     G4cout << ass << G4endl;
  //   }
  // }

  // tempo
}

// Attention. I must change the high energy of all electromagnetic processes
void KM3Physics::ConstructEM() {
  theParticleIterator->reset();
  while ((*theParticleIterator)()) {
    G4ParticleDefinition *particle = theParticleIterator->value();
    G4ProcessManager *pmanager = particle->GetProcessManager();
    G4String particleName = particle->GetParticleName();

    G4double lowE;
    G4double highE;
    G4int nBins;

    if (particleName == "gamma") {
      // gamma conversion to e+e- pair
      G4GammaConversion *aGammaConversion = new G4GammaConversion();
      lowE = 2 * electron_mass_c2;
      highE = 100.0 * PeV;  // 100TeV before
      nBins = 220;          // 120 before
      //      aGammaConversion->SetPhysicsTableBining(lowE,highE,nBins);
      //      //not applicable to Geant 4.8
      //aGammaConversion->SetLambdaBinning(nBins);
      aGammaConversion->SetMinKinEnergy(lowE);
      aGammaConversion->SetMaxKinEnergy(highE);
      pmanager->AddDiscreteProcess(aGammaConversion);
      // gamma conversion to mu+mu- pair is not likely to happen
      //      pmanager->AddDiscreteProcess(new G4GammaConversionToMuons());
      G4ComptonScattering *aComptonScattering = new G4ComptonScattering();
      lowE = 1 * keV;
      highE = 100.0 * PeV;  // 100TeV before
      nBins = 220;          // 120 before
      //      aComptonScattering->SetPhysicsTableBining(lowE,highE,nBins);
      //      //not applicable to Geant 4.8
      //aComptonScattering->SetLambdaBinning(nBins);
      aComptonScattering->SetMinKinEnergy(lowE);
      aComptonScattering->SetMaxKinEnergy(highE);
      pmanager->AddDiscreteProcess(aComptonScattering);
      pmanager->AddDiscreteProcess(new G4PhotoElectricEffect());
      //      pmanager->AddProcess( new G4UserSpecialCuts(),-1,-1,1);

    } else if (particleName == "e-") {
      // electron
      highE = 100.0 * PeV;  // 100TeV before
      nBins = 220;          // 120 before
      G4eMultipleScattering *theeminusMultipleScattering =
          new G4eMultipleScattering();
      //      theeminusMultipleScattering->SetBinning(nBins);
      //      theeminusMultipleScattering->SetMaxKinEnergy(highE);
      G4eIonisation *theeminusIonisation = new G4eIonisation();
      theeminusIonisation->SetDEDXBinning(nBins);
      //theeminusIonisation->SetLambdaBinning(nBins);
      theeminusIonisation->SetMaxKinEnergy(highE);
      G4eBremsstrahlung *theeminusBremsstrahlung = new G4eBremsstrahlung();
      //theeminusBremsstrahlung->SetLambdaBinning(nBins);
      theeminusBremsstrahlung->SetDEDXBinning(nBins);
      theeminusBremsstrahlung->SetMaxKinEnergy(highE);
      //      ((G4eIonisation*)theeminusIonisation)->SetLowerBoundLambda(0.00024);
      //      G4cout<<"lower lambda
      //      e-"<<((G4eIonisation*)theeminusIonisation)->GetLowerBoundLambda()<<G4endl;
      //
      // add processes
      pmanager->AddProcess(theeminusMultipleScattering);
      pmanager->AddProcess(theeminusIonisation);
      pmanager->AddProcess(theeminusBremsstrahlung);
      // pmanager->AddProcess( new G4UserSpecialCuts(),-1,-1,1);
      //
      // set ordering for AlongStepDoIt
      pmanager->SetProcessOrdering(theeminusMultipleScattering, idxAlongStep,
                                   1);
      pmanager->SetProcessOrdering(theeminusIonisation, idxAlongStep, 2);
      // KM3Cherenkov.h:
      // set ordering for PostStepDoIt
      pmanager->SetProcessOrdering(theeminusMultipleScattering, idxPostStep, 1);
      pmanager->SetProcessOrdering(theeminusIonisation, idxPostStep, 2);
      pmanager->SetProcessOrdering(theeminusBremsstrahlung, idxPostStep, 3);

    } else if (particleName == "e+") {
      // positron
      highE = 100.0 * PeV;  // 100TeV before
      nBins = 220;          // 120 before
      G4eMultipleScattering *theeplusMultipleScattering =
          new G4eMultipleScattering();
      //      theeplusMultipleScattering->SetBinning(nBins);
      //      theeplusMultipleScattering->SetMaxKinEnergy(highE);
      G4eIonisation *theeplusIonisation = new G4eIonisation();
      theeplusIonisation->SetDEDXBinning(nBins);
      //theeplusIonisation->SetLambdaBinning(nBins);
      theeplusIonisation->SetMaxKinEnergy(highE);
      G4eBremsstrahlung *theeplusBremsstrahlung = new G4eBremsstrahlung();
      //theeplusBremsstrahlung->SetLambdaBinning(nBins);
      theeplusBremsstrahlung->SetDEDXBinning(nBins);
      theeplusBremsstrahlung->SetMaxKinEnergy(highE);
      G4eplusAnnihilation *theeplusAnnihilation = new G4eplusAnnihilation();
      //theeplusAnnihilation->SetLambdaBinning(nBins);
      theeplusAnnihilation->SetMaxKinEnergy(highE);
      //      G4VProcess* theAnnihiToMuPair          = new G4AnnihiToMuPair();
      //      //is not likely to happen
      //      ((G4eIonisation*)theeplusIonisation)->SetLowerBoundLambda(0.00024);
      //      G4cout<<"lower lambda
      //      e+"<<((G4eIonisation*)theeplusIonisation)->GetLowerBoundLambda()<<G4endl;
      //
      // add processes
      pmanager->AddProcess(theeplusMultipleScattering);
      pmanager->AddProcess(theeplusIonisation);

      pmanager->AddProcess(theeplusBremsstrahlung);
      pmanager->AddProcess(theeplusAnnihilation);
      //      pmanager->AddProcess(theAnnihiToMuPair);
      // pmanager->AddProcess( new G4UserSpecialCuts(),-1,-1,1);
      //
      // set ordering for AtRestDoIt
      pmanager->SetProcessOrderingToFirst(theeplusAnnihilation, idxAtRest);
      //
      // set ordering for AlongStepDoIt
      pmanager->SetProcessOrdering(theeplusMultipleScattering, idxAlongStep, 1);
      pmanager->SetProcessOrdering(theeplusIonisation, idxAlongStep, 2);
      //
      // set ordering for PostStepDoIt
      pmanager->SetProcessOrdering(theeplusMultipleScattering, idxPostStep, 1);
      pmanager->SetProcessOrdering(theeplusIonisation, idxPostStep, 2);
      pmanager->SetProcessOrdering(theeplusBremsstrahlung, idxPostStep, 3);
      pmanager->SetProcessOrdering(theeplusAnnihilation, idxPostStep, 4);
      //      pmanager->SetProcessOrdering(theAnnihiToMuPair, idxPostStep,5);

    } else if (particleName == "mu+" || particleName == "mu-") {
      // muon
      lowE = 500.0 * GeV;
      highE = 100.0 * PeV;  // 100TeV before
      nBins = 220;          // 120 before
      G4MuMultipleScattering *aMultipleScattering =
          new G4MuMultipleScattering();
      //      aMultipleScattering->SetBinning(nBins);
      //      aMultipleScattering->SetMaxKinEnergy(highE);
      G4MuBremsstrahlung *aBremsstrahlung = new G4MuBremsstrahlung();
      //aBremsstrahlung->SetLambdaBinning(nBins);
      aBremsstrahlung->SetDEDXBinning(nBins);
      aBremsstrahlung->SetMaxKinEnergy(highE);
      G4MuPairProduction *aPairProduction = new G4MuPairProduction();
      //aPairProduction->SetLambdaBinning(nBins);
      aPairProduction->SetDEDXBinning(nBins);
      aPairProduction->SetMaxKinEnergy(highE);
      G4MuIonisation *anIonisation = new G4MuIonisation();
      //anIonisation->SetLambdaBinning(nBins);
      anIonisation->SetDEDXBinning(nBins);
      anIonisation->SetMaxKinEnergy(highE);

      G4MuonNuclearProcess *aMuNuclearInteraction = new G4MuonNuclearProcess();
      aMuNuclearInteraction->SetVerboseLevel(0);
      G4MuonVDNuclearModel *muNucModel = new G4MuonVDNuclearModel();
      muNucModel->SetMaxEnergy(highE);
      aMuNuclearInteraction->RegisterMe(muNucModel);
      //      ((G4MuPairProduction*)aPairProduction)->SetLowerBoundLambda(0.0021);
      //      G4cout<<"lower lambda pair m
      //      "<<((G4MuPairProduction*)aPairProduction)->GetLowerBoundLambda()<<G4endl;
      //
      // add processes
      pmanager->AddProcess(anIonisation);
      pmanager->AddProcess(aMultipleScattering);
      pmanager->AddProcess(aBremsstrahlung);
      pmanager->AddProcess(aPairProduction);

      // new add muon minus capture at rest
      if (particleName == "mu-")
        pmanager->AddRestProcess(new G4MuonMinusCapture);
      pmanager->AddDiscreteProcess(aMuNuclearInteraction);
      //
      // set ordering for AlongStepDoIt
      pmanager->SetProcessOrdering(aMultipleScattering, idxAlongStep, 1);
      pmanager->SetProcessOrdering(anIonisation, idxAlongStep, 2);
      //
      // set ordering for PostStepDoIt
      pmanager->SetProcessOrdering(aMultipleScattering, idxPostStep, 1);
      pmanager->SetProcessOrdering(anIonisation, idxPostStep, 2);
      pmanager->SetProcessOrdering(aBremsstrahlung, idxPostStep, 3);
      pmanager->SetProcessOrdering(aPairProduction, idxPostStep, 4);
      pmanager->SetProcessOrdering(aMuNuclearInteraction, idxPostStep, 5);

    } else if ((!particle->IsShortLived()) &&
               (particle->GetPDGCharge() != 0.0) &&
               (particle->GetParticleName() != "chargedgeantino")) {
      // all others charged particles except geantino
      highE = 100.0 * PeV;  // 100TeV before
      nBins = 220;          // 120 before

      G4hMultipleScattering *aMultipleScattering = new G4hMultipleScattering();
      //      aMultipleScattering->SetBinning(nBins);
      //      aMultipleScattering->SetMaxKinEnergy(highE);

      G4hIonisation *anIonisation = new G4hIonisation();
      //anIonisation->SetLambdaBinning(nBins);
      anIonisation->SetDEDXBinning(nBins);
      anIonisation->SetMaxKinEnergy(highE);

      ////G4VProcess*  theUserCuts = new G4UserSpecialCuts();

      //
      // add processes
      pmanager->AddProcess(anIonisation);
      pmanager->AddProcess(aMultipleScattering);
      // if( particleName ==  "pi+" ||
      //     particleName ==  "pi-" ||
      //     particleName ==  "kaon+" ||
      //     particleName ==  "kaon-" ||
      //     particleName ==  "proton" ||
      //     particleName ==  "anti_proton"){
      //   pmanager->AddProcess(new G4hPairProduction());
      //   pmanager->AddProcess(new G4hBremsstrahlung());
      // }

      /// pmanager->AddProcess(theUserCuts);

      //
      // set ordering for AtRestDoIt
      ////pmanager->SetProcessOrderingToFirst(theUserCuts,idxAtRest);

      //
      // set ordering for AlongStepDoIt
      pmanager->SetProcessOrdering(aMultipleScattering, idxAlongStep, 1);
      pmanager->SetProcessOrdering(anIonisation, idxAlongStep, 2);

      //
      // set ordering for PostStepDoIt
      pmanager->SetProcessOrdering(aMultipleScattering, idxPostStep, 1);
      pmanager->SetProcessOrdering(anIonisation, idxPostStep, 2);

      ////pmanager->SetProcessOrdering(theUserCuts,     idxPostStep,3);
    }
  }
}

// Hadronic Physics
// New using physics list QGSP_FTFP_BERT

void KM3Physics::ConstructHA() {
  G4HadronicProcessStore::Instance()->SetVerbose(0);
  // hadron elastic processes. from G4HadronElasticPhysics
  G4double elimitmaxall = 100.0 * PeV;

  // hadron physics (elastic)
  G4HadronElasticPhysics *theHadronElasticPhysics =
      new G4HadronElasticPhysics(2);  // verbose level
  theHadronElasticPhysics->ConstructProcess();

  // hadron physics (inelastic). from HadronPhysicsQGSP_FTFP_BERT builder
  G4HadronPhysicsQGSP_FTFP_BERT *theQGSP_FTFP_BERT =
      new G4HadronPhysicsQGSP_FTFP_BERT("hadron", true);
  theQGSP_FTFP_BERT->SetVerboseLevel(0);
  theQGSP_FTFP_BERT->ConstructProcess();

  // put elastic and inelastic models max energy to 100PeV only
  // for barions and mesons. since I dont want to dublicate the
  // QGSP_FTFP_BERT code here and since at this moment I don't have
  // access to HadronicProcessStore, the only way to change the upper
  // limit of the high energy models of many barions and mesons is to
  // access the Process through the particle manager and access the
  // highest energy interaction model of this process through the the
  // G4EnergyRangeManager
  // Update (moritz): this has changed with 4.10:
  // from: geant4.10.02.p01/source/processes/hadronic/management/History
  // > 15 May 2014 Vladimir Ivanchenko (hadr-man-V10-00-01)
  // > ---------------------------------------------------
  // > - G4HadronicProcess: added method  GetHadronicInteractionList() allowing
  // >     access to the list of registered models, removed obsolete access
  // >     methods to G4EnergyRangeManager
  // > - G4HadronicProcessStore - become G4ThreadLocalSingletone allowing
  // >     proper destruction of hadronic processes/models/x-sections
  // > - G4EnergyRangeManager - cleaned up; instead of c-array models pointers
  // >     are stored in stl vector
  // > - this tag requires remove deletion of Physics vectors from destructors
  // >     of all hadronic classes
  theParticleIterator->reset();
  while ((*theParticleIterator)()) {
    G4ParticleDefinition *particle = theParticleIterator->value();
    G4String particleName = particle->GetParticleName();
    if (particleName == "anti_lambda" || particleName == "anti_neutron" ||
        particleName == "anti_omega-" || particleName == "anti_proton" ||
        particleName == "anti_sigma+" || particleName == "anti_sigma-" ||
        particleName == "anti_xi-" || particleName == "anti_xi0" ||
        particleName == "kaon+" || particleName == "kaon-" ||
        particleName == "kaon0L" || particleName == "kaon0S" ||
        particleName == "lambda" || particleName == "neutron" ||
        particleName == "omega-" || particleName == "pi+" ||
        particleName == "pi-" || particleName == "proton" ||
        particleName == "sigma+" || particleName == "sigma-" ||
        particleName == "xi-" || particleName == "xi0") {
      G4ProcessManager *pmanager = particle->GetProcessManager();
      G4ProcessVector *aVector = pmanager->GetProcessList();
      G4int isi = aVector->size();
      for (G4int iii = 0; iii < isi; iii++) {
        G4VProcess *app = (*aVector)[iii];
        if (app->GetProcessSubType() == 121 ||
            app->GetProcessSubType() ==
                111) {  // is hadronic inelastic or elastic process
          G4HadronicProcess *aHp = (G4HadronicProcess *)(*aVector)[iii];
            // no longer needed with 4.10
          //G4EnergyRangeManager *aMan = aHp->GetManagerPointer();
          G4cout << aHp->GetProcessName() << G4endl;
          const G4MaterialTable *theMaterialTable =
              G4Material::GetMaterialTable();
          G4Material *aMaterial = NULL;
          const G4Element *anElement = NULL;
          for (size_t J = 0; J < theMaterialTable->size(); J++) {
            if ((*theMaterialTable)[J]->GetName() == G4String("Water")) {
              aMaterial = (*theMaterialTable)[J];
              anElement = aMaterial->GetElement(0);
            }
          }
          if (aMaterial == NULL || anElement == NULL) {
            G4Exception(
                "Material Water and one Element of it not found in ConstructHA",
                "", FatalException, "");
          } else {
            // new 4.10 shit
            std::vector<G4HadronicInteraction*> &hadInList =
              aHp->GetHadronicInteractionList();
            G4int n_inter = hadInList.size();
            for (G4int idx = 0; idx < n_inter; idx++) {
              G4HadronicInteraction *hadIn = hadInList[idx];
              // copied from energymanager code
              G4double kinEnergy = 99 * TeV;
              G4double low = hadIn->GetMinEnergy(aMaterial, anElement);
              G4double high = hadIn->GetMaxEnergy(aMaterial, anElement);
              if (low <= kinEnergy && high > kinEnergy) {
                hadIn->SetMaxEnergy(elimitmaxall);
                hadIn->SetVerboseLevel(0);
              }
            }
            // no longer needed with 4.10
            //G4HadronicInteraction *anIn =
            //    aMan->GetHadronicInteraction(99 * TeV, aMaterial, anElement);
            //anIn->SetMaxEnergy(elimitmaxall);
            //anIn->SetVerboseLevel(0);
          }
        }
      }
    }
  }
  // put inelastic models max energy to 100PeV only for barions and mesons

  // next add also inelastic processes for ions not included in
  // HadronPhysicsQGSP_FTFP_BERT builder
  // only low energy models are available
  // alpha particle
  G4AlphaInelasticProcess *theAlphaInelastic = new G4AlphaInelasticProcess();
  G4ProcessManager *pmanager = G4Alpha::Alpha()->GetProcessManager();

  // g4.10
  //G4LEAlphaInelastic *theLEAlphaModel = new G4LEAlphaInelastic();
  //theAlphaInelastic->RegisterMe(theLEAlphaModel);
  //pmanager->AddDiscreteProcess(theAlphaInelastic);

  // Deuteron
  G4DeuteronInelasticProcess *theDeuteronInelastic =
      new G4DeuteronInelasticProcess();
  pmanager = G4Deuteron::Deuteron()->GetProcessManager();

  // g4.10
  //G4LEDeuteronInelastic *theLEDeuteronModel = new G4LEDeuteronInelastic();
  //theDeuteronInelastic->RegisterMe(theLEDeuteronModel);
  //pmanager->AddDiscreteProcess(theDeuteronInelastic);
  //
  // Triton
  G4TritonInelasticProcess *theTritonInelastic = new G4TritonInelasticProcess();
  pmanager = G4Triton::Triton()->GetProcessManager();
  // g4.10
  //G4LETritonInelastic *theLETritonModel = new G4LETritonInelastic();
  //theTritonInelastic->RegisterMe(theLETritonModel);
  //pmanager->AddDiscreteProcess(theTritonInelastic);

  // next take into account the capture processes for pion and kaon minus
  // PionMinus
  // g4.10: pi/k absoroption now in G4HadronicAbsorptionBertini
  pmanager = G4PionMinus::PionMinus()->GetProcessManager();
  //pmanager->AddRestProcess(new G4PionMinusAbsorptionAtRest);
  pmanager->AddRestProcess(new G4HadronicAbsorptionBertini);

  // KaonMinus
  pmanager = G4KaonMinus::KaonMinus()->GetProcessManager();
  //pmanager->AddRestProcess(new G4KaonMinusAbsorption);
  pmanager->AddRestProcess(new G4HadronicAbsorptionBertini);
}

void KM3Physics::ConstructOP() {
  theCerenkovProcess = new KM3Cherenkov("KM3Cherenkov");
  G4OpAbsorption *theAbsorptionProcess = new G4OpAbsorption();

  theCerenkovProcess->DumpPhysicsTable();
  // theScintillationProcess->DumpPhysicsTable();
  // theAbsorptionProcess->DumpPhysicsTable();

  theCerenkovProcess->SetVerboseLevel(0);
  theCerenkovProcess->SetDetector(aDetector);
  theAbsorptionProcess->SetVerboseLevel(0);

  G4int MaxNumPhotons = -30000;

  theCerenkovProcess->SetTrackSecondariesFirst(true);
  theCerenkovProcess->SetMaxNumPhotonsPerStep(MaxNumPhotons);

  G4OpMie *theMieProcess = new G4OpMie();
  theMieProcess->SetVerboseLevel(0);

  theParticleIterator->reset();
  while ((*theParticleIterator)()) {
    G4ParticleDefinition *particle = theParticleIterator->value();
    G4ProcessManager *pmanager = particle->GetProcessManager();
    if (pmanager) {
      G4String particleName = particle->GetParticleName();
      if (theCerenkovProcess->IsApplicable(*particle)) {
        pmanager->AddProcess(theCerenkovProcess);
        pmanager->SetProcessOrdering(theCerenkovProcess, idxPostStep);
      }
      if (particleName == "opticalphoton") {
        pmanager->AddDiscreteProcess(theAbsorptionProcess);
        pmanager->AddDiscreteProcess(theMieProcess);
      }
    }
  }
  // set ordering for AtRestDoIt
}

void KM3Physics::ConstructGeneral() {
  // Add Decay Process
  G4Decay *theDecayProcess = new G4Decay();
  theParticleIterator->reset();
  while ((*theParticleIterator)()) {
    G4ParticleDefinition *particle = theParticleIterator->value();
    G4ProcessManager *pmanager = particle->GetProcessManager();
    if (pmanager) {
      if (theDecayProcess->IsApplicable(*particle)) {
        // set ordering for PostStepDoIt and AtRestDoIt. Muons do not decay but
        // are captured (dense medium). Only for Hadronic runs
        if (particle->GetParticleName() == "mu-") {
          // TODO: Do we want to use these processes:
          // this was only enables if WITHOUT Hadronic compile
          // if WITH hadr comp, do nothing
          /*
          pmanager->AddProcess(theDecayProcess);
          pmanager->SetProcessOrdering(theDecayProcess, idxPostStep);
          pmanager->SetProcessOrdering(theDecayProcess, idxAtRest);
          */
          ;
        } else {
          pmanager->AddProcess(theDecayProcess);
          pmanager->SetProcessOrdering(theDecayProcess, idxPostStep);
          // TODO: if hadronic compile:
          if ((particle->GetParticleName() != "pi-") &&
              (particle->GetParticleName() != "kaon-"))
          // end todo
            pmanager->SetProcessOrdering(theDecayProcess, idxAtRest);
        }
      }
    }
  }

  // Declare radioactive decay to the GenericIon in the IonTable.
  const G4IonTable *theIonTable =
      G4ParticleTable::GetParticleTable()->GetIonTable();
  G4RadioactiveDecay *theRadioactiveDecay = new G4RadioactiveDecay();

  for (G4int i = 0; i < theIonTable->Entries(); i++) {
    G4String particleName = theIonTable->GetParticle(i)->GetParticleName();
    G4String particleType = theIonTable->GetParticle(i)->GetParticleType();
    // G4cout << "*********************************"<< particleName << G4endl;
    if (particleName == "GenericIon") {
      G4ProcessManager *pmanager =
          theIonTable->GetParticle(i)->GetProcessManager();
      pmanager->SetVerboseLevel(0);
      pmanager->AddProcess(theRadioactiveDecay);
      pmanager->SetProcessOrdering(theRadioactiveDecay, idxPostStep);
      pmanager->SetProcessOrdering(theRadioactiveDecay, idxAtRest);
    }
  }
}
// THIS IS END OF TEST
// Where was the beginning?

void KM3Physics::SetCuts() {
  // G4VUserPhysicsList::SetCutsWithDefault method sets
  // the default cut value for all particle types
  //
  SetCutsWithDefault();
  /*
  SetCutValue(defaultCutValue,"gamma");
  SetCutValue(0.6*mm,"e-");
  SetCutValue(0.6*mm,"e+");
  SetCutValue(defaultCutValue,"mu-");
  SetCutValue(defaultCutValue,"mu+");
  SetCutValue(defaultCutValue,"proton");
  SetCutValue(defaultCutValue,"anti_proton");
  SetCutValue(100*m,"opticalphoton");
  SetCutValueForOthers(defaultCutValue);
  */
  /*
  SetCutValue(1.1*m,"gamma");
  SetCutValue(0.6*mm,"e-");
  SetCutValue(0.6*mm,"e+");
  SetCutValue(7.0*m,"mu-");
  SetCutValue(7.0*m,"mu+");
  SetCutValue(defaultCutValue,"proton");
  SetCutValue(defaultCutValue,"anti_proton");
  SetCutValueForOthers(defaultCutValue);
  */

  printf("dfdfdfdfdfdfdfdf\n");
  DumpCutValuesTable(5);    // b quark
  DumpCutValuesTable(22);   // gamma
  DumpCutValuesTable(-11);  // e+
  DumpCutValuesTable(11);   // e-
  DumpCutValuesTable(-13);  // mu+
  DumpCutValuesTable(13);   // mu-
  DumpCutValuesTable(0);    // opticalphoton
  printf("sssssssssssssssss\n");

  //  DumpCutValues();
}

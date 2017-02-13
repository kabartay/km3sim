#include "KM3Detector.h"
#include "KM3SD.h"
#include "KM3StackingAction.h"

#include "G4UnitsTable.hh"
#include "G4VUserDetectorConstruction.hh"

#include "globals.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4Element.hh"
#include "G4ElementTable.hh"
#include "G4Box.hh"
#include "G4Sphere.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4SDManager.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4UserLimits.hh"
#include "G4RegionStore.hh"
#include "G4VoxelLimits.hh"
#include "G4GDMLParser.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4GeometryManager.hh"
#include "CLHEP/Evaluator/Evaluator.h"

// detector MaxRho
// detectorCenter [0, 1] = [x, y]
// bottomposition
// detectorMaxz
// quantum_efficiency
// total_cathod_area

using CLHEP::atmosphere;
using CLHEP::bar;
using CLHEP::c_light;
using CLHEP::cm3;
using CLHEP::cm;
using CLHEP::deg;
using CLHEP::g;
using CLHEP::h_Planck;
using CLHEP::kelvin;
using CLHEP::kg;
using CLHEP::m;
using CLHEP::meter;
using CLHEP::mm;
using CLHEP::mole;
using CLHEP::pi;
using CLHEP::s;

KM3Detector::KM3Detector() {
  allCathods = new KM3Cathods();
  //allStoreys = new std::vector<StoreysPositions *>;
  //allOMs = new std::vector<OMPositions *>;
  //allTowers = new std::vector<TowersPositions *>;  // new towers
}

KM3Detector::~KM3Detector() {
  // newgeant  sxp.Finalize();
  delete allCathods;

  //for (size_t i = 0; i < allOMs->size(); i++) {
  //  (*allOMs)[i]->CathodsIDs->clear();
  //  delete (*allOMs)[i]->CathodsIDs;
  //  // free((*allOMs)[i]);
  //  delete (*allOMs)[i];
  //}
  //allOMs->clear();
  //delete allOMs;

  //for (size_t i = 0; i < allStoreys->size(); i++) {
  //  (*allStoreys)[i]->BenthosIDs->clear();
  //  delete (*allStoreys)[i]->BenthosIDs;
  //  // free((*allStoreys)[i]);
  //  delete (*allStoreys)[i];
  //}
  //allStoreys->clear();
  //delete allStoreys;

  //for (size_t i = 0; i < allTowers->size(); i++) {
  //  (*allTowers)[i]->BenthosIDs->clear();
  //  delete (*allTowers)[i]->BenthosIDs;
  //  // free((*allTowers)[i]);
  //  delete (*allTowers)[i];
  //}
  //allTowers->clear();
  //delete allTowers;
}

void KM3Detector::FindDetectorRadius() {
  G4double absdetectorRadius = 0.0;
  lowestStorey = 0.0;
  highestStorey = 0.0;
  outerStorey = 0.0;
  detectorMaxz = 0.0;
  detectorMaxRho = 0.0;
  detectorCenter = G4ThreeVector(0.0, 0.0, 0.0);

  //// TODO: derive from cathods
  //for (size_t isto = 0; isto < allStoreys->size(); isto++) {
  //  G4ThreeVector pos = (*(allStoreys))[isto]->position;
  //  G4double radius = (*(allStoreys))[isto]->radius;
  //  G4double dist = pos.mag() + radius;
  //  if (absdetectorRadius < dist) absdetectorRadius = dist;
  //  if (pos[2] < lowestStorey) lowestStorey = pos[2];
  //  if (pos[2] > highestStorey) highestStorey = pos[2];
  //  G4double distRho = sqrt(pos[0] * pos[0] + pos[1] * pos[1]) + radius;
  //  if (outerStorey < distRho) outerStorey = distRho;
  //}

  // have a small offset to account for the fact
  // that we take maximum positions of cathods, not the
  // storey/tower/om boundaries.
  // TODO: No idea if it's actually 1 meter, I estimated that from
  // the old gdml file:
  // 0.5 * [diameter(storeybox) + diameter(towerbox)] + radius(omsphere)
  // is approx 1 meter
  outerStorey = (*std::max_element(
        r_all.begin(), r_all.end()) + 1.0) * meter;
  highestStorey = (*std::max_element(
        z_all.begin(), z_all.end()) + 1.0) * meter;
  bottomPosition = (*std::min_element(
        z_all.begin(), z_all.end()) + 1.0) * meter;
  
  std::cout << "Min Z, Max Z, MaxAbsDist (on top): " << bottomPosition / m
    << ", " << highestStorey / m << ", " << MaxAbsDist / m << std::endl;

  detectorMaxRho = outerStorey + MaxAbsDist;
  detectorRadius = MaxAbsDist + absdetectorRadius;
  bottomPosition += lowestStorey;
  detectorMaxz = highestStorey + MaxAbsDist;
  std::cout << "Detector radius (m) and bottom position (m) " << detectorRadius / m
    << " " << bottomPosition / m << std::endl;

  // we don't actually need storeys/towers for this
  MyGenerator->PutFromDetector(detectorCenter, detectorMaxRho, detectorMaxz,
      bottomPosition);
}

void KM3Detector::SetUpVariables() {
  std::FILE *infile;
  G4double MaxRelDist;
  NUMENTRIES = -10;
  NUMENTRIES_ANGLEACC = -10;

  // Mie, water etc parameters
  if ((infile = std::fopen(Parameter_File.c_str(), "r")) == NULL) {
    G4Exception("Error open input parameter file\n", "", FatalException, "");
  } else {
    char varname[50];
    char expression[50];
    G4int readvalues[18] = {18 * 0};
    G4String String0 = G4String("detectorDepth");
    G4String String1 = G4String("bottomPosition");
    G4String String2 = G4String("PPCKOV");
    G4String String3 = G4String("RINDEX_WATER");
    G4String String4 = G4String("ABSORPTION_WATER");
    G4String String5 = G4String("RINDEX_GLASS");
    G4String String6 = G4String("ABSORPTION_GLASS");
    G4String String7 = G4String("RINDEX_GELL");
    G4String String8 = G4String("ABSORPTION_GELL");
    G4String String9 = G4String("RINDEX_AIR");
    G4String String10 = G4String("ABSORPTION_AIR");
    G4String String11 = G4String("RINDEX_CATH");
    G4String String12 = G4String("ABSORPTION_CATH");
    G4String String13 = G4String("Q_EFF");
    G4String String14 = G4String("MIE_WATER");
    G4String String15 = G4String("MIE_MODEL");
    G4String String16 = G4String("COS_ANGLES");
    G4String String17 = G4String("ANG_ACCEPT");

    HepTool::Evaluator fCalc;
    fCalc.setSystemOfUnits(
        1.e+3, 1. / 1.60217733e-25, 1.e+9, 1. / 1.60217733e-10, 1.0, 1.0,
        1.0);  // asign default variables to evaluator (Geant4 Units)
    while (fscanf(infile, "%s", varname) != EOF) {
      G4String thename = G4String(varname);
      if (thename == String0) {
        readvalues[0] = 1;
        fscanf(infile, "%s\n", expression);
        // this should be the detector depth at the center of the
        // detector. It is used to calculate the seawater density using
        // the compressibility of the sea water (see below)
        detectorDepth = fCalc.evaluate(expression);
      } else if (thename == String1) {
        readvalues[1] = 1;
        fscanf(infile, "%s\n", expression);
        // this is the position of the bottom of the sea relative to detector
        // center
        bottomPosition = fCalc.evaluate(expression);
      } else if (thename == String2) {
        readvalues[2] = 1;
        fscanf(infile, "%s", expression);
        NUMENTRIES = (G4int)fCalc.evaluate(expression);
        for (G4int i = 0; i < NUMENTRIES - 1; i++) {
          fscanf(infile, "%s", expression);
          PPCKOV[i] = h_Planck * c_light / fCalc.evaluate(expression);
        }
        fscanf(infile, "%s\n", expression);
        PPCKOV[NUMENTRIES - 1] =
          h_Planck * c_light / fCalc.evaluate(expression);
      } else if (thename == String3) {
        readvalues[3] = 1;
        if (NUMENTRIES < 0)
          G4Exception(
              "Wavelengths of Optical Photons must be set before "
              "setting any other Optical Property",
              "", FatalException, "");
        for (G4int i = 0; i < NUMENTRIES - 1; i++) {
          fscanf(infile, "%s", expression);
          RINDEX_WATER[i] = fCalc.evaluate(expression);
        }
        fscanf(infile, "%s\n", expression);
        RINDEX_WATER[NUMENTRIES - 1] = fCalc.evaluate(expression);
      } else if (thename == String4) {
        readvalues[4] = 1;
        if (NUMENTRIES < 0)
          G4Exception(
              "Wavelengths of Optical Photons must be set before "
              "setting any other Optical Property",
              "", FatalException, "");
        fscanf(infile, "%s", expression);
        Water_Transparency = fCalc.evaluate(expression);
        fscanf(infile, "%s", expression);
        MaxRelDist = fCalc.evaluate(expression);  // How many absorption lengths
        // optical photons can cross
        for (G4int i = 0; i < NUMENTRIES - 1; i++) {
          fscanf(infile, "%s", expression);
          ABSORPTION_WATER[i] = Water_Transparency / fCalc.evaluate(expression);
        }
        fscanf(infile, "%s\n", expression);
        ABSORPTION_WATER[NUMENTRIES - 1] =
          Water_Transparency / fCalc.evaluate(expression);
      } else if (thename == String5) {
        readvalues[5] = 1;
        if (NUMENTRIES < 0)
          G4Exception(
              "Wavelengths of Optical Photons must be set before "
              "setting any other Optical Property",
              "", FatalException, "");
        for (G4int i = 0; i < NUMENTRIES - 1; i++) {
          fscanf(infile, "%s", expression);
          RINDEX_GLASS[i] = fCalc.evaluate(expression);
        }
        fscanf(infile, "%s\n", expression);
        RINDEX_GLASS[NUMENTRIES - 1] = fCalc.evaluate(expression);
      } else if (thename == String6) {
        readvalues[6] = 1;
        if (NUMENTRIES < 0)
          G4Exception(
              "Wavelengths of Optical Photons must be set before "
              "setting any other Optical Property",
              "", FatalException, "");
        for (G4int i = 0; i < NUMENTRIES - 1; i++) {
          fscanf(infile, "%s", expression);
          ABSORPTION_GLASS[i] = fCalc.evaluate(expression);
        }
        fscanf(infile, "%s\n", expression);
        ABSORPTION_GLASS[NUMENTRIES - 1] = fCalc.evaluate(expression);
      } else if (thename == String7) {
        readvalues[7] = 1;
        if (NUMENTRIES < 0)
          G4Exception(
              "Wavelengths of Optical Photons must be set before "
              "setting any other Optical Property",
              "", FatalException, "");
        for (G4int i = 0; i < NUMENTRIES - 1; i++) {
          fscanf(infile, "%s", expression);
          RINDEX_GELL[i] = fCalc.evaluate(expression);
        }
        fscanf(infile, "%s\n", expression);
        RINDEX_GELL[NUMENTRIES - 1] = fCalc.evaluate(expression);
      } else if (thename == String8) {
        readvalues[8] = 1;
        if (NUMENTRIES < 0)
          G4Exception(
              "Wavelengths of Optical Photons must be set before "
              "setting any other Optical Property",
              "", FatalException, "");
        for (G4int i = 0; i < NUMENTRIES - 1; i++) {
          fscanf(infile, "%s", expression);
          ABSORPTION_GELL[i] = fCalc.evaluate(expression);
        }
        fscanf(infile, "%s\n", expression);
        ABSORPTION_GELL[NUMENTRIES - 1] = fCalc.evaluate(expression);
      } else if (thename == String9) {
        readvalues[9] = 1;
        if (NUMENTRIES < 0)
          G4Exception(
              "Wavelengths of Optical Photons must be set before "
              "setting any other Optical Property",
              "", FatalException, "");
        for (G4int i = 0; i < NUMENTRIES - 1; i++) {
          fscanf(infile, "%s", expression);
          RINDEX_AIR[i] = fCalc.evaluate(expression);
        }
        fscanf(infile, "%s\n", expression);
        RINDEX_AIR[NUMENTRIES - 1] = fCalc.evaluate(expression);
      } else if (thename == String10) {
        readvalues[10] = 1;
        if (NUMENTRIES < 0)
          G4Exception(
              "Wavelengths of Optical Photons must be set before "
              "setting any other Optical Property",
              "", FatalException, "");
        for (G4int i = 0; i < NUMENTRIES - 1; i++) {
          fscanf(infile, "%s", expression);
          ABSORPTION_AIR[i] = fCalc.evaluate(expression);
        }
        fscanf(infile, "%s\n", expression);
        ABSORPTION_AIR[NUMENTRIES - 1] = fCalc.evaluate(expression);
      } else if (thename == String11) {
        readvalues[11] = 1;
        if (NUMENTRIES < 0)
          G4Exception(
              "Wavelengths of Optical Photons must be set before "
              "setting any other Optical Property",
              "", FatalException, "");
        for (G4int i = 0; i < NUMENTRIES - 1; i++) {
          fscanf(infile, "%s", expression);
          RINDEX_CATH[i] = fCalc.evaluate(expression);
        }
        fscanf(infile, "%s\n", expression);
        RINDEX_CATH[NUMENTRIES - 1] = fCalc.evaluate(expression);
      } else if (thename == String12) {
        readvalues[12] = 1;
        if (NUMENTRIES < 0)
          G4Exception(
              "Wavelengths of Optical Photons must be set before "
              "setting any other Optical Property",
              "", FatalException, "");
        for (G4int i = 0; i < NUMENTRIES - 1; i++) {
          fscanf(infile, "%s", expression);
          ABSORPTION_CATH[i] = fCalc.evaluate(expression);
        }
        fscanf(infile, "%s\n", expression);
        ABSORPTION_CATH[NUMENTRIES - 1] = fCalc.evaluate(expression);
      } else if (thename == String13) {
        readvalues[13] = 1;
        if (NUMENTRIES < 0)
          G4Exception(
              "Wavelengths of Optical Photons must be set before "
              "setting any other Optical Property (PPCKOV keyword)",
              "", FatalException, "");
        fscanf(infile, "%s", expression);
        Quantum_Efficiency = fCalc.evaluate(
            expression);  // maximum quantum efficienscy, used in KM3Cherenkov
        for (G4int i = 0; i < NUMENTRIES - 1; i++) {
          fscanf(infile, "%s", expression);
          Q_EFF[i] = fCalc.evaluate(expression);
        }
        fscanf(infile, "%s\n", expression);
        Q_EFF[NUMENTRIES - 1] = fCalc.evaluate(expression);
      } else if (thename == String14) {
        readvalues[14] = 1;
        if (NUMENTRIES < 0)
          G4Exception(
              "Wavelengths of Optical Photons must be set before "
              "setting any other Optical Property (PPCKOV keyword)",
              "", FatalException, "");
        for (G4int i = 0; i < NUMENTRIES - 1; i++) {
          fscanf(infile, "%s", expression);
          SCATTER_WATER[i] = fCalc.evaluate(expression);
        }
        fscanf(infile, "%s\n", expression);
        SCATTER_WATER[NUMENTRIES - 1] = fCalc.evaluate(expression);
      } else if (thename == String15) {
        readvalues[15] = 1;
        fscanf(infile, "%s\n", expression);
        // Mie model as of mie phase factors file
        MieModel = fCalc.evaluate(expression);
      } else if (thename == String16) {
        readvalues[16] = 1;
        fscanf(infile, "%s", expression);
        NUMENTRIES_ANGLEACC = (G4int)fCalc.evaluate(expression);
        for (G4int i = 0; i < NUMENTRIES_ANGLEACC - 1; i++) {
          fscanf(infile, "%s", expression);
          COSANGLES[i] = fCalc.evaluate(expression);
        }
        fscanf(infile, "%s\n", expression);
        COSANGLES[NUMENTRIES_ANGLEACC - 1] = fCalc.evaluate(expression);
      } else if (thename = String17) {
        readvalues[17] = 1;
        if (NUMENTRIES_ANGLEACC < 0)
          G4Exception(
              "Cosine angles values must be set before setting angular "
              "acceptance values",
              "", FatalException, "");
        for (G4int i = 0; i < NUMENTRIES_ANGLEACC - 1; i++) {
          fscanf(infile, "%s", expression);
          ACCEPTANCE[i] = fCalc.evaluate(expression);
        }
        fscanf(infile, "%s\n", expression);
        ACCEPTANCE[NUMENTRIES_ANGLEACC - 1] = fCalc.evaluate(expression);
      } else {
        G4Exception("Not a keyword I can recognize\n", "", FatalException, "");
      }
    }
    fclose(infile);

    for (G4int i = 0; i < 18; i++) {
      if (readvalues[i] == 0) {
        switch (i) {
          case 0:
            G4Exception("detectorDepth not set", "", FatalException, "");
            break;
          case 1:
            G4Exception("bottomPosition not set", "", FatalException, "");
            break;
          case 2:
            G4Exception("PPCKOV not set", "", FatalException, "");
            break;
          case 3:
            G4Exception("RINDEX_WATER not set", "", FatalException, "");
            break;
          case 4:
            G4Exception("ABSORPTION_WATER not set", "", FatalException, "");
            break;
          case 5:
            G4Exception("RINDEX_GLASS not set", "", FatalException, "");
            break;
          case 6:
            G4Exception("ABSORPTION_GLASS not set", "", FatalException, "");
            break;
          case 7:
            G4Exception("RINDEX_GELL not set", "", FatalException, "");
            break;
          case 8:
            G4Exception("ABSORPTION_GELL not set", "", FatalException, "");
            break;
          case 9:
            G4Exception("RINDEX_AIR not set", "", FatalException, "");
            break;
          case 10:
            G4Exception("ABSORPTION_AIR not set", "", FatalException, "");
            break;
          case 11:
            G4Exception("RINDEX_CATH not set", "", FatalException, "");
            break;
          case 12:
            G4Exception("ABSORPTION_CATH not set", "", FatalException, "");
            break;
          case 13:
            G4Exception("Q_EFF not set", "", FatalException, "");
            break;
          case 14:
            G4Exception("MIE_WATER not set", "", FatalException, "");
            break;
          case 15:
            G4Exception("MIE_MODEL not set", "", FatalException, "");
            break;
          case 16:
            G4Exception("COS_ANGLES not set", "", FatalException, "");
            break;
          case 17:
            G4Exception("ANG_ACCEPT not set", "", FatalException, "");
            break;
        }
      }
    }
  }
  MaxAbsDist = MaxRelDist * Water_Transparency;
}

void KM3Detector::ConstructMaterials() {
  // All Basic Elements
  // --------------------------------------------------------------------------
  G4double weightH = 1.007940 * g / mole;
  G4double weightO = 15.999400 * g / mole;
  G4double weightCl = 35.453000 * g / mole;
  G4double weightMg = 24.305000 * g / mole;
  G4double weightNa = 22.989770 * g / mole;
  G4double weightSi = 28.085500 * g / mole;
  G4double weightB = 10.811000 * g / mole;
  G4double weightAl = 26.981538 * g / mole;
  G4double weightC = 12.010700 * g / mole;
  G4double weightN = 14.006700 * g / mole;
  G4double weightCa = 40.078000 * g / mole;
  G4double weightK = 39.098300 * g / mole;
  G4double weightS = 32.065000 * g / mole;
  G4double weightFe = 55.845000 * g / mole;
  G4Element *elementH = new G4Element("Hydrogen", "H", 1., weightH);
  G4Element *elementO = new G4Element("Oxygen", "O", 8., weightO);
  G4Element *elementCl = new G4Element("Chlorine", "Cl", 17., weightCl);
  G4Element *elementMg = new G4Element("Magnisium", "Mg", 12., weightMg);
  G4Element *elementNa = new G4Element("Sodium", "Na", 11., weightNa);
  G4Element *elementSi = new G4Element("Silicon", "Si", 14., weightSi);
  G4Element *elementB = new G4Element("Boron", "B", 5., weightB);
  G4Element *elementAl = new G4Element("Aluminium", "Al", 13., weightAl);
  G4Element *elementC = new G4Element("Carbon", "C", 6., weightC);
  G4Element *elementN = new G4Element("Nitrogen", "N", 7., weightN);
  G4Element *elementCa = new G4Element("Calcium", "Ca", 20., weightCa);
  G4Element *elementK = new G4Element("Potassium", "K", 19., weightK);
  G4Element *elementS = new G4Element("Sulphur", "S", 16., weightS);
  G4Element *elementFe = new G4Element("Iron", "Fe", 26., weightFe);

  // Earths Crust
  //G4Material *Crust = new G4Material("Crust", 2.6 * g / cm3, 8, kStateSolid,
  //    287.15 * kelvin, 1.0 * atmosphere);
  Crust = new G4Material("Crust", 2.6 * g / cm3, 8, kStateSolid,
      287.15 * kelvin, 1.0 * atmosphere);
  Crust->AddElement(elementO, 0.481);
  Crust->AddElement(elementSi, 0.277);
  Crust->AddElement(elementAl, 0.081);
  Crust->AddElement(elementFe, 0.050);
  Crust->AddElement(elementCa, 0.036);
  Crust->AddElement(elementNa, 0.028);
  Crust->AddElement(elementK, 0.026);
  Crust->AddElement(elementMg, 0.021);

  // WATER---here is added the depedence of the salinity and density on the
  // depth (UNESCO)--
  // At the surface the seawater density is approximately 1.029gr/cm3
  // (E.G.Anassontzis, "The NESTOR Site",
  // Proceedings of the 3rd NESTOR International Workshop, October 19-21, 1993,
  // Pylos Greece, pp614-630)
  // based on the composition of ocean seawater for 35o/oo
  // www.soest.hawaii.edu/oceanography we have
  G4double abundanceNa = 469.0e-3 * mole / kg;
  G4double abundanceMg = 52.8e-3 * mole / kg;
  G4double abundanceCa = 10.3e-3 * mole / kg;
  G4double abundanceK = 10.2e-3 * mole / kg;
  G4double abundanceCl = 545.9e-3 * mole / kg;
  G4double abundanceSO4 = 28.2e-3 * mole / kg;
  G4double abundanceHCO3 = 2.33e-3 * mole / kg;
  // The Mediterranean Salinity is about 40o/oo (gr/kgr of seawater)
  // we scale these abundances for 40o/oo salinity
  G4double scale = 40.0 / 35.0;
  abundanceNa *= scale;
  abundanceMg *= scale;
  abundanceCa *= scale;
  abundanceK *= scale;
  abundanceCl *= scale;
  abundanceSO4 *= scale;
  abundanceHCO3 *= scale;
  // we convert to gr/kgr of the solution (seawater) for each element or
  // composite
  abundanceNa *= weightNa;
  abundanceMg *= weightMg;
  abundanceCa *= weightCa;
  abundanceK *= weightK;
  abundanceCl *= weightCl;
  abundanceSO4 *= (weightS + 4 * weightO);
  abundanceHCO3 *= (weightH + weightC + 3 * weightO);
  // the composites of sea water
  G4double abundanceH2O =
    1.0 - (abundanceNa + abundanceMg + abundanceCa + abundanceK +
        abundanceCl + abundanceSO4 + abundanceHCO3);
  // if the material is not used to fill a specific logical volume the density
  // is not used
  G4Material *H2O = new G4Material("H2O", 1.0 * g / cm3, 2);
  H2O->AddElement(elementH, 2);
  H2O->AddElement(elementO, 1);
  G4Material *SO4 = new G4Material("SO4", 1.0 * g / cm3, 2);
  SO4->AddElement(elementS, 1);
  SO4->AddElement(elementO, 4);
  G4Material *HCO3 = new G4Material("HCO3", 1.0 * g / cm3, 3);
  HCO3->AddElement(elementH, 1);
  HCO3->AddElement(elementC, 1);
  HCO3->AddElement(elementO, 3);
  // calculate the density of sea water using the compressibility (Apostolis
  // Thesis appendix C)
  // and the detector depth (in meters : not water equivalent)
  G4double Compressibility = 4.29e-5 / bar;
  G4double gravity = 9.8 * m / (s * s);
  G4double surfaceDensity = 1.029 * g / cm3;
  G4double seawaterDensity =
    surfaceDensity *
    exp(gravity * surfaceDensity * Compressibility * detectorDepth);
  //-----------------------------------------------------------------
  Water = new G4Material("Water", seawaterDensity, 8, kStateLiquid,
      287.15 * kelvin, 1.0 * atmosphere);
  //G4Material *Water = new G4Material("Water", seawaterDensity, 8, kStateLiquid,
  //    287.15 * kelvin, 1.0 * atmosphere);
  Water->AddMaterial(H2O, abundanceH2O);
  Water->AddMaterial(SO4, abundanceSO4);
  Water->AddMaterial(HCO3, abundanceHCO3);
  Water->AddElement(elementCl, abundanceCl);
  Water->AddElement(elementMg, abundanceMg);
  Water->AddElement(elementNa, abundanceNa);
  Water->AddElement(elementCa, abundanceCa);
  Water->AddElement(elementK, abundanceK);

  // GLASS and GELL change (27/5/2005) of the composition of borosilicate glass
  // according to
  // US Standards (http://www.udel.edu/chem/GlassShop/PhysicalProperties.htm)
  // 80.6% SiO2, 13.0% B2O3, 4.0% Na2O, 2.4% Al2O3
  G4Material *materialSiO2 = new G4Material("SiO2", 1.0 * g / cm3, 2);
  materialSiO2->AddElement(elementSi, 1);
  materialSiO2->AddElement(elementO, 2);
  G4Material *materialB2O3 = new G4Material("B2O3", 1.0 * g / cm3, 2);
  materialB2O3->AddElement(elementB, 2);
  materialB2O3->AddElement(elementO, 3);
  G4Material *materialNa2O = new G4Material("Na2O", 1.0 * g / cm3, 2);
  materialNa2O->AddElement(elementNa, 2);
  materialNa2O->AddElement(elementO, 1);
  G4Material *materialAl2O3 = new G4Material("Al2O3", 1.0 * g / cm3, 2);
  materialAl2O3->AddElement(elementAl, 2);
  materialAl2O3->AddElement(elementO, 3);
  G4Material *Glass = new G4Material("Glass", 2.23 * g / cm3, 4, kStateSolid,
      287.15 * kelvin, 1.0 * atmosphere);
  Glass->AddMaterial(materialSiO2, 0.806);
  Glass->AddMaterial(materialB2O3, 0.130);
  Glass->AddMaterial(materialNa2O, 0.040);
  Glass->AddMaterial(materialAl2O3, 0.024);

  // Silicone Gel Material.
  // density is 0.97g/cm3
  // (http://www.wackersilicones.com/documents/techdatasheets/silgel612.pdf)
  // Polydimethylsiloxane polymeres -- (C2H6OSi)n
  G4Material *Gell = new G4Material("Gell", 0.97 * g / cm3, 4, kStateSolid,
      287.15 * kelvin, 1.0 * atmosphere);
  Gell->AddElement(elementC, 2);
  Gell->AddElement(elementH, 6);
  Gell->AddElement(elementO, 1);
  Gell->AddElement(elementSi, 1);

  // AIR
  G4Material *Air = new G4Material("Air", 1.29e-03 * g / cm3, 2);
  Air->AddElement(elementN, .7);
  Air->AddElement(elementO, .3);

  // CATHOD
  //G4Material *Cathod =
  //  new G4Material("Cathod", 22, 47.867 * g / mole, 4.507 * g / cm3,
  //      kStateSolid, 287.15 * kelvin, 1.0 * atmosphere);
  Cathod = new G4Material("Cathod", 22, 47.867 * g / mole, 4.507 * g / cm3,
      kStateSolid, 287.15 * kelvin, 1.0 * atmosphere);

  // Set OPTICAL PROPERTIES (read from file) of materials

  // WATER
  G4MaterialPropertiesTable *Properties_Water = new G4MaterialPropertiesTable();
  Properties_Water->AddProperty("RINDEX", PPCKOV, RINDEX_WATER, NUMENTRIES);
  Properties_Water->AddProperty("ABSLENGTH", PPCKOV, ABSORPTION_WATER,
      NUMENTRIES);
  Properties_Water->AddProperty("MIELENGTH", PPCKOV, SCATTER_WATER, NUMENTRIES);
  Properties_Water->AddConstProperty("MIEPHASE", MieModel);
  Water->SetMaterialPropertiesTable(Properties_Water);

  // GLASS    -----------------------------------------------------------
  G4MaterialPropertiesTable *Properties_Glass = new G4MaterialPropertiesTable();
  Properties_Glass->AddProperty("ABSLENGTH", PPCKOV, ABSORPTION_GLASS,
      NUMENTRIES);
  Properties_Glass->AddProperty("RINDEX", PPCKOV, RINDEX_GLASS, NUMENTRIES);
  Glass->SetMaterialPropertiesTable(Properties_Glass);

  // GELL      -----------------------------------------------------------
  G4MaterialPropertiesTable *Properties_Gell = new G4MaterialPropertiesTable();
  Properties_Gell->AddProperty("ABSLENGTH", PPCKOV, ABSORPTION_GELL,
      NUMENTRIES);
  Properties_Gell->AddProperty("RINDEX", PPCKOV, RINDEX_GELL, NUMENTRIES);
  Gell->SetMaterialPropertiesTable(Properties_Gell);

  // AIR       -----------------------------------------------------------
  // absorption length of air set to 1 meter to prevent trapping of a photon
  // inside air material
  G4MaterialPropertiesTable *Properties_Air = new G4MaterialPropertiesTable();
  Properties_Air->AddProperty("ABSLENGTH", PPCKOV, ABSORPTION_AIR, NUMENTRIES);
  Properties_Air->AddProperty("RINDEX", PPCKOV, RINDEX_AIR, NUMENTRIES);
  Air->SetMaterialPropertiesTable(Properties_Air);

  // CATHOD       ----------------------------------------------------------
  G4MaterialPropertiesTable *Properties_Cath = new G4MaterialPropertiesTable();
  Properties_Cath->AddProperty("ABSLENGTH", PPCKOV, ABSORPTION_CATH,
      NUMENTRIES);
  Properties_Cath->AddProperty("RINDEX", PPCKOV, RINDEX_CATH, NUMENTRIES);
  Properties_Cath->AddProperty("Q_EFF", PPCKOV, Q_EFF, NUMENTRIES);
  Properties_Cath->AddProperty("ANGULAR_ACCEPTANCE", COSANGLES, ACCEPTANCE,
      NUMENTRIES_ANGLEACC);
  Cathod->SetMaterialPropertiesTable(Properties_Cath);
}


G4VPhysicalVolume *KM3Detector::SetSensitiveCathods(G4VPhysicalVolume *pvol, 
    G4VSensitiveDetector *sd) {
  int n_cath = 0;
  int n_daughters = pvol->GetLogicalVolume()->GetNoDaughters();
  for (G4int i=0; i < n_daughters; i++) {
    G4VPhysicalVolume *daughter = pvol->GetLogicalVolume()->GetDaughter(i);
    if ((daughter->GetName()).contains("CathodVolume")) {
      //std::cout << "Got Cathod, setting SD..." << i << std::endl;
      daughter->GetLogicalVolume()->SetSensitiveDetector(sd);
    }
  }
  return pvol;
}
  


G4int KM3Detector::TotalPMTEntities(const G4VPhysicalVolume *pvol) {
  int n_cath = 0;
  int n_daughters = pvol->GetLogicalVolume()->GetNoDaughters();
  std::cout << "N Daughter..." << n_daughters << std::endl;
  for (G4int i=0; i < n_daughters; i++) {
    G4VPhysicalVolume *daughter = pvol->GetLogicalVolume()->GetDaughter(i);
    if ((daughter->GetName()).contains("CathodVolume")) {
      n_cath++;
    }
  }
  return n_cath;
}



G4VPhysicalVolume *KM3Detector::Construct() {
  SetUpVariables();
  ConstructMaterials();
  G4GeometryManager::GetInstance()->SetWorldMaximumExtent(1000.0 * m);

  //G4GDMLParser parser;
  //parser.Read(Geometry_File);
  //fWorld = parser.GetWorldVolume();
  std::cout << "Build the World..." << std::endl;
  fWorld = ConstructWorldVolume(Geometry_File);

  if (fWorld == 0) {
    G4Exception(
        "World volume not set properly check your setup selection "
        "criteria or GDML input!",
        "", FatalException, ""); 
  }

  std::cout << "Count Cathods..." << std::endl;
  std::cout << "Total Cathods " << TotalPMTEntities(fWorld) << std::endl;

  //------------------------------------------------
  // Sensitive detectors
  //------------------------------------------------
  std::cout << "Define Sensitive Detector... " << std::endl;

  G4SDManager *SDman = G4SDManager::GetSDMpointer();
  G4String MySDname = "mydetector1/MySD";
  KM3SD *aMySD = new KM3SD(MySDname);
  aMySD->SetVerboseLevel(1);
  aMySD->myStDetector = this;
  SDman->AddNewDetector(aMySD);

  // next find the Cathod && Dead logical volumes and assign them the sensitive
  // detectors
  // "Deadvolume is obsolete" anyways...
  std::cout << "Assign Cathods as Sensitive areas... " << std::endl;
  G4LogicalVolume *aLogicalVolume;
  std::vector<G4LogicalVolume *> *aLogicalStore;
  G4String cathVol("CathodVolume");
  G4String deadVol("DeadVolume");
  size_t theSize = G4LogicalVolumeStore::GetInstance()->size();
  std::cout << "Volume Store size (# of logical volumes): " << theSize << std::endl;
  aLogicalStore = G4LogicalVolumeStore::GetInstance();
  for (size_t i = 0; i < theSize; i++) {
    aLogicalVolume = (*aLogicalStore)[i];
    std::cout << aLogicalVolume->GetName() << std::endl;

    //if (((aLogicalVolume->GetName()).contains(cathVol)) ||
    //    ((aLogicalVolume->GetName()).contains(deadVol))) 
    if( (aLogicalVolume->GetName() == cathVol) ||
        (aLogicalVolume->GetName() == deadVol) ) {
      aLogicalVolume->SetSensitiveDetector(aMySD);
      std::cout << "Found Cathvol!" << std::endl;
    }
  }

  fWorld = SetSensitiveCathods(fWorld, aMySD);

  aMySD->PrintAll();

  //tempotest
  G4VPhysicalVolume* aPhysicalVolume;
  std::vector<G4VPhysicalVolume*> *aPhysicalStore;
  theSize=G4PhysicalVolumeStore::GetInstance()->size();
  aPhysicalStore = G4PhysicalVolumeStore::GetInstance();
  for(size_t i=0 ; i<theSize ; i++){
    aPhysicalVolume = (*aPhysicalStore)[i];
    //std::cout <<  aPhysicalVolume->GetName() <<" "<<aPhysicalVolume->GetMultiplicity()<<" "<<aPhysicalVolume->GetCopyNo()<< std::endl;
  }
  //tempotest

  //fully adjustable benthos and storey linked list
  std::cout <<"Total World Volume Entities= " << fWorld->GetLogicalVolume()->TotalVolumeEntities() <<std::endl;


  // find detector radius and detector center from the Storeys
  std::cout << "Compute the KM3Sim Can... " << std::endl;
  FindDetectorRadius();

  //--------Write the header of the outfile and the Cathods Position, Direction
  // and History Tree
  std::cout << "Write outfile header... " << std::endl;
  G4int nnn0 = 0;
  G4int nnn1 = 1;
  G4int nben = allCathods->GetNumberOfCathods();

  // here in case of antares evt format I should write something general about
  // simulation
  TheEVTtoWrite->ReadRunHeader();

  std::FILE *oofile = std::fopen("PmtPositionsAndDirections", "w");
  fprintf(oofile, "%d %f\n", nben, Quantum_Efficiency);
  allCathods->PrintAllCathods(oofile);
  fclose(oofile);

  TheEVTtoWrite->WriteRunHeader();

  // find the total photocathod area on a OM
  std::cout << "Compute total photocathod area... " << std::endl;
  //G4int CaPerOM = (*allOMs)[0]->CathodsIDs->size();
  G4int CaPerOM = 31;
  TotCathodArea =
    CaPerOM * pi * allCathods->GetCathodRadius(0) *
    allCathods->GetCathodRadius(0);
  std::cout << TotCathodArea  << std::endl;
  // this is valid only if at simulation level (not EM or HA param)
  // all cathods have the same radius. Easy to change to account for a
  // detector with varius cathod types

  // return the physical World
  return fWorld;
}


G4VPhysicalVolume* KM3Detector::ConstructWorldVolume(const std::string &detxFile) {
  // Parse according to
  // http://wiki.km3net.de/index.php/Dataformats#Detector_Description_.28.detx.29

  // there are 3 different materials:
  //  Water
  //  Crust
  //  Cathod (composite)
  std::cout << "Define Volumes..." << std::endl;
  G4Box *worldBox = new G4Box("WorldBox",
      1100 * meter, 1100 * meter, 1100 * meter);
  G4LogicalVolume *worldLog = new G4LogicalVolume(worldBox,
      Water, "World");
  G4VPhysicalVolume *worldPV = new G4PVPlacement(
      0,                // rotation
      G4ThreeVector(),  // translation
      "World",
      worldLog,         // assoc. logical volume
      0,                // no mother
      true,            // MANY
      887766554433);              // cpNR

  G4Box *crustBox = new G4Box("CrustBox",
      1100 * meter, 1100 * meter, (984.7 / 2) * meter);
  G4LogicalVolume *crustLog = new G4LogicalVolume(crustBox,
      Crust, "Crust");
  G4VPhysicalVolume *crustPV = new G4PVPlacement(
      0,
      G4ThreeVector(0, 0, -607.65),
      crustLog,
      "Crust",
      worldLog,
      true,
      99887766);

  //G4LogicalVolume *towerLog = new G4LogicalVolume(towerBox,
  //    Water, "TowerVolume");
  //G4LogicalVolume *storeyLog = new G4LogicalVolume(storeyBox,
  //    Water, "StoreyVolume");
  //G4LogicalVolume *omLog = new G4LogicalVolume(omSphere,
  //    Water, "OMVolume");
  //G4Box *towerBox = new G4Box("TowerBox",
  //    1 * meter, 1 * meter, 170.0 * meter);
  //G4Box *storeyBox = new G4Box("StoreyBox",
  //    0.6 * meter, 0.6 * meter, 0.6 * meter);
  // G4Sphere *omSphere = new G4Sphere("OMSphere",
  //     0.0 * cm, 21.6 * cm, 0.0 * deg, 360.0 * deg, 0.0 * deg, 180.0 * deg);


  std::cout << "Define Cathods..." << std::endl;
  // create cathod volumes
  G4Tubs *cathodTube = new G4Tubs("CathodTube", 0.0 * cm, 4.7462 * cm, 0.5 * cm, 0.0 * deg, 360.0 * deg);
  G4LogicalVolume *cathodLog = new G4LogicalVolume(cathodTube, Cathod, "CathodVolume");

  // DETX V2 FORMA-
  // --------------
  //
  // global_det_id format_version\n
  //  
  // UTC_validity_from UTC_validity_to \n
  // 
  // UTM_ref_grid UTM_ref_easting UTM_ref_northing UTM_ref_z \n
  // 
  // ndoms \n
  // 
  // dom_id line_id floor_id npmts \n
  // 
  //  pmt_id_global x y z dx dy dz t0 \n
  // 
  //  pmt_id_global x y z dx dy dz t0 \n 
  // 
  //  ... 
  // 
  //  pmt_id_global x y z dx dy dz t0 \n
  //  
  // #repeat for each dom
  
  std::cout << "Parse DETX..." << std::endl;
  std::ifstream infile(detxFile);
  std::string line;
  std::istringstream iss(line);

  int global_det_id, format_version;
  infile >> global_det_id >> format_version;
  std::cout << "Det ID: "<< global_det_id << std::endl;
  std::cout << "DetX version: "<< format_version << std::endl;

  float UTC_validity_from, UTC_validity_to;
  infile >> UTC_validity_from >> UTC_validity_to;
  std::cout << "UTC validity from: "<< UTC_validity_from << std::endl;
  std::cout << "UTC validity to: "<< UTC_validity_to << std::endl;
  
  float UTM_ref_grid, UTM_ref_easting, UTM_ref_northing, UTM_ref_z;
  infile >> UTM_ref_grid >> UTM_ref_easting >> UTM_ref_northing >> UTM_ref_z;
  std::cout << "UTM ref grid: "<< UTM_ref_grid << std::endl;
  
  int n_doms;
  infile >> n_doms;
  std::cout << "Num Doms: "<< n_doms << std::endl;
  
  numCathods = 0;

  for (int dom = 0; dom < n_doms; dom++) {
    int dom_id, line_id, floor_id, n_pmts;
    infile >> dom_id >> line_id >> floor_id >> n_pmts;

    for (int pmt = 0; pmt < n_pmts; pmt++) {
      int pmt_id_global;
      double pos_x, pos_y, pos_z, dir_x, dir_y, dir_z, t0;
      infile >> pmt_id_global >> pos_x >> pos_y >> pos_z >> dir_x >> dir_y >> dir_z >> t0;

      // needed for can computation
      z_all.push_back(pos_z);
      r_all.push_back(std::sqrt(std::pow(pos_x, 2) + std::pow(pos_y, 2)));

      // the api is completely weird.
      // pass rotation as pointer, but vector by value
      // docs are at
      // http://proj-clhep.web.cern.ch/proj-clhep/manual/UserGuide/VectorDefs/node25.html
      G4RotationMatrix *rota = new G4RotationMatrix(
          G4ThreeVector(dir_x, dir_y, dir_z), 0);
      int dumb_id = 100 * dom + pmt;
      G4VPhysicalVolume *cathodPV = new G4PVPlacement(
          rota,
          G4ThreeVector(pos_x, pos_y, pos_z),
          cathodLog,
          "CathodVolume",
          worldLog,
          true,          // no boolean operation, whatever that means
          dumb_id         // copy ID
          );
      G4double CathodHeight = -1.0 * mm;
      G4double CathodRadius = 0.0;
      CathodRadius = ((G4Tubs *)cathodPV->GetLogicalVolume()->GetSolid())
        ->GetOuterRadius();
      CathodHeight = ((G4Tubs *)cathodPV->GetLogicalVolume()->GetSolid())
        ->GetZHalfLength();
      G4AffineTransform AffineTrans = G4AffineTransform();
      G4RotationMatrix RotationMatr = G4RotationMatrix();
      RotationMatr = RotationMatr * cathodPV->GetObjectRotationValue();
      //G4ThreeVector Direction = RotationMatr(G4ThreeVector(0.0, 0.0, 1.0));
      G4ThreeVector Direction = RotationMatr(G4ThreeVector(dir_x, dir_y, dir_z));
      G4ThreeVector Position = AffineTrans.TransformPoint(cathodPV->GetObjectTranslation());
      G4Transform3D trans(RotationMatr, Position);

      // applicable to thin tube cathods (normal run)
      // correct to full height
      CathodHeight *= 2.0;
      allCathods->addCathod(trans, Position, Direction, CathodRadius,
          CathodHeight);
      numCathods++;
    }
  }
  // derive OM/storey/tower positions from PMT positions
  // dont use them as Geant volumes (it's water after all)

  // Towers/benthos: only referenced in this file
  // interesting for propa:
  //    materials volumnes: cathods, crust, water
  //    detector boundaries (bottomPos, topPos, radius)

  // if you want to make sure, that your geometry definition is correct,
  // just uncomment the following to write out the entire world to xml
  // (also include the gdml headers)
  G4GDMLParser parser;
  parser.Write("orca.gdml", worldPV, false);

  return worldPV;
}

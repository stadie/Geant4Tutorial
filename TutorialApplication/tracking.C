#include "TCanvas.h"
#include "TGeoMatrix.h"
#include "TH1.h"
#include "THelix.h"
#include "TList.h"
#include "TMatrixTSym.h"
#include "TMinuit.h"
#include "TObjArray.h"
#include "TPad.h"
#include "TROOT.h"
#include "TRandom.h"
#include "TVector3.h"
#include "TVirtualGeoTrack.h"
#include "include/TutorialApplication.hh"

typedef TMatrixTSym<double> TMatrixDSym;

TH1F* hlayer1 =
    new TH1F("hlayer1", "layer1;z [cm]; counts", 100 / 0.0150, -50, 50);
TH1F* hlayer2 =
    new TH1F("hlayer2", "layer2;z [cm]; counts", 100 / 0.0150, -50, 50);
TH1F* hlayer3 =
    new TH1F("hlayer3", "layer3;z [cm]; counts", 100 / 0.0150, -50, 50);

TH1F* hresid1 =
    new TH1F("hresid1", "resid1; z_{hit}-z_{true} [cm]; hits", 100, -0.1, 0.1);
TH1F* hresid2 =
    new TH1F("hresid2", "resid2; z_{hit}-z_{true} [cm]; hits", 100, -0.1, 0.1);
TH1F* hresid3 =
    new TH1F("hresid3", "resid3; z_{hit}-z_{true} [cm]; hits", 100, -0.1, 0.1);

class Cluster : public TVector3 {
 public:
  Cluster(double x = 0, double y = 0, double startz = 0, double pitch = 0,
          unsigned char* strips = 0, int nstrips = 0)
      : TVector3(x, y, startz),
        fStartz(startz),
        fPitch(pitch),
        fNstrips(nstrips),
        fErrX(0),
        fErrY(0),
        fErrZ(0) {
    for (int i = 0; i < fNstrips; ++i) {
      fStrips[i] = strips[i];
    }
  }

  int nStrips() const { return fNstrips; }
  unsigned int signal(int istrip) const { return fStrips[istrip]; }
  double ZofFirstStrip() const { return fStartz; }
  double pitch() const { return fPitch; }

  double errX() const { return fErrX; }
  void setErrX(double ex) { fErrX = ex; }
  double errY() const { return fErrY; }
  void setErrY(double ey) { fErrY = ey; }
  double errZ() const { return fErrZ; }
  void setErrZ(double ez) { fErrZ = ez; }

 private:
  double fStartz;
  double fPitch;
  unsigned char fStrips[10];
  int fNstrips;
  double fErrX, fErrY, fErrZ;
  ClassDef(Cluster, 0)
};

ClassImp(Cluster);

unsigned char getSignal(const std::string& n) {
  TutorialApplication* app =
      (TutorialApplication*)TutorialApplication::Instance();

  int c = app->depEinNode(n) * 600000;
  // add noise
  c += gRandom->Gaus(0, 3);
  // noise cut
  int noisecut = 0;
  if (c < noisecut) return 0;
  if (c > 255) return 255;
  return c;
}

int updateClusters(TObjArray* clusters) {
  hlayer1->Reset();
  hlayer2->Reset();
  hlayer3->Reset();
  clusters->Clear();
  gGeoManager->CdTop();
  // gGeoManager->CdDown(0);
  // TGeoVolume* top = gGeoManager->GetMasterVolume();

  unsigned char strips[10];
  TH1F* hcurrent = 0;
  // loop over layers
  for (int i = 0, ml = gGeoManager->GetCurrentNode()->GetNdaughters(); i < ml;
       ++i) {
    hcurrent = 0;
    if (i == 0) hcurrent = hlayer1;
    if (i == 1) hcurrent = hlayer2;
    if (i == 2) hcurrent = hlayer3;
    // gGeoManager->GetCurrentNode()->Print();
    gGeoManager->CdDown(i);
    // gGeoManager->GetCurrentNode()->Print();
    for (int j = 0, mj = gGeoManager->GetCurrentNode()->GetNdaughters(); j < mj;
         ++j) {
      gGeoManager->CdDown(j);
      // gGeoManager->GetCurrentNode()->Print();
      TString name(gGeoManager->GetCurrentNode()->GetName());
      if (!name.BeginsWith("SiLayer")) {
        gGeoManager->CdUp();
        continue;
      }
      // loop over strips
      for (int k = 0, mk = gGeoManager->GetCurrentNode()->GetNdaughters();
           k < mk; ++k) {
        gGeoManager->CdDown(k);
        TString name2(gGeoManager->GetCurrentNode()->GetName());
        assert(name2.BeginsWith("Strip"));
        // gGeoManager->GetCurrentNode()->Print();
        strips[0] = getSignal(gGeoManager->GetPath());
        if (strips[0]) {
          int nstrips = 0;
          double zmin, zmax;
          gGeoManager->GetCurrentVolume()->GetShape()->GetAxisRange(3, zmin,
                                                                    zmax);
          double pitch = zmax - zmin;
          double local[3] = {0, 0, 0};
          double pos[3] = {0, 0, 0};
          gGeoManager->LocalToMaster(local, pos);
          // std::cout << gGeoManager->GetCurrentNavigator()->GetPath() << "
          // Node:" << gGeoManager->GetCurrentNodeId() << ", " <<
          // gGeoManager->GetCurrentNode() << '\n';
          // std::cout << pos[0] << "," << pos[1] << ", " << pos[2]  << ":" <<
          // (int)strips[0] << '\n';
          if (hcurrent) hcurrent->Fill(pos[2], strips[0]);
          for (nstrips = 1; nstrips < 10; ++nstrips) {
            gGeoManager->CdUp();
            if (k + nstrips >= gGeoManager->GetCurrentNode()->GetNdaughters()) {
              gGeoManager->CdDown(k);
              break;
            }
            gGeoManager->CdDown(k + nstrips);
            // std::cout << gGeoManager->GetPath() << std::endl;
            strips[nstrips] = getSignal(gGeoManager->GetPath());
            if (!strips[nstrips]) break;
            if (hcurrent)
              hcurrent->Fill(pos[2] + nstrips * pitch, strips[nstrips]);
          }
          // std::cout << "cluster:" << pos[0] << "," << pos[1] << ", " <<
          // pos[2]  << ":" << (int)strips[0] << ", " <<
          // (int)strips[0]+(int)strips[1] << " nstrips = " << nstrips << '\n';
          clusters->Add(
              new Cluster(pos[0], pos[1], pos[2], pitch, strips, nstrips));
          k += nstrips;
        }  // strip with charge
        gGeoManager->CdUp();
      }  // loop within silayer
      gGeoManager->CdUp();
    }  // loop within layer
    gGeoManager->CdUp();
  }  // loop over layers
  return clusters->GetEntriesFast();
}

int reconstructHits(TObjArray* clusters) {
  for (int i = 0; i < clusters->GetEntriesFast(); ++i) {
    Cluster* c = (Cluster*)clusters->At(i);
    // use first strip for position
    c->SetZ(c->ZofFirstStrip());
    c->setErrZ(c->nStrips() * c->pitch());
  }
  return clusters->GetEntriesFast();
}

double getTrueZ(double detx) {
  // get primary track
  TObjArray* tracks = gGeoManager->GetListOfTracks();
  TVirtualGeoTrack* track = (TVirtualGeoTrack*)tracks->At(0);
  // track->Print();
  Double_t x, y, z, t;
  Int_t j = 0, N = track->GetNpoints();
  do {
    if (j >= N) return -999;
    track->GetPoint(j, x, y, z, t);
    j++;
  } while (detx > x);
  double x1 = x;
  double z1 = z;
  j -= 2;
  track->GetPoint(j, x, y, z, t);
  double x2 = x;
  double z2 = z;
  // std::cout << x1 << "," << z1 << "; " << x2 << "," << z2 << '\n';
  // std::cout << (z2-z1)/(x2-x1)*(detx-x1) + z1 << '\n';
  return (z2 - z1) / (x2 - x1) * (detx - x1) + z1;
}

void plotResdiuals(TObjArray* clusters) {
  for (int i = 0; i < clusters->GetEntriesFast(); ++i) {
    Cluster* c = (Cluster*)clusters->At(i);
    double x = c->X();
    // fill residual plots; x-position of layers hardcoded!!!
    if (x == -45) {
      double zorig = getTrueZ(x);
      hresid1->Fill(zorig - c->Z());
    }
    if (x == -30) {
      double zorig = getTrueZ(x);
      hresid2->Fill(zorig - c->Z());
    }
    if (x == 45) {
      double zorig = getTrueZ(x);
      hresid3->Fill(zorig - c->Z());
    }
  }
}

void tracking() {
  TutorialApplication* app =
      (TutorialApplication*)TutorialApplication::Instance();

  // initialize calorimeter volumes and material
  app->InitMC("geometry/tracker2");

  // define particle and control parameters of loop
  unsigned int nevt = 1;
  double p = 0.5;
  app->SetPrimaryPDG(-13);  //-13    // +/-11: PDG code of e+/-
  /* other PDG codes     22: Photon    +-13: muon
                     +/-211: pion   +/-2212: proton     */
  app->SetPrimaryMomentum(p);
  // generate  some events
  TObjArray* clusters = new TObjArray();
  clusters->SetOwner(true);
  hresid1->Reset();
  hresid2->Reset();
  hresid3->Reset();
  for (unsigned int i = 0; i < nevt; ++i) {
    bool draw = !(i % 10);
    p = gRandom->Gaus(1.0, 0.1);
    app->SetPrimaryMomentum(p);
    app->RunMC(1, draw);
    updateClusters(clusters);
    reconstructHits(clusters);
    plotResdiuals(clusters);
  }

  TCanvas* c = new TCanvas("c");
  c->Divide(3, 2);
  c->cd(1);
  hlayer1->Draw();
  c->cd(2);
  hlayer2->Draw();
  c->cd(3);
  hlayer3->Draw();
  c->cd(4);
  hresid1->Draw();
  c->cd(5);
  hresid2->Draw();
  c->cd(6);
  hresid3->Draw();
}

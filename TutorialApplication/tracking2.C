#include "include/TutorialApplication.hh"
#include "TH1.h"
#include "TCanvas.h"
#include "TVector3.h"
#include "TRandom.h"
#include "TObjArray.h"
#include "TVirtualGeoTrack.h"
#include "TROOT.h"
#include "TGeoMatrix.h"
#include "TMatrixTSym.h"
#include "THelix.h"
#include "TMinuit.h"
#include "TList.h"
#include "TPad.h"
#include "TF1.h"

#include <cassert>

typedef TMatrixTSym<double>	TMatrixDSym;

TH1F *hlayer1 = new TH1F("hlayer1","layer1;z [cm]; counts",100/0.0150,-50,50);
TH1F *hlayer2 = new TH1F("hlayer2","layer2;z [cm]; counts",100/0.0150,-50,50);
TH1F *hlayer3 = new TH1F("hlayer3","layer3;z [cm]; counts",100/0.0150,-50,50);
TH1F *hresid1 = new TH1F("hresid1","resid1; z_{hit}-z_{true} [cm]; events",100,-0.1,0.1);
TH1F *hresid2 = new TH1F("hresid2","resid2; z_{hit}-z_{true} [cm]; events",100,-0.1,0.1);
TH1F *hresid3 = new TH1F("hresid3","resid3; z_{hit}-z_{true} [cm]; events",100,-0.1,0.1);
TH1F *hpt = new TH1F("hpt","; p_{T} [GeV]",100,0,10);
TH1F *hptpull = new TH1F("hptpull","; (p_{T}^{meas} - p_{T}^{true})/#sigma",100,-10,10);

class Cluster : public TVector3 {
public:
  Cluster(double x = 0, double y = 0, double startz = 0, double pitch = 0,unsigned char* strips = 0, 
	  int nstrips = 0, int layer = 0) : 
    TVector3(x,y, startz), fStartz(startz), fPitch(pitch), fNstrips(nstrips), fLayer(layer),
    fErrX(0), fErrY(0), fErrZ(0) {
    for(int i = 0 ; i < fNstrips ; ++i) {
      fStrips[i] = strips[i];
    }
  } 
  
  int nStrips() const { return fNstrips;}
  unsigned int signal(int istrip) const {return fStrips[istrip];}
  double  ZofFirstStrip() const { return fStartz;}
  double pitch() const { return fPitch;}
  int layer() const {return fLayer;}

  double errX() const {return fErrX;}
  void setErrX(double ex) { fErrX = ex;} 
  double errY() const {return fErrY;}
  void setErrY(double ey) { fErrY = ey;} 
  double errZ() const {return fErrZ;}
  void setErrZ(double ez) { fErrZ = ez;} 
private:
  double fStartz;
  double fPitch;
  unsigned char fStrips[10]; 
  int fNstrips;
  int fLayer;
  double fErrX,fErrY,fErrZ;
  ClassDef(Cluster,0)
};


class Track {
public:
  Track(double c, double phi, double d) :
    fCurv(c), fPhi0(phi), fD0(d),fCov(3) {}
  
  THelix* helix() const;
  
  int charge() const { return fCurv > 0 ? 1 : -1;}
  double r() const { return std::abs(1/curvature());}
  double curvature() const { return fCurv ? std::abs(fCurv) : 1e-10;}
  double d0() const { return fD0;}
  double phi0() const { return fPhi0;}
  double cov(int i, int j) const { return fCov(i,j);}
  double chi2() const { return fChi2;}
  int nHits() const { return fNHits;}

  void setCurvature(double c) { fCurv = c;};
  void setD0(double d0) { fD0 = d0;};
  void setPhi0(double phi0) { fPhi0 = phi0;};
  void setChi2(double chi2) { fChi2 = chi2;};
  void setNHits(int nhits) { fNHits = nhits;};
  void setParameters(double a, double b, double c) {
    fCurv  = a;
    fPhi0  = b;
    fD0    = c;
  }
  double pt() const { return 1.49898e-03 * 2 * B() *r();}

  double rErr() const { return sqrt(fCov(0,0))*r()*r();}
  double ptErr() const { return 1.49898e-03 * 2 *  B() * rErr();}


  void setCov(int i, int j, double c) { fCov(i,j) = c;}
  
  double x(double lambda) const { return x0() + r() * charge() * sin(charge()*lambda + phi0());}
  double z(double lambda) const { return z0() - r() * charge() * cos(charge()*lambda + phi0());}
  double y(double ) const { return 0; }
  
  double lambdaFromX(double posx) const {
    double arg =  (posx-x0()) /charge()/r();
    if(arg < -1) arg = -1;
    if(arg > 1) arg =1;
    return charge()*(asin(arg ) - phi0());}

  double x0() const {return -sin(phi0()) * (d0()+charge()*r()) - 50;}
  double z0() const {return  cos(phi0()) * (d0()+charge()*r());}

private:
  double fCurv, fPhi0, fD0, fChi2;
  int fNHits;
  TMatrixDSym fCov;

  static double B() {
    TutorialApplication* app = (TutorialApplication*)TutorialApplication::Instance();
    double x[]={0,0,0};
    double bfield[3];
    app->Field(x,bfield);
    return bfield[1]/10;
  }
};

THelix* Track::helix() const {
  double xyz[3],v[3],range[2],axis[3];
  xyz[0] = x(0);
  xyz[1] = y(0);
  xyz[2] = z(0);
  v[0] = charge() * r();
  v[1] = 0;
  v[2] = 0;
  range[0] = 50;//std::min(50.0,x0()+r());
  range[1] = -50;//std::max(-50.0,x0()-r());
  axis[0] = 0;
  axis[1] = 1;
  axis[2] = 0;
  THelix* h = new THelix(xyz,v,1,range,kHelixY,axis);
  //h->Print();
  h->SetLineColor(7);
  return h;
}


ClassImp(Cluster)

unsigned char getSignal(const std::string& n) 
{ 
  TutorialApplication* app = (TutorialApplication*)TutorialApplication::Instance();
  
  int c = app->depEinNode(n) * 600000;
  //if(c > 0) std::cout << "getSignal for " << n << " :" << c << std::endl;
  //add noise
  c += gRandom->Gaus(0,3);
  //noise cut
  int noisecut = 12;
  if( c < noisecut ) return 0;
  if(c > 255) return 255;
  return c;
}

int updateClusters(TObjArray* clusters) 
{ 
  hlayer1->Reset();
  hlayer2->Reset();
  hlayer3->Reset();
  clusters->Clear();
  gGeoManager->CdTop();
  //gGeoManager->CdDown(0);
  //TGeoVolume* top = gGeoManager->GetMasterVolume();  
  
  unsigned char strips[10];
  TH1F* hcurrent = 0;
  //loop over layers
  for(int i = 0, ml = gGeoManager->GetCurrentNode()->GetNdaughters() ; i < ml ; ++i) {
    hcurrent = 0;
    if(i == 0) hcurrent = hlayer1;
    if(i == 1) hcurrent = hlayer2;


    if(i == 2) hcurrent = hlayer3;
    //gGeoManager->GetCurrentNode()->Print();
    gGeoManager->CdDown(i);
    //gGeoManager->GetCurrentNode()->Print();
    for(int j = 0, mj =  gGeoManager->GetCurrentNode()->GetNdaughters() ; j < mj ; ++j) {
      gGeoManager->CdDown(j);
      //gGeoManager->GetCurrentNode()->Print();
      TString name(gGeoManager->GetCurrentNode()->GetName());
      if(! name.BeginsWith("SiLayer")) { gGeoManager->CdUp(); continue;}
      //loop over strips
      for(int k = 0, mk = gGeoManager->GetCurrentNode()->GetNdaughters(); k < mk ; ++k) {
        gGeoManager->CdDown(k);
        TString name2(gGeoManager->GetCurrentNode()->GetName());
        assert(name2.BeginsWith("Strip"));
        //gGeoManager->GetCurrentNode()->Print();
        strips[0] = getSignal(gGeoManager->GetPath());
        if(strips[0]) {
	  //std::cout << "strip with signal:" << gGeoManager->GetPath() << ":" << strips[0] << '\n';
          int nstrips = 0;
          double zmin,zmax;
          gGeoManager->GetCurrentVolume()->GetShape()->GetAxisRange(3,zmin,zmax);
          double pitch = zmax-zmin;
          double local[3]={0,0,0};
          double pos[3]={0,0,0};
          gGeoManager->LocalToMaster(local,pos);
          //std::cout << gGeoManager->GetCurrentNavigator()->GetPath() << " Node:" << gGeoManager->GetCurrentNodeId() << ", " << gGeoManager->GetCurrentNode() << '\n';
          //std::cout << pos[0] << "," << pos[1] << ", " << pos[2]  << ":" << (int)strips[0] << '\n';
          if(hcurrent) hcurrent->Fill(pos[2],strips[0]);
          for(nstrips = 1 ; nstrips < 10 ; ++nstrips) { 
            gGeoManager->CdUp();
            if(k+nstrips >= gGeoManager->GetCurrentNode()->GetNdaughters()) {gGeoManager->CdDown(k); break;}
            gGeoManager->CdDown(k+nstrips);
            //std::cout << gGeoManager->GetPath() << std::endl;
            strips[nstrips] = getSignal(gGeoManager->GetPath());
            if(! strips[nstrips]) break;
            if(hcurrent) hcurrent->Fill(pos[2]+nstrips*pitch,strips[nstrips]);
          }
          //std::cout << "cluster:" << pos[0] << "," << pos[1] << ", " << pos[2]  << ":" << (int)strips[0] << ", " << (int)strips[0]+(int)strips[1] << " nstrips = " << nstrips << '\n';
          clusters->Add(new Cluster(pos[0],pos[1],pos[2],pitch,strips,nstrips,i+1));
          k+= nstrips;
        }//strip with charge
        gGeoManager->CdUp();
      }//loop within silayer
      gGeoManager->CdUp();
    }//loop within layer
    gGeoManager->CdUp();
  }//loop over layers
  return clusters->GetEntriesFast();
}


int reconstructHitsSimple(TObjArray* clusters) 
{
  for(int i = 0 ; i < clusters->GetEntriesFast() ; ++i) {
    Cluster* c = (Cluster*)clusters->At(i);
    //use first strip for position
    c->SetZ(c->ZofFirstStrip());
    c->setErrZ(c->nStrips()*c->pitch());
  }
  return clusters->GetEntriesFast();
}

int reconstructHitsBinary(TObjArray* clusters) 
{
  for(int i = 0 ; i < clusters->GetEntriesFast() ; ++i) {
    Cluster* c = (Cluster*)clusters->At(i);
    unsigned int maxc = c->signal(0);
    int maxstrip = 0;
    for(int j = 0 ; j < c->nStrips() ; ++j) {
      if(maxc < c->signal(j)) {
	maxstrip = j;
	maxc = c->signal(maxstrip);
      }
    }
    //std::cout << "maxstrip " << maxstrip << ", " << maxc << '\n';
    //use strip with max charge for position
    c->SetZ(c->ZofFirstStrip()+maxstrip*c->pitch());
    c->setErrZ(c->pitch()/sqrt(12));
  }
  return clusters->GetEntriesFast();
}


double getTrueZ(double detx) {
  //get primary track
  TObjArray* tracks = gGeoManager->GetListOfTracks();
  TVirtualGeoTrack* track = (TVirtualGeoTrack*)tracks->At(0);
  //track->Print();
  Double_t x,y,z,t;
  Int_t j=0;
  do {
    track->GetPoint(j,x,y,z,t);
    j++;
  } while(detx > x && j < track->GetNpoints());
  double x1 = x;
  double z1 = z;
  j -= 2;
  track->GetPoint(j,x,y,z,t);
  double x2 = x;
  double z2 = z;
  //std::cout << x1 << "," << z1 << "; " << x2 << "," << z2 << '\n';
  //std::cout << (z2-z1)/(x2-x1)*(detx-x1) + z1 << '\n';
  return (z2-z1)/(x2-x1)*(detx-x1) + z1;
}

void plotResdiuals(TObjArray* clusters) {
  for(int i = 0 ; i < clusters->GetEntriesFast() ; ++i) {
    Cluster* c = (Cluster*)clusters->At(i);
    double x = c->X();
    //fill residual plots; x-position of layers hardcoded!!!
    double zorig = getTrueZ(x);    
    if(c->layer() == 1)
      hresid1->Fill(zorig-c->Z());
    else {
      if(c->layer() == 2)
	hresid2->Fill(zorig-c->Z());
      else if(c->layer() == 3)
	hresid3->Fill(zorig-c->Z());
    }
  }
}

//globals for fit :-(
Track* gTrack;
const std::vector<Cluster*> *gClusters;

void fcn(Int_t &, Double_t *, Double_t &f, Double_t *par, Int_t) {
  //set new track parameters:
  gTrack->setParameters(par[0],par[1],par[2]);
  //std::cout << "track: r,x0,z0 = " << fTrack->r() << ", " << fTrack->x0() << ", " << fTrack->z0() << '\n'; 
  double chi2 = 0;
  for(unsigned int i = 0 ; i < gClusters->size() ; ++i) {
    Cluster* c = gClusters->at(i);
    double x = c->X();
    double z = c->Z();
    double lambda = gTrack->lambdaFromX(x);
    
    //std::cout << "hit:" << x << ", " << z << "   track:" << fTrack->x(lambda) << ", " << fTrack->z(lambda) 
    //	      << "  lambda = " << lambda << '\n';
    
    double dZ = z - gTrack->z(lambda);
    double dX = x - gTrack->x(lambda);
    chi2 += (dZ*dZ)/c->errZ()/c->errZ() + dX*dX*1e6;
  }
  //std::cout << "Chi2:  " << chi2 << '\n';
  f = chi2;
}


Track* fitTrack(const std::vector<Cluster*>& clusters) {
  gTrack = new Track(0,0,0);
  gClusters = &clusters;
  
  TMinuit * minuit = new TMinuit(3);
  
  minuit->SetFCN(&fcn);
  minuit->DefineParameter(0,"Curvature",gTrack->curvature(),0.00001,0,0);
  minuit->DefineParameter(1,"Phi0",gTrack->phi0(),0.001,0,0);
  minuit->DefineParameter(2,"D0",gTrack->d0(),0.1,0,0);
  //minuit->DefineParameter(0,"R",(gTrack->charge() * gTrack->r()),1,0,0);
  //minuit->DefineParameter(1,"X0",gTrack->x0(),1,0,0);
  //minuit->DefineParameter(2,"Z0",gTrack->z0(),1,0,0);
  minuit->SetPrintLevel(1);
  int iret = minuit->Migrad();
  //minuit->PrintResults(1,0);
  if(iret != 0) {
    std::cout << "track fit failed.\n";
    gTrack->setParameters(1.0e10,0,0);
    gTrack->setNHits(0);
    gTrack->setChi2(1e10);
  } else {
    std::vector<double> cov(9);
    minuit->mnemat(&cov.front(),3);
    for(int i = 0 ; i < 3 ; ++i) {
      for(int j = i ; j < 3; ++j) {
	gTrack->setCov(i,j,cov[i*3+j]);
      }
    }
    gTrack->setNHits(clusters.size());
    double amin,edm,errdef;
    int nvpar,nparx;
    minuit->mnstat(amin,edm,errdef,nvpar,nparx,iret);
    gTrack->setChi2(amin);
  }
  return gTrack;
}

void removeAllHelices() {
  TObjLink *lnk = gPad->GetListOfPrimitives()->FirstLink();
  while (lnk) {
    TObject* to = lnk->GetObject();
    if(to->InheritsFrom(THelix::Class())) to->Delete(); 
    lnk = lnk->Next();    
  }
}


void tracking2()
{
  TutorialApplication* app = (TutorialApplication*)TutorialApplication::Instance();
  // position of silicon layers in x   
  double pos1 = -45.0;
  double pos2 = -30.0;
  double pos3 = 45.0; 
  double pitch = 0.0150;
  double materialLength = 0.05;//length of support structures
  double Bfield = 2.0;//magnetic field in T
  TString geom("geometry/tracker2(");
  geom+=pos1; geom.Append(",");
  geom+=pos2; geom.Append(",");
  geom+=pos3; geom.Append(",");
  geom+=pitch; geom.Append(",");
  geom+=materialLength; geom.Append(",");
  geom+=Bfield; geom.Append(")"); 
  app->InitMC(geom); 


  TH2F* hpt2 = new TH2F("hpt2", " ;p_{T} [Gev]; #frac{p_{T}^{reco}-p_{T}}{p_{T}};", 15, 1, 15, 40,-0.1,0.1);
  TH1F* hpterr = new TH1F("hpterr",";p_{T} [Gev]; #sigma(#frac{p_{T}^{reco}-p_{T}}{p_{T}})",12,1,15);
  for(int bin = 1 ; bin <= hpt2->GetNbinsX() ; ++bin) {
    // define particle and control parameters of loop
    unsigned int nevt = 500;
    double p = ((TAxis*)hpt2->GetXaxis())->GetBinCenter(bin);;
    app->SetPrimaryPDG(-13);    // +/-11: PDG code of e+/-
    /* other PDG codes     22: Photon    +-13: muon
       +/-211: pion   +/-2212: proton     */
    app->SetPrimaryMomentum(p);
    // generate  some events
    hresid1->Reset();
    hresid2->Reset();
    hresid3->Reset();
    hpt->Delete();
    hpt = new TH1F("hpt","; p_{T} [GeV]",20,p-1,p+1);
    hptpull->Reset();
    TObjArray* clusters = new TObjArray();
    clusters->SetOwner(true);
    for(unsigned int i=0;i<nevt;++i) {
      bool draw = !i;
      double z = gRandom->Uniform(-5.0,5.0);
      app->SetPrimaryVertex(-50,0,z);
      double phi = gRandom->Uniform(TMath::Pi()/2-0.1,TMath::Pi()/2+0.1);
      TVector3 dir;
      dir.SetPtThetaPhi(p,phi,0);
      app->SetPrimaryMomentum(dir);
      removeAllHelices(app->GetDrawPad());
      app->RunMC(1, draw);
      updateClusters(clusters);
      reconstructHitsBinary(clusters);
      plotResdiuals(clusters);
      if(clusters->GetEntriesFast() >= 3) {
	std::vector<Cluster*> clust;
	for(int i = 0 ; i < clusters->GetEntriesFast() ; ++i) {
	  Cluster* c = (Cluster*)clusters->At(i);
	  clust.push_back(c);
	}
	Track *t = fitTrack(clust);
	if(draw) t->helix()->Draw();
	hpt->Fill(t->pt());
	hpt2->Fill(p,(t->pt()-p)/p);
	hptpull->Fill((t->pt()-p)/t->ptErr());
      } else {
	std::cout << "Warning: Not enough hits for track fit.\n";
      }
    }
    hpterr->SetBinContent(bin,hpt->GetRMS());
    hpterr->SetBinError(bin,hpt->GetRMSError());
  }
  TCanvas* c = new TCanvas("c");
  c->Divide(3,2);
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

  TCanvas* c2 = new TCanvas("c2");
  c2->Divide(2,1);
  c2->cd(1);
  hpt->Draw();
  c2->cd(2);
  hptpull->Draw();
  TCanvas* c3 = new TCanvas("c3");
  TF1* f= new TF1("f","sqrt([0]*[0]*x*x+[1]*[1])");
  c3->Divide(2,1);
  c3->cd(1);
  hpterr->Fit(f);
  c3->cd(2);
  hpt2->FitSlicesY();
  ((TH1F*)gROOT->FindObject("hpt2_2"))->Fit(f);
}

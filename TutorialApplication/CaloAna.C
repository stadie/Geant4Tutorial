#include "include/TutorialApplication.hh"
#include "TGeoManager.h"
#include "TVirtualGeoTrack.h"
#include "TDatabasePDG.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TF1.h"
#include <iostream>
#include <string>

Float_t XofFirstSecondary()
{
  TObjArray* tracks = gGeoManager->GetListOfTracks();
  
  if(tracks->GetEntriesFast() < 2) return 999;
  
  //get first secondary
  TVirtualGeoTrack* track = (TVirtualGeoTrack*)tracks->At(1);
  
  //get the first point of this track
  Double_t x,y,z,t;
  track->GetPoint(0,x,y,z,t);
  
  return x;
}



Int_t CountChargedinScint()
{
  Int_t ncharged = 0;

  TObjArray* tracks = gGeoManager->GetListOfTracks();

  for(Int_t i=0,l=tracks->GetEntriesFast();i<l;++i) {
    TVirtualGeoTrack* track = (TVirtualGeoTrack*)tracks->At(i);
    //track->Print();
    
    Int_t pdg = track->GetPDG();
    Double_t charge = TDatabasePDG::Instance()->GetParticle(pdg)->Charge();
    if( charge == 0 ) continue;
    
    Double_t x,y,z,t;
    TGeoNode* lastnode = NULL;
    for(Int_t j=0,lp=track->GetNpoints();j<lp;++j) {
      track->GetPoint(j,x,y,z,t);
      TGeoNode* node = gGeoManager->FindNode(x,y,z);
      if(! node ) continue;
      //node->Print();
      
      if( lastnode == node ) continue;
      lastnode = node;
      //is scintillator ?
      //std::cout << node->GetMedium()->GetMaterial()->GetZ() << std::endl;
      
      if(node->GetMedium()->GetMaterial()->GetZ() == 1)
	++ncharged;
    }
    //std::cout << "charge:" << ncharged << std::endl;
  }
  //std::cout << ncharged << std::endl;
  return ncharged;
}


void CaloAna()
{
  TutorialApplication* app = (TutorialApplication*)TutorialApplication::Instance();
// initialize geometry: volumes and materials of a Sampling Calorimeter   
  Double_t AbsWid=0.3;         //Absorber width
  Double_t SciWid=1.;         //Scintillator width, 
  Double_t SizeFact=10;      //size of the calorimeter in interaction lengths labmda_I, 4.
  Int_t IMat=1;               //material 1:Pb 2:Fe 
  TString geom("geometry/SamplingCalorimeter(");
  geom+=AbsWid; geom.Append(",");
  geom+=SciWid; geom.Append(",");
  geom+=SizeFact; geom.Append(",");
  geom+=IMat; geom.Append(")");
  app->InitMC(geom);
  // xxx app->InitMC("geometry/SamplingCalorimeter(2.,1.,2.,1)"); 

// Book histogram(s)
 // for shower analyis
  TH1F* hx  = new TH1F("hx","starting point of shower",100, 0.,25.);
  hx->SetYTitle("# of entries");  
  hx->SetXTitle("x of first vertex [cm]");
  TH1F* hwidth  = new TH1F("hwidth","width of the shower",50,0.,25.);
  hwidth->SetYTitle("# of entries");
  hwidth->SetXTitle("width of the shower [cm]");
  TH1F* hlength = new TH1F("hlength","length of the shower",50,0.,25.);
  hlength->SetYTitle("# of entries");
  hlength->SetXTitle("length of the shower [cm]");
  // for hit counting
  TProfile* hcounts = new TProfile("hcounts","Counts vs particle energy",
				   20,0.,10.,"s");
  // option "s": show sigma(i) instead of sigma(i)/sqrt(n_i)
  hcounts->SetXTitle("energy [GeV]");
  hcounts->SetYTitle("mean number of counts");
  TH2D* hresponse = new TH2D("hresponse","measured energy/particle energy vs particle energy; energy [GeV]; response",10,1.,20, 50,0,2);
  const double C = 3./345;
  // const double C = 1./176;
//simulate events at fixed momentum
  TH1F* hhelp; // for analysis of internal histograms
  Double_t xp[1]={0.90},xq[1];

  unsigned int nevt = 100;
  double       p = 3;//GeV

  app->SetPrimaryPDG(11); 
  /* PDG codes     22: Photon    +/-11: e+/-  +-13: muon   
               +/-211: pion    +/-2212: proton              */
  app->SetPrimaryMomentum(p);
  for(unsigned int i = 0 ; i < nevt ; ++i) {
    app->RunMC(1,!(i%100)); 
    // fill starting point of shower (pos. of first secondary)
    hx->Fill(XofFirstSecondary());
    // access GEANT internal histograms
    hhelp = (TH1F*) gROOT->Get("hEdepTrans"); assert(hhelp);
    // and evaluate quantiles x-wise (-> radius)
    hhelp->GetQuantiles(1,xq,xp);
    // fill the width of the event as two times the max. radius
    hwidth->Fill(2.*xq[0]);
    hhelp = (TH1F*) gROOT->Get("hEdepLong"); assert(hhelp);
    hhelp->GetQuantiles(1,xq,xp);
    hlength->Fill(xq[0]);
    
    // reset internal histograms
    app->FinishRun();
  }
  
  // events at different momenta
  nevt = 500; 
  for(int bin = 1 ; bin <= hresponse->GetNbinsX() + 1 ; ++bin) {
    // define particle and control parameters of loop
    double p = ((TAxis*)hresponse->GetXaxis())->GetBinCenter(bin);
    // generate a large number of events
    for(unsigned int i=0;i<nevt;++i) {
      app->SetPrimaryMomentum(p);
      app->RunMC(1,!(i%100));
      hcounts->Fill(p,CountChargedinScint());
      hresponse->Fill(p,C*CountChargedinScint()/p);      
      // reset internal histograms
      app->FinishRun();
    }
  }
  // display results  
  TCanvas* c = new TCanvas(); c->Divide(2,2);
  c->cd(1);  hx->Draw();
  c->cd(2);  hwidth->Draw();
  c->cd(3);  hlength->Draw();
  c->cd(4);  hcounts->Fit("pol1");
  TCanvas* c2 = new TCanvas("c2"); c2->Divide(2,1);
  c2->cd(1);
  hresponse->Draw();
  c2->cd(2);
  hresponse->FitSlicesY();
  TH1D* hresponse_2 = (TH1D*)gROOT->FindObject("hresponse_2");
  TF1* f= new TF1("f","sqrt([0]*[0]/x+[1]*[1]+[2]*[2]/x/x)");
  f->SetParNames("S","C","N");
  hresponse_2->Fit(f);
}

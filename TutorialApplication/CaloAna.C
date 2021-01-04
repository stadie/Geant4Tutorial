void CaloAna()
/* 
 sample analyis macro for simulation of calorimeters with geant4
 use macro CountChargedinScint.C to plot the number of hits in
    an active calorimeter layer as a function of momentum 
*/
{
  // load hepler scripts
  gROOT->ProcessLine(".L CountChargedinScint.C+O");
  gROOT->ProcessLine(".L XofFirstSecondary.C+");

// initialize geometry: volumes and materials of a Sampling Calorimeter   
  Double_t AbsWid=2.;         //Absorber width
  Double_t SciWid=1.;         //Scintillator width, 
  Double_t SizeFact=0.7;      //size of the calorimeter in interaction lengths labmda_I, 4.
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
  TH2D* hresponse = new TH2D("hresponse","measured energy/particle energy vs particle energy; energy [GeV]; response",
			     20,0.,10, 50,0,2);
  const double C = 1./53;
  // const double C = 1./176;
//simulate events at fixed momentum
  TH1F* hhelp; // for analysis of internal histograms
  Double_t xp[1]={0.90},xq[1];

  unsigned int nevt = 100; Int_t i;
  double       p = 3.;

  app->SetPrimaryPDG(-11); 
  /* PDG codes     22: Photon    +/-11: e+/-  +-13: muon   
               +/-211: pion    +/-2212: proton              */
  app->SetPrimaryMomentum(p);
  for(i = 0 ; i < nevt ; ++i) {
    app->RunMC(1,!(i%10)); 
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
  nevt = 1000; p = 0.1;
  double stepping = 9.9 / nevt;
  // generate a large number of events
  for(i=0;i<nevt;++i) {
    app->SetPrimaryMomentum(p);
    app->RunMC(1,!(i%10));
    hcounts->Fill(p,CountChargedinScint());
    hresponse->Fill(p,C*CountChargedinScint()/p);
    p += stepping;
    
    // reset internal histograms
    app->FinishRun();
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
  hresponse_2->Draw();
}

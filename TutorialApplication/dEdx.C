void dEdx()
{

  const Int_t nev = 400;
  const Double_t density = 8.960;//g cm3
  const Double_t mass = 0.1057;
  const Double_t length = 0.2;
  app->InitMC("geometry/cubox");
  app->SetPrimaryPDG(-13);

  TGraphErrors* gdEdx =  new TGraphErrors(0);
  TCanvas* c2 = new TCanvas("c2");
  //gdEdx->Set(1);
  c2->Clear();
  c2->cd();
  TH1F* hloss = new TH1F("hloss","; -dE [MeV]",100,0,10);
  TH1F *h1= new TH1F("h1",";#beta#gamma;-dE/dx [MeV g^{-1} cm^{2}]",100,pow(10,-3),pow(10,7));
  h1->Draw();
  h1->SetMaximum(100);
  h1->SetMinimum(0.1);
  c2->SetLogx();
  c2->SetLogy();
  gdEdx->Draw("P");
  Int_t gdEdx_i = 0;
  TFolder* histfolder=(TFolder*)gROOT->FindObjectAny("/Geant4/Histograms");
  TProfile* hprim=(TProfile*)gROOT->FindObjectAny("/Geant4/Histograms/hPrimaryEnergy");
  for(double x = -3; x < 6 ; x+=0.2) {
    double momentum = pow(10,x);
    double minDE=100000000;
    double maxDE=0;
    hloss->Reset();
    for(int i = 0 ; i < nev ; ++i) {
      Double_t beta = momentum/sqrt(momentum*momentum + mass*mass);
      Double_t gamma = 1/sqrt(1-beta*beta);
      app->SetPrimaryMomentum(momentum);
      hprim->Reset();
      app->RunMC(1,!i);
      double Eafter = 0;
      for(int j = hprim->GetNbinsX()+1 ;  hprim->GetBinCenter(j) > 0. ; --j) {
	if(hprim->GetBinEntries(j)) {
	  //std::cout << "j:" << j << "z:" << hprim->GetBinCenter(j) << "  E:" << hprim->GetBinContent(j) << '\n';
	  Eafter = hprim->GetBinContent(j);
	  break;
	}
      }
      std::cout <<  sqrt(mass*mass+momentum*momentum) << ", " << Eafter << '\n';
      //double loss = sqrt(mass*mass+momentum*momentum)-Eafter;
      double loss = app->depEinNode("/EXPH_1/CALB_1");
      loss *= 1000; //MeV
      hloss->Fill(loss);
    }
    double dEdx = hloss->GetMean() / density /length; //MeV g-1 cm2
    std::cout << "betagamma:" << momentum/mass << "     -dE/dex:" << dEdx << '\n';
    gdEdx->SetPoint(gdEdx_i,momentum/mass,dEdx);
    gdEdx->SetPointError(gdEdx_i,0,hloss->GetMeanError() / density /length);
    c2->Modified();
    c2->Update();
    ++gdEdx_i;
  }
  TCanvas* c1 = new TCanvas("c1");
  hloss->Draw();
}

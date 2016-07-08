//
// Script to define a simple geometry, a lead box, for TGeantApplication
//
void cubox(double dx = 0.2)
{
  // identifier of tracking media
  Int_t indVac = gGeoManager->GetMedium("Vacuum")->GetId();
  Int_t indCu  = gGeoManager->GetMedium("Cu")->GetId();
  
  //half lengths!!! Defines a .2 cm x 20 cm x 20 cm box
  Double_t exphXYZ[3] = { 4.+dx/2,10.,10. };
  Double_t caloXYZ[3] = { dx/2,10,10. };

  Double_t* ubuf(0);
  
  // experimental hall, "world volume"
  TGeoVolume* top = gGeoManager->Volume("EXPH","BOX",indVac,exphXYZ,3);
  gGeoManager->SetTopVolume(top);
  
  // box made of Pb 
  gGeoManager->Volume("CALB","BOX",indCu,caloXYZ,3);
  gGeoManager->Node("CALB",1,"EXPH",0,0.0,0.0,0,kTRUE,ubuf);
}
    

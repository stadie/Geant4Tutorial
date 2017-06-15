/* Definition of a sampling calorimetr 
    consisting of an absorber (Fe or Pb) and and an active material
    scaled size in terms of average hadronic interaction lengh
*/
void SamplingCalorimeter
(Double_t width1 = 2., Double_t width2 = 1.0, Double_t f=1.5, Int_t IAbs=1)
// width1: width of absorber in cm
// width2: width of active material in cm
// f: size scaling factor (depth = f*lambda_i, width = f*lambda_i/2)
// IAbs: Absorber type 1: Pb  2: Fe
//      default values: a typical EM calorimeter
{
  // media
  const TGeoMedium *medVac = gGeoManager->GetMedium("Vacuum");
  const TGeoMedium *medSci = gGeoManager->GetMedium("Scintillator");
  const TGeoMedium *medAbs = (IAbs == 2) ? gGeoManager->GetMedium("Fe") : gGeoManager->GetMedium("Pb");

  Double_t lambda_i_Scint = medSci->GetMaterial()->GetIntLen();
  Double_t lambda_i_Abs= medAbs->GetMaterial()->GetIntLen();
  Double_t x = width1/lambda_i_Abs + (width2/lambda_i_Scint);
  Double_t comb_lambda_i = (width1+width2)/x; //combined interaction length
  std::cout << "lambdas:" << lambda_i_Scint << ", " <<  lambda_i_Abs << "comb:" <<  comb_lambda_i << std::endl;
  // calculate size of calorimeter in terms of combined interaction length
  Double_t overallwidth = comb_lambda_i*f;
  Double_t offsetX      = 0.0;
  Double_t exphXYZ[3]   = {overallwidth, overallwidth/4, overallwidth/4}; 

  Double_t layer1XYZ[3] = {width1 / 2, exphXYZ[1], exphXYZ[2]};
  Double_t layer2XYZ[3] = {width2 / 2, exphXYZ[1], exphXYZ[2]};
  
  Double_t* ubuf(0);
  
  // experimental hall, "world volume"  
  TGeoVolume* top = gGeoManager->MakeBox("EXPH",medVac,exphXYZ[0],exphXYZ[1],exphXYZ[2]);
  gGeoManager->SetTopVolume(top);

  // box for  layer
  TGeoVolume* layer = gGeoManager->MakeBox("Layer",medVac,0.5*(width1+width2), exphXYZ[1], exphXYZ[2]);
  //boxes with absorber and scintillator
  TGeoVolume* abs = gGeoManager->MakeBox("ABS",medAbs, 0.5* width1, exphXYZ[1], exphXYZ[2]); 
  TGeoVolume* sci = gGeoManager->MakeBox("SCI",medSci, 0.5* width2, exphXYZ[1], exphXYZ[2]); 
  //construct layer
  layer->AddNode(abs, 0,new TGeoTranslation(-width2/2,0,0));
  layer->AddNode(sci, 1,new TGeoTranslation(width1/2,0,0));

  int i = 0;
  for(double  x= 0  ; x< exphXYZ[0]-width1 - width2 ; x+= width1+width2) {
    top->AddNode(layer,i,new TGeoTranslation(x,0,0));
    ++i;
  }
}

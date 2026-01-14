#include <iostream>
#include "TSystem.h"
#include "TROOT.h"
#include "TInterpreter.h"

void run_g4_modern() {
  //force single thread 
  gSystem->Setenv("G4FORCENUMBEROFTHREADS", "1");

  TString pwd = gSystem->GetWorkingDirectory();
  TString localInclude = TString::Format("-I%s/include", pwd.Data());

  gSystem->AddIncludePath(localInclude);
  gSystem->AddIncludePath("-I/opt/root_vmc/include/vmc");
  gSystem->AddIncludePath("-I/opt/root_vmc/include");
  gSystem->AddIncludePath("-I/opt/geant4_vmc/include");
  gSystem->AddIncludePath("-I/opt/geant4_vmc/include/geant4vmc");
  gSystem->AddIncludePath("-I/opt/geant4/include/Geant4");

  gInterpreter->AddIncludePath("include");
  gInterpreter->AddIncludePath("/opt/root_vmc/include/vmc");

  gSystem->Load("libgeant4vmc");
  gSystem->Load("libEG");

  std::cout << "Compiling Application..." << std::endl;
  gROOT->ProcessLine(".L src/TutorialStack.cxx++");
  gROOT->ProcessLine(".L src/TGeomWrapper.cc++");
  gROOT->ProcessLine(".L src/TutorialApplication.cxx++");
  gROOT->ProcessLine(".L src/TutorialMainFrame.cxx++");

  gROOT->ProcessLine("#include \"TutorialApplication.hh\"");
  gROOT->ProcessLine("#include \"TutorialMainFrame.hh\"");

  std::cout << "Creating Application..." << std::endl;
  gROOT->ProcessLine("TutorialApplication* app = new TutorialApplication();");

  std::cout << "Launching GUI..." << std::endl;
  gROOT->ProcessLine("new TutorialMainFrame(app);");

  std::cout << "Configuring Geant4..." << std::endl;
  gROOT->ProcessLine(".x g4Config.C");
  
  std::cout << "Initializing MC..." << std::endl;
  gROOT->ProcessLine("app->InitMC(\"geometry/cubox\");");
}

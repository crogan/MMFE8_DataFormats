// C++ includes
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <dirent.h>
#include <ctime>
#include <bitset>
#include <cmath>

// ROOT includes
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>

using namespace std;

int main(int argc, char* argv[]) {
  char inputFileName[400];
  char outputFileName[400];

  if ( argc < 2 ){
    cout << "Error at Input: please specify an input .dat file";
    cout << " and an output filename" << endl;
    cout << "Example:   ./tpfit2root input_file.dat" << endl;
    cout << "Example:   ./tpfit2root input_file.dat -o output_file.root" << endl;
    return 1;
  }
  bool user_output = false;
  for (int i=0;i<argc;i++){
    sscanf(argv[1],"%s", inputFileName);
    if (strncmp(argv[i],"-o",2)==0){
      sscanf(argv[i+1],"%s", outputFileName);
      user_output = true;
    }
  }
  if(!user_output)
    sprintf(outputFileName,"%s.root",inputFileName);

  cout << "Input File:  " << inputFileName << endl;
  cout << "Output File: " << outputFileName << endl;

  string line;
  ifstream ifile(inputFileName);

  // ROOT output tree and branches
  TTree* tree = new TTree("TPfit_data","TPfit_data");

  int Nb = 8;

  int EventNum;
  int cntr;
  int Time_sec;
  int Time_nsec;
  int BCID;
  float mxlocal;

  vector<int> boardOrder = {2,4,3,5,1,0,6,7};
  vector<int> MMFE8Order = {118,116,102,119,106,107,117,105};

  vector<int> tpfit_VMM;
  vector<int> tpfit_CH;
  vector<int> tpfit_MMFE8;

  tree->Branch("EventNum",  &EventNum);
  tree->Branch("cntr",  &cntr);
  tree->Branch("Time_sec",  &Time_sec);
  tree->Branch("Time_nsec", &Time_nsec);
  tree->Branch("BCID", &BCID);
  tree->Branch("mxlocal", &mxlocal);
  tree->Branch("tpfit_VMM", &tpfit_VMM);
  tree->Branch("tpfit_CH", &tpfit_CH);
  tree->Branch("tpfit_MMFE8", &tpfit_MMFE8);

  int p;
  
  // Loop through the entire input file
  if(ifile.is_open()){
    while(getline(ifile,line)){
      string dum;
      std::stringstream sline;
      std::stringstream sline2;
      std::stringstream sline4;
      std::stringstream sline5;
      sline << line;
      sline >> dum;
      sline >> EventNum;
      sline >> dum;
      sline >> Time_sec;
      sline >> dum;
      sline >> Time_nsec;

      tpfit_VMM.clear();
      tpfit_CH.clear();
      tpfit_MMFE8.clear();
      
      if(getline(ifile ,line)) {
        sline2 << line;
        sline2 >> dum;
        sline2 >> dum;
        sline2 >> BCID;
        }
      for(int i = 0; i < Nb; i++){
	      if(getline(ifile,line)){
        std::stringstream sline3;
        sline3.str("");
	      sline3 << line;
        dum = sline3.str();
        sline3 >> p;
	      tpfit_VMM.push_back(p);
	      sline3 >> p;
	      tpfit_CH.push_back(p);
        tpfit_MMFE8.push_back(MMFE8Order[i]);
	      }
      }
      if(getline(ifile ,line)) {
        sline4 << line;
        sline4 >> dum;
        sline4 >> dum;
        sline4 >> mxlocal;
        }
      if(getline(ifile ,line)) {
        sline5 << line;
        sline5 >> dum;
        sline5 >> cntr;
        }
      tree->Fill();
    }
  }

  ifile.close();

  TFile* fout = new TFile(outputFileName, "RECREATE");
  fout->cd();
  tree->Write();
  fout->Close();

  return 0;
}
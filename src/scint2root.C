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
    cout << "Example:   ./scint2root input_file.dat" << endl;
    cout << "Example:   ./scint2root input_file.dat -o output_file.root" << endl;
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
  TTree* tree = new TTree("SCINT_data","SCINT_data");

  int EventNum;
  int Time_sec;
  int Time_nsec;
  int RunNum;

  int N_sci;
  vector<int> sci_CH;
  vector<int> sci_count;

  tree->Branch("EventNum",  &EventNum);
  tree->Branch("RunNum",  &RunNum);
  tree->Branch("Time_sec",  &Time_sec);
  tree->Branch("Time_nsec", &Time_nsec);

  tree->Branch("N_sci", &N_sci);
  tree->Branch("sci_CH", &sci_CH);
  tree->Branch("sci_count", &sci_count);

  int p;
  
  // Loop through the entire input file
  if(ifile.is_open()){
    while(getline(ifile,line)){
      std::stringstream sline;
      sline << line;
      sline >> RunNum;
      sline >> EventNum;
      sline >> Time_sec;
      sline >> Time_nsec;
      sline >> N_sci;

      sci_CH.clear();
      sci_count.clear();

      for(int i = 0; i < N_sci; i++){
	if(getline(ifile,line)){
	  sline << line;
	  sline >> p;
	  sci_CH.push_back(p);
	  sline >> p;
	  sci_count.push_back(p);
	}
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

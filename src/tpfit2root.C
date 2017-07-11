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
#include <chrono>

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
    cout << "Example:   ./tpfit2root input_file.dat -r runnumber" << endl;
    cout << "Example:   ./tpfit2root input_file.dat -o output_file.root -r runnumber" << endl;
    return 1;
  }
  bool user_output = false;
  int RunNum = -1;
  for (int i=0;i<argc;i++){
    sscanf(argv[1],"%s", inputFileName);
    if (strncmp(argv[i],"-o",2)==0){
      sscanf(argv[i+1],"%s", outputFileName);
      user_output = true;
    }
    if (strncmp(argv[i],"-r",2)==0){
      RunNum = atoi(argv[i+1]);
    }
  }
  if(!user_output)
    sprintf(outputFileName,"%s.root",inputFileName);

  cout << "Input File:  " << inputFileName << endl;
  cout << "Output File: " << outputFileName << endl;
  cout << "Run Number: " << RunNum << endl;

  string line;
  ifstream ifile(inputFileName);

  // ROOT output tree and branches
  TFile* fout = new TFile(outputFileName, "RECREATE");
  TTree* tree = new TTree("TPfit_data","TPfit_data");

  int Nb = 8;

  int EventNum;
  int cntr;
  int Time_sec;
  int Time_nsec;
  int BCID;
  float mxlocal;

  vector<int> MMFE8Order;
  vector<int> MMFE8Order_3525 = {118,111,120,119,106,107,101,105};
  vector<int> MMFE8Order_3524 = {118,116,102,119,106,107,101,105};
  vector<int> MMFE8Order_3518 = {118,116,102,119,106,107,117,105};
  vector<int> boardOrder = {5, 3, 4, 2, 7, 6, 1, 0};
  if (RunNum >= 3525){
    MMFE8Order = MMFE8Order_3525;
  }
  else if (RunNum >= 3524)
    MMFE8Order = MMFE8Order_3524;
  else if (RunNum >= 3518)
    MMFE8Order = MMFE8Order_3518;
  else {
    cout << "Put in MMFE8 config!" << endl;
    return 1;
  }
  cout << "Using board IPs (from ind 0 to 7): " << endl;
  for (int i = 0; i < Nb; i++){
    cout << MMFE8Order[i] << " ";
  }
  cout << endl;
  cout << endl;

  vector<int> tpfit_VMM;
  vector<int> tpfit_CH;
  vector<int> tpfit_MMFE8;
  int tpfit_n;

  tree->Branch("EventNum",  &EventNum);
  tree->Branch("cntr",  &cntr);
  tree->Branch("Time_sec",  &Time_sec);
  tree->Branch("Time_nsec", &Time_nsec);
  tree->Branch("BCID", &BCID);
  tree->Branch("mxlocal", &mxlocal);
  tree->Branch("tpfit_VMM", &tpfit_VMM);
  tree->Branch("tpfit_CH", &tpfit_CH);
  tree->Branch("tpfit_MMFE8", &tpfit_MMFE8);
  tree->Branch("tpfit_n", &tpfit_n);

  std::chrono::time_point<std::chrono::system_clock> time_start, time_now;
  std::chrono::duration<double> elapsed_seconds;
  time_start = std::chrono::system_clock::now();
  float rate = 0.0;

  int p;
  int p2;
  
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
      tpfit_n = 0;
      for(int i = 0; i < Nb; i++){
	      if(getline(ifile,line)){
                std::stringstream sline3;
                sline3.str("");
                sline3 << line;
                dum = sline3.str();
                sline3 >> p;
                sline3 >> p2;
                if (p != 0 || p2 != 0) {
                  tpfit_VMM.push_back(p);
                  tpfit_CH.push_back(p2);
                  tpfit_MMFE8.push_back(MMFE8Order[boardOrder[i]]);
                  tpfit_n++;
                }
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

      if (EventNum % 10000 == 0){
        time_now = std::chrono::system_clock::now();
        elapsed_seconds = time_now - time_start;
        rate = (float)(EventNum)/elapsed_seconds.count();
        std::cout << "Processing event " << EventNum << ", rate = " << rate << "Hz\r" << std::flush;
      }
    }
  }

  std::cout << std::endl;
  ifile.close();

  tree->Write("", TObject::kOverwrite);
  fout->Close();

  return 0;
}

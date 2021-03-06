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

long gray_to_int(long gray, int nbit = 16){
  std::bitset<16> b_gray(gray);
  std::bitset<16> b_int(gray);

  for(int i = 15; i > 0; i--){
    if(b_int[i] == b_gray[i-1])
      b_int[i-1] = 0;
    else
      b_int[i-1] = 1;
  }
  return b_int.to_ulong();
}

/// Main function that runs the analysis algorithm on the
/// specified input files
int main(int argc, char* argv[]) {
  char inputFileName[400];
  char outputFileName[400];

  if ( argc < 2 ){
    cout << "Error at Input: please specify an input .dat file";
    cout << " and an output filename" << endl;
    cout << "Example:   ./raw2root input_file.dat" << endl;
    cout << "Example:   ./raw2root input_file.dat -o output_file.root" << endl;
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

  long billion = pow(10,9);

  string line;
  ifstream ifile(inputFileName);

  // ROOT output tree and branches
  TTree* tree = new TTree("MM_data","MM_data");

  int EventNum;
  int Time_sec;
  int Time_nsec;

  int N_mm = 0;
  vector<int> trig_BCID;
  vector<int> trig_ph;
  vector<int> mm_VMM;
  vector<int> mm_CH;
  vector<int> mm_PDO;
  vector<int> mm_TDO;
  vector<int> mm_BCID;
  vector<int> mm_MMFE8;
  vector<int> mm_FIFOcount;

  tree->Branch("EventNum",  &EventNum);
  tree->Branch("Time_sec",  &Time_sec);
  tree->Branch("Time_nsec", &Time_nsec);
  tree->Branch("trig_BCID", &trig_BCID);
  tree->Branch("trig_ph", &trig_ph);

  tree->Branch("N_mm", &N_mm);
  tree->Branch("mm_VMM", &mm_VMM);
  tree->Branch("mm_CH", &mm_CH);
  tree->Branch("mm_PDO", &mm_PDO);
  tree->Branch("mm_TDO", &mm_TDO);
  tree->Branch("mm_BCID", &mm_BCID);
  tree->Branch("mm_MMFE8", &mm_MMFE8);
  tree->Branch("mm_FIFOcount", &mm_FIFOcount);

  long machinetime;
  float mt;
  int fifocount;
  int cycle;
  long fifotrig;
  long num_trig;
  long bcid_trig;
  long ph_trig;

  string sword0;
  string sword1;
  long iword0;
  long iword1;
  
  long bcid_int;
  long bcid_gray;
  
  int MMFE8;
  int VMM;
  int CH;
  int TDO;
  int PDO;

  int eventnum = -1;

  // Loop through the entire input file
  if(ifile.is_open()){
    while(getline(ifile,line)){
      // Read line into buffer
      char sline[1000];
      sprintf(sline,"%s",line.c_str());
      // Break up line by spaces
      char* p = strtok(sline, " ");
      int itok = 0;
      while(p){

	if(itok == 0){
	  std::stringstream sfifo;
	  sfifo << p;
	  sfifo >> MMFE8;
	  sfifo >> machinetime;
	  sfifo >> mt;
	  sfifo >> fifocount;
	  sfifo >> cycle;
	  sfifo >> std::hex >> fifotrig;
	  num_trig = fifotrig & 65535;
	  //num_trig = fifotrig & 1048575;
      fifotrig = fifotrig >> 16;
      ph_trig = fifotrig & 7;
      fifotrig = fifotrig >> 4;
	  bcid_trig = fifotrig & 4095;

	  if(eventnum > 0 && num_trig != eventnum){
	    tree->Fill();
        trig_BCID.clear();
        trig_ph.clear();
	    mm_VMM.clear();
	    mm_CH.clear();
	    mm_PDO.clear();
	    mm_TDO.clear();
	    mm_BCID.clear();
	    mm_MMFE8.clear();
	    mm_FIFOcount.clear();
	    N_mm = 0;
	  }
	  eventnum = num_trig;

	  EventNum = num_trig + pow(2,20)*cycle;
	  Time_sec = machinetime/billion;
	  Time_nsec = machinetime%billion;
	}

	if(itok > 1){
	  if(itok%2 == 0){
	    std::stringstream sword;
	    sword << std::hex << p;
	    sword >> iword0;
	    sword0 = string(p);
	  }
	  else {
	    std::stringstream sword;
	    sword << std::hex << p;
	    sword >> iword1;
	    sword1 = string(p);

	    iword0 = iword0 >> 2;
	    CH = (iword0 & 63) + 1;
	    if(iword0 <= 0)
	      CH = 0;
	    iword0 = iword0 >> 6;
	    PDO = iword0 & 1023;
	    iword0 = iword0 >> 10;
	    TDO = iword0 & 255;
	    iword0 = iword0 >> 8;
	    VMM = iword0 & 7;

	    bcid_gray = iword1 & 4095;
	    bcid_int = gray_to_int(bcid_gray);

	    mm_VMM.push_back(VMM);
	    mm_CH.push_back(CH);
	    mm_PDO.push_back(PDO);
	    mm_TDO.push_back(TDO);
	    mm_BCID.push_back(bcid_int);
	    mm_MMFE8.push_back(MMFE8);
        trig_BCID.push_back(bcid_trig);
        trig_ph.push_back(ph_trig);
	    mm_FIFOcount.push_back(fifocount/2);
	    N_mm++;
	  }
	}
    
      	p = strtok(NULL, " ");
	itok++;
      } // End of line read
    }
    if(mm_VMM.size() > 0)
      tree->Fill();
  }

  ifile.close();

  TFile* fout = new TFile(outputFileName, "RECREATE");
  fout->cd();
  tree->Write();
  fout->Close();

  return 0;
}

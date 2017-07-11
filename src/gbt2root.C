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
#include <string>
#include <chrono>

// ROOT includes
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>

using namespace std;
bool debug = false;

int main(int argc, char* argv[]) {
  char inputFileName[400];
  char outputFileName[400];
  int RunNum;
  if ( argc < 2 ){
    cout << "Error at Input: please specify an input .dat file";
    cout << " and an output filename" << endl;
    cout << "Example:   ./gbt2root input_file.dat -r runnum" << endl;
    cout << "Example:   ./gbt2root input_file.dat -o output_file.root -r runnum" << endl;
    return 1;
  }
  bool user_output = false;
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
  cout << "RunNumber: " << RunNum << endl;

  string line;
  ifstream ifile(inputFileName);

  // ROOT output tree and branches
  TFile* fout = new TFile(outputFileName, "RECREATE");
  TTree* tree = new TTree("GBT_data","GBT_data");

  int Nb = 8;
  int Nbuff = 30; // length of GBT buffer sent per trigger

  int EventNum;
  int Time_sec;
  int Time_nsec;

  vector<int> boardOrder = {2,4,3,5,1,0,6,7};
  vector<int> MMFE8Order;
  vector<int> MMFE8Order_3525 = {118,111,120,119,106,107,101,105};
  vector<int> MMFE8Order_3524 = {118,116,102,119,106,107,101,105};
  vector<int> MMFE8Order_3518 = {118,116,102,119,106,107,117,105};
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
  cout << "NB: This assumes GBT packets go AB, AB... (where A is 4-7, B is 0-3)" << endl;
  vector<int> gbt_VMM;
  vector<int> gbt_CH;
  vector<int> gbt_MMFE8;
  vector<int> gbt_BCID;

  tree->Branch("EventNum",  &EventNum);
  tree->Branch("Time_sec",  &Time_sec);
  tree->Branch("Time_nsec", &Time_nsec);
  tree->Branch("gbt_VMM", &gbt_VMM);
  tree->Branch("gbt_CH", &gbt_CH);
  tree->Branch("gbt_MMFE8", &gbt_MMFE8);
  tree->Branch("gbt_BCID", &gbt_BCID);

  bool TPfromA = false;
  if (TPfromA)
    cout << "Taking BCID from first packet in GBT buffer" << endl;
  else
    cout << "Taking BCID from second packet in GBT buffer" << endl;
  
  int bcid;
  int TPbcid;
  int nhits;
  int nhitscycle;
  int vmm;
  int ch;
  int ib;
  bool Aflag = false;
  int nevent = 0;
  
  std::chrono::time_point<std::chrono::system_clock> time_start, time_now;
  std::chrono::duration<double> elapsed_seconds;
  time_start = std::chrono::system_clock::now();
  float rate = 0.0;

  std::stringstream sline;
  std::stringstream sline2;
  std::stringstream sline3;
  // Loop through the entire input file
  if(ifile.is_open()){
    string dum;
    while(getline(ifile,line)){ 
      sline2.clear();
      if (line.substr(0,5) != "Event") {
        if (debug)
          cout << "line: " << line << endl;
        if (!Aflag)
          Aflag = true;
        else
          Aflag = false;
        sline2 << line;
        sline2 >> dum;
        sline2 >> bcid;
        if (!Aflag)
          if (!TPfromA)
            TPbcid = bcid;
        if (Aflag)
          if (TPfromA)
            TPbcid = bcid;
        sline2 >> dum;
        sline2 >> nhits;
        if (Aflag)
          nhitscycle = nhits;
        else
          nhitscycle = nhitscycle + nhits;
        for (int j = 0; j < 4; j++){
          sline3.clear();
          if(getline(ifile,line)){
            sline3 << line;
            sline3 >> ib;
            if (Aflag)
              ib = ib + 4;
            if (debug){
              cout << "line: " << line << endl;
              cout << "ib: " << ib << endl;
            }
            string hitvmm;
            while (sline3 >> hitvmm) {
              if (debug){
                cout << "hitvmm " << hitvmm << endl;
              }
              int vmm, ch;
              vmm = atoi(hitvmm.substr(0,hitvmm.find(",")).c_str());
              ch = atoi(hitvmm.substr(hitvmm.find(",")+1).c_str());
              gbt_VMM.push_back(vmm);
              gbt_CH.push_back(ch);
              gbt_MMFE8.push_back(MMFE8Order[ib]);
            }
          }
        }
        if (!Aflag)
          for (int k = 0; k < nhitscycle; k++)
            gbt_BCID.push_back(TPbcid);
      } // if not event
      else {
        if (nevent != 0){
          tree->Fill();
          sline.clear();
          sline2.clear();
          sline3.clear();
        }
        nevent++;
        if (nevent % 10000 == 0){
          time_now = std::chrono::system_clock::now();
          elapsed_seconds = time_now - time_start;
          rate = (float)(nevent)/elapsed_seconds.count();
          std::cout << "Processing event " << nevent << ", rate = " << rate << "Hz\r" << std::flush;
        }
        sline.clear();
        sline << line;
        sline >> dum;
        sline >> EventNum;
        sline >> dum;
        sline >> Time_sec;
        sline >> dum;
        sline >> Time_nsec;
        if (debug)
          cout << "EventNum: " << EventNum << endl;
        gbt_VMM.clear();
        gbt_CH.clear();
        gbt_MMFE8.clear();
        gbt_BCID.clear();
      }
    }
    tree->Fill();
  }

  std::cout << std::endl;
  ifile.close();

  tree->Write("", TObject::kOverwrite);
  fout->Close();

  return 0;
}

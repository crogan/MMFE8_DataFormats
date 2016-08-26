// C++ includes
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <dirent.h>
#include <ctime>

// ROOT includes
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>

using namespace std;

/// Main function that runs the analysis algorithm on the
/// specified input files
int main(int argc, char* argv[]) {
  char inputFileName[400];
  char outputFileName[400];

  if ( argc < 2 ){
    cout << "Error at Input: please specify an input .dat file";
    cout << " and an output filename" << endl;
    cout << "Example:   ./dat2root input_file.dat" << endl;
    cout << "Example:   ./dat2root input_file.dat -o output_file.root" << endl;
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

  vector<string> sVARv;
  sVARv.push_back("MMFE8");
  sVARv.push_back("VMM");
  sVARv.push_back("CHword");
  sVARv.push_back("CHpulse");
  sVARv.push_back("PDO");
  sVARv.push_back("TDO");
  sVARv.push_back("BCID");
  sVARv.push_back("BCIDgray");
  sVARv.push_back("BCIDtrig");
  sVARv.push_back("TPDAC");
  sVARv.push_back("THDAC");
  sVARv.push_back("Delay");
  sVARv.push_back("TACslope");
  sVARv.push_back("PeakTime");
  sVARv.push_back("PulseNum");
  sVARv.push_back("FIFO");
  sVARv.push_back("Ntrig");

  vector<string> sVARx;
  // sVARx.push_back("MMFE8");
  sVARx.push_back("VMM");
  sVARx.push_back("CKTPrunning");
  sVARx.push_back("PDAC");
  sVARx.push_back("XADC");

  int Nvarv = sVARv.size();
  vector<int> vVARv (Nvarv, 0);
  int Nvarx = sVARx.size();
  vector<int> vVARx (Nvarx, 0);

  string line;
  ifstream ifile(inputFileName);

  TFile* ofile = new TFile(outputFileName,"RECREATE");
  ofile->cd();
  TTree* vtree = new TTree("VMM_data","VMM_data");
  TTree* xtree = new TTree("xADC_data","xADC_data");
  TTree* time_tree = new TTree("date", "Calibration date");

  for(int i = 0; i < Nvarv; i++){
    vtree->Branch(sVARv[i].c_str(), &vVARv[i]);
  }
  for (int i = 0; i < Nvarx; i++){
    xtree->Branch(sVARx[i].c_str(), &vVARx[i]);
  }

  vector<bool> vFoundv (Nvarv, false);
  vector<bool> vFoundx (Nvarx, false);

  // Loop through the entire input file
  if(ifile.is_open()){
    while(getline(ifile,line)){
      // Flags for which kind of line it is
      int num_xadc_matches = 0;
      int num_vmm_matches = 0;
      
      // check if variables found in the line
      for(int v = 0; v < Nvarv; v++)
	vFoundv[v] = false;
      for(int x = 0; x < Nvarx; x++)
	vFoundx[x] = false;

      // Read into buffer
      char sline[1000];
      sprintf(sline,"%s",line.c_str());
      // Break up line by spaces
      char* p = strtok(sline, " ");
      while(p){
        if (num_xadc_matches < 3) {
          for(int v = 0; v < Nvarv; v++){
	    if(strncmp((sVARv[v]+"=").c_str(),p,sVARv[v].length()+1)==0){
	      sscanf(p,(sVARv[v]+"=%d").c_str(), &vVARv[v]);
              num_vmm_matches++;
	      vFoundv[v] = true;
	      break;
	    }
	  } // End vmm part
        } if (num_vmm_matches < 3) {
          for (int x = 0; x < Nvarx; x++) {
            if(strncmp(sVARx[x].c_str(),p,sVARx[x].length())==0){
	      sscanf(p,(sVARx[x]+"=%d").c_str(), &vVARx[x]);
              num_xadc_matches++;
	      vFoundv[x] = true;
	      break;
	    }
          }
        } // End xadc part
      	p = strtok(NULL, " ");
      } // End of line read

      if (num_vmm_matches > 1){
	// set unfound vars to -1
	for(int v = 0; v < Nvarv; v++)
	  if(vFoundv[v] == false)
	    vVARv[v] = -1;
        vtree->Fill();
        //printf("%s -> VMM\n", line.c_str());
      } else {//if (num_xadc_matches > Nvarx)
	for(int x = 0; x < Nvarx; x++)
	  if(vFoundx[x] == false)
	    vVARx[x] = -1;
        xtree->Fill();
        //printf("%s -> xADC\n", line.c_str());
      }
    }
  }

  // Grab the current date and time, store them as YYYY MM DD integers
  time_t current_time = time(NULL);
  tm* timestamp = localtime(&current_time);
  int year = timestamp->tm_year + 1900;
  time_tree->Branch("Year",&year);
  time_tree->Branch("Month",&(timestamp->tm_mon));
  time_tree->Branch("Day",&(timestamp->tm_mday));
  time_tree->Fill();

  // Always write the date tree; write the VMM data and xADC data trees if full.
  ofile->cd();
  time_tree->Write();
  if (vtree->GetEntries() > 0){
    vtree->Write();
  }
  if (xtree->GetEntries() > 0){
    xtree->Write();
  }
  ofile->Close();

  return 0;
}

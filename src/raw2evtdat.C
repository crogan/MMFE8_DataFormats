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
    cout << "Example:   ./raw2dat input_file.dat" << endl;
    cout << "Example:   ./raw2dat input_file.dat -o output_file.dat" << endl;
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
    sprintf(outputFileName,"%s.dat",inputFileName);

  cout << "Input File:  " << inputFileName << endl;
  cout << "Output File: " << outputFileName << endl;

  long billion = pow(10,9);

  string line;
  ifstream ifile(inputFileName);
  ofstream ofile;
  ofile.open(outputFileName);

  long eventnum = 0;
  long num_trig = -1;
  long num_trig_new = -1;
  long bcid_trig;
  long bcid_trig_new = -1;

  // Loop through the entire input file
  if(ifile.is_open()){
    while(getline(ifile,line)){

      long machinetime;
      int fifocount;
      int cycle;
      long fifotrig;
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
	  float mt;
	  sfifo >> machinetime;
	  sfifo >> mt;
	  sfifo >> fifocount;
	  sfifo >> cycle;
	  sfifo >> std::hex >> fifotrig;
	  num_trig_new = fifotrig & 65535;
	  fifotrig = fifotrig >> 16;
      ph_trig = fifotrig & 7;
      fifotrig = fifotrig >> 4;
	  bcid_trig_new = fifotrig & 4095;
      if (num_trig_new != num_trig){
        num_trig = num_trig_new;
      }
      // if (num_trig_new != num_trig && (MMFE8 != 122 && MMFE8 != 126 && MMFE8 != 106 && MMFE8 != 124 && MMFE8 != 119)){
      //   num_trig = num_trig_new;
      // }
      // else if (num_trig_new != num_trig && (MMFE8 == 122 || MMFE8 == 126 || MMFE8 == 106 || MMFE8 == 124) && (fabs(bcid_trig_new-bcid_trig) > 1)){
      //   num_trig = num_trig_new;
      // }
      bcid_trig = bcid_trig_new;
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

	    if(num_trig != eventnum){
          ofile << "EventNum " << num_trig + pow(2,16)*cycle;
	      ofile << " Sec " << machinetime/billion;
	      ofile << " NS " << machinetime%billion;
	      //ofile << " BCIDtrig " << bcid_trig;
	      ofile << endl;
	    }
          
	    eventnum = num_trig;

	    ofile << std::setw(3) << left << VMM;
	    ofile << std::setw(4) << left << CH;
	    ofile << std::setw(6) << left << PDO;
	    ofile << std::setw(6) << left << TDO;
	    ofile << std::setw(6) << left << bcid_int;
	    ofile << std::setw(5) << left << MMFE8;
	    ofile << std::setw(6) << left << bcid_trig;
	    ofile << std::setw(3) << left << ph_trig;
	    ofile << std::setw(4) << left << fifocount/2;
	    ofile << endl;
	  }
	}
    
      	p = strtok(NULL, " ");
	itok++;
      } // End of line read
    }
  }

  ifile.close();
  ofile.close();

  return 0;
}

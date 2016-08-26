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

#include "include/DACToCharge.hh"

using namespace std;

/// Main function that runs the analysis algorithm on the
/// specified input files
int main(int argc, char* argv[]) {
  char inputFileName[400];
  char outputFileName[400];

  if ( argc < 2 ){
    cout << "Error at Input: please specify an input .root file which";
    cout << " contains xADCcalib tree" << endl;
    cout << "Example:   ./xADCcalib2dat input_file.root" << endl;
    cout << "Example:   ./xADCcalib2dat input_file.root -o output_file.dat" << endl;
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

  // xADC calibration object
  DACToCharge DAC2Charge(inputFileName);
  DAC2Charge.PrintToFile(outputFileName);

  return 0;
}

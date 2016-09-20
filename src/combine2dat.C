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

#include "include/MMdataBase.hh"
#include "include/SCINTdataBase.hh"

using namespace std;

bool g_out;
bool g_mdat;
bool g_mroot;
bool g_sdat;
bool g_sroot;
int g_offset;

MMdataBase* g_MMbase;
int g_N_MMbase;
int g_i_MMbase;

ifstream* g_MMstream;
string g_MMline;

SCINTdataBase* g_SCINTbase;
int g_N_SCINTbase;
int g_i_SCINTbase;

ifstream* g_SCINTstream;
string g_SCINTline;

ofstream* output_file;

int RunNum;
int sci_EventNum;
int sci_Time_sec;
int sci_Time_nsec;
int mm_EventNum;
int mm_Time_sec;
int mm_Time_nsec;
int mm_trig_BCID;

int N_sci;
vector<int> sci_CH;
vector<int> sci_count;

int N_mm;
vector<int> mm_VMM;
vector<int> mm_CH;
vector<int> mm_PDO;
vector<int> mm_TDO;
vector<int> mm_BCID;
vector<int> mm_MMFE8;
vector<int> mm_FIFOcount;

bool Initialize_MM(const string& filename);
bool Initialize_SCINT(const string& filename);
bool Initialize_output(const string& filename);

bool NextMMEvent();
bool NextSCINTEvent();

void WriteEvent();

void Close_MM();
void Close_SCINT();
void Write_output();

void print_error(){
  cout << "Please specify a micromega data input ";
  cout << "(.dat with -mdat flag, .root with -mroot flag)";
  cout << ", a scintilator data input ";
  cout << "(.dat with -sdat flag, .roort with -sroot flag)";
  cout << " and an output file name (with -out flag)." << endl;
  cout << "Example:   ./combine2dat -mdat mm_input.dat -sdat scint_input.dat -out output_file.root" << endl;
  cout << "Example:   ./combine2dat -mroot mm_input.root -sdat scint_input.dat -out output_file.root" << endl;
  cout << "Example:   ./combine2dat -mdat mm_input.dat -sroot scint_input.root -out output_file.root" << endl;
  cout << "Example:   ./combine2dat -mroot mm_input.root -sroot scint_input.root -out output_file.root" << endl;
  cout << "Example:   ./combine2dat -mdat mm_input.dat -sdat scint_input.dat -out output_file.root -offset N" << endl;
}

int main(int argc, char* argv[]) {
  char inputMMFileName[400];
  char inputSCINTFileName[400];
  char outputFileName[400];

  if ( argc < 7 ){
    cout << "Input Error: insufficient number of arguments" << endl;
    print_error();
    return 1;
  }
 
  g_out   = false;
  g_mdat  = false;
  g_mroot = false;
  g_sdat  = false;
  g_sroot = false;
  g_offset = 0;

  for (int i=1; i<argc-1;i++){
    if(strncmp(argv[i],"-mdat",5)==0){
      sscanf(argv[i+1],"%s", inputMMFileName);
      g_mdat = true;
    }
    if(strncmp(argv[i],"-sdat",5)==0){
      sscanf(argv[i+1],"%s", inputSCINTFileName);
      g_sdat = true;
    }
    if(strncmp(argv[i],"-mroot",6)==0){
      sscanf(argv[i+1],"%s", inputMMFileName);
      g_mroot = true;
    }
    if(strncmp(argv[i],"-sroot",6)==0){
      sscanf(argv[i+1],"%s", inputSCINTFileName);
      g_sroot = true;
    }
    if(strncmp(argv[i],"-out",4)==0){
      sscanf(argv[i+1],"%s", outputFileName);
      g_out = true;
    }
    if(strncmp(argv[i],"-offset",7)==0){
      g_offset = atoi(argv[i+1]);
    }
  }

  if(!g_out){
    cout << "Input Error: no output file specified" << endl;
    print_error();
    return 1;
  }
  if(g_mdat && g_mroot){
    cout << "Input Error: cannot input micromega data in both .dat and .root format" << endl;
    print_error(); 
    return 1;
  }
  if(g_sdat && g_sroot){
    cout << "Input Error: cannot input scintilator data in both .dat and .root format" << endl;
    print_error();
    return 1;
  }
  if(!g_mdat && !g_mroot){
    cout << "Input Error: missing micromega data input" << endl;
    print_error();
    return 1;
  }
  if(!g_sdat && !g_sroot){
    cout << "Input Error: missing scintilator data input" << endl;
    print_error();
    return 1;
  }
  
  cout << "Micromega Input File:  " << inputMMFileName << endl;
  cout << "Scintilator Input File:  " << inputSCINTFileName << endl;
  cout << "Output File: " << outputFileName << endl;

  if(!Initialize_output(outputFileName))
    return 1;

  if(!Initialize_MM(inputMMFileName))
    return 1;

  if(!Initialize_SCINT(inputSCINTFileName))
    return 1;
  
  if(!NextMMEvent())
    return 1;
  
  if(!NextSCINTEvent())
    return 1;

  bool bgood = true;
  while(bgood){
    if(sci_EventNum == mm_EventNum + g_offset){
      WriteEvent();
      if(!NextMMEvent())
	bgood = false;
    } else {
      if(sci_EventNum > mm_EventNum + g_offset){
	if(!NextMMEvent())
	  bgood = false;
      } else {
	if(!NextSCINTEvent())
	  bgood = false;
      }
    }
  }
  
  Close_MM();
  Close_SCINT();
  Write_output();

  return 0;
}

bool Initialize_MM(const string& filename){
  if(g_mdat){ //.dat format input
    g_MMstream = new ifstream(filename);
    if(!g_MMstream || !g_MMstream->is_open()){
      cout << "Error: unable to open input file " << filename << endl;
      return false;
    }
    while(true){
      if(!getline(*g_MMstream, g_MMline))
	return false;
      if(strncmp(g_MMline.c_str(),"EventNum",8)==0)
	break;
    }
  }
  if(g_mroot){ //.root format input
    TFile* f = new TFile(filename.c_str(), "READ");
    if(!f){
      cout << "Error: unable to open input file " << filename << endl;
      return false;
    }
    TTree* T = (TTree*) f->Get("MM_data");
    if(!T){
      cout << "Error: cannot find tree MM_data in " << filename << endl;
      return false;
    }
    g_MMbase = (MMdataBase*) new MMdataBase(T);
    g_N_MMbase = T->GetEntries();
    g_i_MMbase = -1;
    g_MMbase->EventNum = 0;
  }
  
  return true;
}

bool Initialize_SCINT(const string& filename){
  if(g_sdat){ //.dat format input
    g_SCINTstream = new ifstream(filename);
    if(!g_SCINTstream){
      cout << "Error: unable to open input file " << filename << endl;
      return false;
    }
  }
  if(g_sroot){ //.root format input
    TFile* f = new TFile(filename.c_str(), "READ");
    if(!f){
      cout << "Error: unable to open input file " << filename << endl;
      return false;
    }
    TTree* T = (TTree*) f->Get("SCINT_data");
    if(!T){
      cout << "Error: cannot find tree SCINT_data in " << filename << endl;
      return false;
    }
    g_SCINTbase = (SCINTdataBase*) new SCINTdataBase(T);
    g_N_SCINTbase = T->GetEntries();
    g_i_SCINTbase = -1;
    g_SCINTbase->EventNum = 0;
  }
  
  return true;
}

bool Initialize_output(const string& filename){
  output_file = new ofstream();
  output_file->open(filename.c_str());

  *output_file << "Start event: ";
  *output_file << std::setw(7) << left << "run";
  *output_file << std::setw(9) << left << "sc_evt";
  *output_file << std::setw(9) << left << "mm_evt";
  *output_file << std::setw(12) << left << "sc_time_s";
  *output_file << std::setw(12) << left << "mm_time_s";
  *output_file << std::setw(12) << left << "sc_time_ns";
  *output_file << std::setw(12) << left << "mm_time_ns";
  *output_file << std::setw(13) << left << "mm_trig_bcid";
  *output_file << std::setw(8) << left << "sc_hits";
  *output_file << std::setw(8) << left << "mm_hits" << endl;

  return true;
}

void Write_output(){
  if(output_file){
    output_file->close();
    output_file = nullptr;
  }
}

bool NextMMEvent(){
  if(g_mdat){
    if(!g_MMstream || !g_MMstream->is_open())
      return false;

    if(!(strncmp(g_MMline.c_str(),"EventNum",8)==0))
      return false;

    string dum;
    std::stringstream sline;
    sline << g_MMline;
    sline >> dum;
    sline >> mm_EventNum;
    sline >> dum;
    sline >> mm_Time_sec;
    sline >> dum;
    sline >> mm_Time_nsec;
    sline >> dum;
    sline >> mm_trig_BCID;
    
    N_mm = 0;
    mm_VMM.clear();
    mm_CH.clear();
    mm_PDO.clear();
    mm_TDO.clear();
    mm_BCID.clear();
    mm_MMFE8.clear();
    mm_FIFOcount.clear();

    int VMM;
    int CH;
    int PDO;
    int TDO;
    int BCID;
    int MMFE8;
    int FIFOcount;

    while(true){
      if(!getline(*g_MMstream, g_MMline)){
	g_MMline = "";
	return true;
      }

      if(strncmp(g_MMline.c_str(),"EventNum",8)==0){
	return true;
      }

      std::stringstream mline;
      mline << g_MMline;
      mline >> VMM;
      mline >> CH;
      mline >> PDO;
      mline >> TDO;
      mline >> BCID;
      mline >> MMFE8;
      mline >> FIFOcount;
      N_mm++;
      
      mm_VMM.push_back(VMM);
      mm_CH.push_back(CH);
      mm_PDO.push_back(PDO);
      mm_TDO.push_back(TDO);
      mm_BCID.push_back(BCID);
      mm_MMFE8.push_back(MMFE8);
      mm_FIFOcount.push_back(FIFOcount);
    }
  }

  if(g_mroot){
    g_i_MMbase++;
    if(g_i_MMbase >= g_N_MMbase)
      return false;
    g_MMbase->GetEntry(g_i_MMbase);

    mm_EventNum = g_MMbase->EventNum;
    mm_Time_sec = g_MMbase->Time_sec;
    mm_Time_nsec = g_MMbase->Time_nsec;
    mm_trig_BCID = g_MMbase->trig_BCID;

    N_mm = g_MMbase->N_mm;
    mm_VMM.clear();
    mm_CH.clear();
    mm_PDO.clear();
    mm_TDO.clear();
    mm_BCID.clear();
    mm_MMFE8.clear();
    mm_FIFOcount.clear();

    for(int i = 0; i < N_mm; i++){
      mm_VMM.push_back(g_MMbase->mm_VMM->at(i));
      mm_CH.push_back(g_MMbase->mm_CH->at(i));
      mm_PDO.push_back(g_MMbase->mm_PDO->at(i));
      mm_TDO.push_back(g_MMbase->mm_TDO->at(i));
      mm_BCID.push_back(g_MMbase->mm_BCID->at(i));
      mm_MMFE8.push_back(g_MMbase->mm_MMFE8->at(i));
      mm_FIFOcount.push_back(g_MMbase->mm_FIFOcount->at(i));
    }
  }

  return true;
}

bool NextSCINTEvent(){
  if(g_sdat){
    if(!g_SCINTstream || !g_SCINTstream->is_open())
      return false;

    if(!getline(*g_SCINTstream, g_SCINTline))
      return false;

    std::stringstream sline;
    sline << g_SCINTline;
    sline >> RunNum;
    sline >> sci_EventNum;
    sline >> sci_Time_sec;
    sline >> sci_Time_nsec;
    sline >> N_sci;

    sci_CH.clear();
    sci_count.clear();

    int p;

    for(int i = 0; i < N_sci; i++){
      if(getline(*g_SCINTstream, g_SCINTline)){
	sline << g_SCINTline;
	sline >> p;
	sci_CH.push_back(p);
	sline >> p;
	sci_count.push_back(p);
      } else{
	return false;
      }
    }
  }

  if(g_sroot){
    g_i_SCINTbase++;
    if(g_i_SCINTbase >= g_N_SCINTbase)
      return false;
    g_SCINTbase->GetEntry(g_i_SCINTbase);

    RunNum = g_SCINTbase->RunNum;
    sci_EventNum = g_SCINTbase->EventNum;
    sci_Time_sec = g_SCINTbase->Time_sec;
    sci_Time_nsec = g_SCINTbase->Time_nsec;

    N_sci = g_SCINTbase->N_sci;
    sci_CH.clear();
    sci_count.clear();

    for(int i = 0; i < N_sci; i++){
      sci_CH.push_back(g_SCINTbase->sci_CH->at(i));
      sci_count.push_back(g_SCINTbase->sci_count->at(i));
    }
  }

  return true;
}

void WriteEvent(){
  *output_file << "Start event: ";
  *output_file << std::setw(7) << left << RunNum;
  *output_file << std::setw(9) << left << sci_EventNum;
  *output_file << std::setw(9) << left << mm_EventNum;
  *output_file << std::setw(12) << left << sci_Time_sec;
  *output_file << std::setw(12) << left << mm_Time_sec;
  *output_file << std::setw(12) << left << sci_Time_nsec;
  *output_file << std::setw(12) << left << mm_Time_nsec;
  *output_file << std::setw(13) << left << mm_trig_BCID;
  *output_file << std::setw(8) << left << N_sci;
  *output_file << std::setw(8) << left << N_mm << endl;

  for(int i = 0; i < N_sci; i++){
    *output_file << std::setw(3) << left << sci_CH[i];
    *output_file << std::setw(5) << left << sci_count[i] << endl;
  }
  for(int i = 0; i < N_mm; i++){
    *output_file << "MM "; 
    *output_file << std::setw(2) << left << mm_VMM[i];
    *output_file << std::setw(3) << left << mm_CH[i];
    *output_file << std::setw(5) << left << mm_PDO[i];
    *output_file << std::setw(5) << left << mm_TDO[i];
    *output_file << std::setw(5) << left << mm_BCID[i];
    *output_file << std::setw(4) << left << mm_MMFE8[i];
    *output_file << std::setw(4) << left << mm_FIFOcount[i] << endl;
  }
}

void Close_MM(){
  if(g_mdat){
    if(g_MMstream)
      g_MMstream->close();
    g_MMstream = nullptr;
  }
  if(g_mroot){
    if(g_MMbase)
      delete g_MMbase;
    g_MMbase = nullptr;
    g_N_MMbase = 0;
    g_i_MMbase = -1;
  }
}

void Close_SCINT(){
  if(g_sdat){
    if(g_SCINTstream)
      g_SCINTstream->close();
    g_SCINTstream = nullptr;
  }
  if(g_sroot){
    if(g_SCINTbase)
      delete g_SCINTbase;
    g_MMbase = nullptr;
    g_N_MMbase = 0;
    g_i_MMbase = -1;
  }
}

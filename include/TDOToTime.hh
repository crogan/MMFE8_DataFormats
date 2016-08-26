#ifndef TDOToTime_HH
#define TDOToTime_HH

#include "include/TDOcalibBase.hh"

using namespace std;

///////////////////////////////////////////////
// TDOToTime class
//
// Class takes TDOcalibBase input
// and provide methods for calibrated
// TDO to time conversion
///////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <iomanip>

class TDOToTime {

public:
  TDOToTime(const string& TDOcalib_filename){

    TChain tree("TDO_calib");
    tree.AddFile(TDOcalib_filename.c_str());
    TDOcalibBase base(&tree);
    
    int Nentry = base.fChain->GetEntries();

    for(int i = 0; i < Nentry; i++){
      base.GetEntry(i);

      pair<int,int> key(base.MMFE8, base.VMM);

      if(m_MMFE8VMM_to_index.count(key) == 0){
	m_MMFE8VMM_to_index[key] = m_CH_to_index.size();
	m_CH_to_index.push_back(map<int,int>());
      }

      int index = m_MMFE8VMM_to_index[key];

      if(m_CH_to_index[index].count(base.CH) == 0){
	m_CH_to_index[index][base.CH] = m_prob.size();
	m_C.push_back(base.C);
	m_S.push_back(base.S);
	m_chi2.push_back(base.chi2);
	m_prob.push_back(base.prob);
      } else {
	int c = m_CH_to_index[index][base.CH];
	m_C[c] = base.C;
	m_S[c] = base.S;
	m_chi2[c] = base.chi2;
	m_prob[c] = base.prob;
      }
    }
  }

  ~TDOToTime(){}

  void PrintToFile(const std::string& outfile){
    std::ofstream fout;
    fout.open(outfile.c_str());
    
    fout << std::setw(6) << left << "MMFE8";
    fout << std::setw(4) << left << "VMM";
    fout << std::setw(4) << left << "CH";
    fout << std::setw(10) << left << "Gain";
    fout << std::setw(10) << left << "Pedestal";
    fout << "(TDO = time*G + P)" << endl;
    
    typedef std::map<std::pair<int,int>, int>::iterator it0_type;
    typedef std::map<int,int>::iterator it1_type;
  
    for(it0_type it0 = m_MMFE8VMM_to_index.begin(); 
	it0 != m_MMFE8VMM_to_index.end(); it0++){
      int index0 = it0->second;
      for(it1_type it1 = m_CH_to_index[index0].begin(); 
	it1 != m_CH_to_index[index0].end(); it1++){
	int index = it1->second;

	fout << std::setw(6) << left << it0->first.first;
	fout << std::setw(4) << left << it0->first.second;
	fout << std::setw(4) << left << it1->first;
	fout << std::setw(10) << left << scientific << setprecision(2) << m_S[index];
	fout << std::setw(10) << left << scientific << setprecision(2) << m_C[index];
	fout << endl;
      }
    }
    fout.close();
  }

  // returns charge in fC
  double GetTime(double TDO, int MMFE8, int VMM, int CH) const {
    pair<int,int> key(MMFE8,VMM);
    if(m_MMFE8VMM_to_index.count(key) == 0){
      PrintError(MMFE8,VMM,CH);
      return 0.;
    }
    int i = m_MMFE8VMM_to_index[key];

    if(m_CH_to_index[i].count(CH) == 0){
      PrintError(MMFE8,VMM,CH);
      return 0.;
    }
    int c = m_CH_to_index[i][CH];

    return (TDO-m_C[c])/m_S[c];
  }

  // returns chi2 from PDO v charge fit
  double GetFitChi2(int MMFE8, int VMM, int CH) const {
    pair<int,int> key(MMFE8,VMM);
    if(m_MMFE8VMM_to_index.count(key) == 0){
      PrintError(MMFE8,VMM,CH);
      return 0.;
    }
    int i = m_MMFE8VMM_to_index[key];

    if(m_CH_to_index[i].count(CH) == 0){
      PrintError(MMFE8,VMM,CH);
      return 0.;
    }
    int c = m_CH_to_index[i][CH];
    return m_chi2[c];
  }

  // returns probability from PDO v charge fit
  double GetFitProb(int MMFE8, int VMM, int CH) const {
    pair<int,int> key(MMFE8,VMM);
    if(m_MMFE8VMM_to_index.count(key) == 0){
      PrintError(MMFE8,VMM,CH);
      return 0.;
    }
    int i = m_MMFE8VMM_to_index[key];

    if(m_CH_to_index[i].count(CH) == 0){
      PrintError(MMFE8,VMM,CH);
      return 0.;
    }
    int c = m_CH_to_index[i][CH];
    return m_prob[c];
  }

private:
  mutable map<pair<int,int>, int> m_MMFE8VMM_to_index;
  mutable vector<map<int,int> > m_CH_to_index;
  
  vector<double> m_C;
  vector<double> m_S;
  vector<double> m_chi2;
  vector<double> m_prob;

  void PrintError(int MMFE8, int VMM, int CH) const {
    cout << "TDOToTime ERROR: ";
    cout << "No parameters for requested MMFE8 = " << MMFE8;
    cout << " VMM = " << VMM << endl;
    cout << " CH = " << CH << endl;
  }

};


#endif

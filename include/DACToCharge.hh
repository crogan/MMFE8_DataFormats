#ifndef DacToCharge_HH
#define DacToCharge_HH

#include <iostream>
#include <fstream>
#include <iomanip>

#include "include/fit_functions.hh"
#include "include/xADCcalibBase.hh"

using namespace std;

///////////////////////////////////////////////
// DACToCharge class
//
// Class takes xADCcalibBase input
// and provide methods for converting DAC
// values to corresponding induced charge
///////////////////////////////////////////////

class DACToCharge {

public:
  DACToCharge(const string& xADCcalib_filename){

    TChain tree("xADC_calib");
    tree.AddFile(xADCcalib_filename.c_str());
    xADCcalibBase base(&tree);
    
    int Nentry = base.fChain->GetEntries();

    for(int i = 0; i < Nentry; i++){
      base.GetEntry(i);

      pair<int,int> key(base.MMFE8, base.VMM);
    
      if(m_MMFE8VMM_to_index.count(key) == 0){
	m_MMFE8VMM_to_index[key] = m_sigma.size();
	m_sigma.push_back(base.sigma);
	m_c0.push_back(base.c0);
	m_A2.push_back(base.A2);
	m_t02.push_back(base.t02);
	m_d21.push_back(base.d21);
	m_chi2.push_back(base.chi2);
	m_prob.push_back(base.prob);
      } else {
	int index = m_MMFE8VMM_to_index[key];
	m_sigma[index] = base.sigma;
	m_c0[index] = base.c0;
	m_A2[index] = base.A2;
	m_t02[index] = base.t02;
	m_d21[index] = base.d21;
	m_chi2[index] = base.chi2;
	m_prob[index] = base.prob;
      }
    }
  }

  ~DACToCharge(){}

  void PrintToFile(const std::string& outfile){
    std::ofstream fout;
    fout.open(outfile.c_str());

    fout << std::setw(6) << left << "MMFE8" << std::setw(4) << left << "VMM";
    fout << std::setw(10) << left << "c0" << std::setw(10) << left << "A2";
    fout << std::setw(10) << left << "t02" << std::setw(10) << left << "d21" << endl;

    typedef std::map<std::pair<int,int>, int>::iterator it_type;
    for(it_type it = m_MMFE8VMM_to_index.begin(); 
	it != m_MMFE8VMM_to_index.end(); it++){
      int index = it->second;
      fout << std::setw(6) << left << it->first.first << std::setw(4) << left << it->first.second;
      fout << std::setw(10) << left << scientific << setprecision(2) << m_c0[index];
      fout << std::setw(10) << left << scientific << setprecision(2) << m_A2[index];
      fout << std::setw(10) << left << scientific << setprecision(2) << m_t02[index];
      fout << std::setw(10) << left << scientific << setprecision(2) << m_d21[index] << endl;
    }
    fout.close();
  }

  // returns charge in fC
  double GetCharge(double DAC, int MMFE8, int VMM) const {
    pair<int,int> key(MMFE8,VMM);
    if(m_MMFE8VMM_to_index.count(key) == 0){
      PrintError(MMFE8,VMM);
      return 0.;
    }
    int i = m_MMFE8VMM_to_index[key];
    double par[4];
    par[0] = m_c0[i];
    par[1] = m_A2[i];
    par[2] = m_t02[i];
    par[3] = m_d21[i];
    return P0_P2_P1(&DAC, par);
  }

  // returns charge error in fC
  double GetChargeError(double DAC, int MMFE8, int VMM) const {
    pair<int,int> key(MMFE8,VMM);
    if(m_MMFE8VMM_to_index.count(key) == 0){
      PrintError(MMFE8,VMM);
      return 0.;
    }
    int index = m_MMFE8VMM_to_index[key];
    return m_sigma[index];
  }

  // returns chi2 from charge v DAC fit
  double GetFitChi2(int MMFE8, int VMM) const {
    pair<int,int> key(MMFE8,VMM);
    if(m_MMFE8VMM_to_index.count(key) == 0){
      PrintError(MMFE8,VMM);
      return 0.;
    }
    int index = m_MMFE8VMM_to_index[key];
    return m_chi2[index];
  }

  // returns probability from charge v DAC fit
  double GetFitProb(int MMFE8, int VMM) const {
    pair<int,int> key(MMFE8,VMM);
    if(m_MMFE8VMM_to_index.count(key) == 0){
      PrintError(MMFE8,VMM);
      return 0.;
    }
    int index = m_MMFE8VMM_to_index[key];
    return m_prob[index];
  }

private:
  mutable map<pair<int,int>, int> m_MMFE8VMM_to_index;
  
  vector<double> m_sigma;
  vector<double> m_c0;
  vector<double> m_A2;
  vector<double> m_t02;
  vector<double> m_d21;
  vector<double> m_chi2;
  vector<double> m_prob;

  void PrintError(int MMFE8, int VMM) const {
    cout << "DACToCharge ERROR: ";
    cout << "No parameters for requested MMFE8 = " << MMFE8;
    cout << " VMM = " << VMM << endl;
  }

};


#endif

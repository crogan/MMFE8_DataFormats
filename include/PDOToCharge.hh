#ifndef PDOToCharge_HH
#define PDOToCharge_HH

#include "include/PDOcalibBase.hh"

using namespace std;

///////////////////////////////////////////////
// PDOToCharge class
//
// Class takes PDOcalibBase input
// and provide methods for calibrated
// PDO to charge conversion
///////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <iomanip>

class PDOToCharge {

public:
  PDOToCharge(const string& PDOcalib_filename){

    TChain tree("PDO_calib");
    tree.AddFile(PDOcalib_filename.c_str());
    PDOcalibBase base(&tree);
    
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
	m_c0.push_back(base.c0);
	m_A2.push_back(base.A2);
	m_t02.push_back(base.t02);
	m_d21.push_back(base.d21);
	m_chi2.push_back(base.chi2);
	m_prob.push_back(base.prob);
      } else {
	int c = m_CH_to_index[index][base.CH];
	m_c0[c] = base.c0;
	m_A2[c] = base.A2;
	m_t02[c] = base.t02;
	m_d21[c] = base.d21;
	m_chi2[c] = base.chi2;
	m_prob[c] = base.prob;
      }
    }
  }

  ~PDOToCharge(){}

  void PrintToFile(const std::string& outfile){
    std::ofstream fout;
    fout.open(outfile.c_str());
    
    fout << std::setw(6) << left << "MMFE8";
    fout << std::setw(4) << left << "VMM";
    fout << std::setw(4) << left << "CH";
    fout << std::setw(10) << left << "Gain";
    fout << std::setw(10) << left << "Pedestal";
    fout << "(PDO = Q*G + P)" << endl;
    
    typedef std::map<std::pair<int,int>, int>::iterator it0_type;
    typedef std::map<int,int>::iterator it1_type;
  
    for(it0_type it0 = m_MMFE8VMM_to_index.begin(); 
	it0 != m_MMFE8VMM_to_index.end(); it0++){
      int index0 = it0->second;
      for(it1_type it1 = m_CH_to_index[index0].begin(); 
	it1 != m_CH_to_index[index0].end(); it1++){
	int index = it1->second;
	double m = m_A2[index]*m_d21[index];
	double b = m_c0[index] - m*(m_d21[index] + 2.*m_t02[index]);

	fout << std::setw(6) << left << it0->first.first;
	fout << std::setw(4) << left << it0->first.second;
	fout << std::setw(4) << left << it1->first;
	fout << std::setw(10) << left << scientific << setprecision(2) << m;
	fout << std::setw(10) << left << scientific << setprecision(2) << b;
	fout << endl;
      }
    }
    fout.close();
  }

  // returns charge in fC
  double GetCharge(double PDO, int MMFE8, int VMM, int CH) const {
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

    // PDO above fit saturation
    if(PDO >= m_c0[c])
      return m_t02[c];

    // PDO in quadratic part
    if(PDO > m_c0[c] + m_A2[c]*m_d21[c]*m_d21[c])
      return -1.*sqrt(max(0., (PDO-m_c0[c])/m_A2[c])) + m_t02[c];

    // linear part
    return 0.5*( (PDO-m_c0[c])/m_A2[c]/m_d21[c] + m_d21[c] + 2.*m_t02[c] );
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
  
  vector<double> m_c0;
  vector<double> m_A2;
  vector<double> m_t02;
  vector<double> m_d21;
  vector<double> m_chi2;
  vector<double> m_prob;

  void PrintError(int MMFE8, int VMM, int CH) const {
    cout << "PDOToCharge ERROR: ";
    cout << "No parameters for requested MMFE8 = " << MMFE8;
    cout << " VMM = " << VMM << endl;
    cout << " CH = " << CH << endl;
  }

};


#endif

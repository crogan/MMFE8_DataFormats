//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Sep 20 11:14:50 2016 by ROOT version 5.34/25
// from TTree MM_data/MM_data
// found on file: test.root
//////////////////////////////////////////////////////////

#ifndef MMdataBase_h
#define MMdataBase_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include <vector>

using namespace std;

class MMdataBase {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   Int_t           EventNum;
   Int_t           Time_sec;
   Int_t           Time_nsec;
   Int_t           trig_BCID;
   Int_t           N_mm;
   vector<int>     *mm_VMM;
   vector<int>     *mm_CH;
   vector<int>     *mm_PDO;
   vector<int>     *mm_TDO;
   vector<int>     *mm_BCID;
   vector<int>     *mm_MMFE8;
   vector<int>     *mm_FIFOcount;

   // List of branches
   TBranch        *b_EventNum;   //!
   TBranch        *b_Time_sec;   //!
   TBranch        *b_Time_nsec;   //!
   TBranch        *b_trig_BCID;   //!
   TBranch        *b_N_mm;   //!
   TBranch        *b_mm_VMM;   //!
   TBranch        *b_mm_CH;   //!
   TBranch        *b_mm_PDO;   //!
   TBranch        *b_mm_TDO;   //!
   TBranch        *b_mm_BCID;   //!
   TBranch        *b_mm_MMFE8;   //!
   TBranch        *b_mm_FIFOcount;   //!

   MMdataBase(TTree *tree=0);
   virtual ~MMdataBase();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

inline MMdataBase::MMdataBase(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("test.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("test.root");
      }
      f->GetObject("MM_data",tree);

   }
   Init(tree);
}

inline MMdataBase::~MMdataBase()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

inline Int_t MMdataBase::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
inline Long64_t MMdataBase::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

inline void MMdataBase::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   mm_VMM = 0;
   mm_CH = 0;
   mm_PDO = 0;
   mm_TDO = 0;
   mm_BCID = 0;
   mm_MMFE8 = 0;
   mm_FIFOcount = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("EventNum", &EventNum, &b_EventNum);
   fChain->SetBranchAddress("Time_sec", &Time_sec, &b_Time_sec);
   fChain->SetBranchAddress("Time_nsec", &Time_nsec, &b_Time_nsec);
   fChain->SetBranchAddress("trig_BCID", &trig_BCID, &b_trig_BCID);
   fChain->SetBranchAddress("N_mm", &N_mm, &b_N_mm);
   fChain->SetBranchAddress("mm_VMM", &mm_VMM, &b_mm_VMM);
   fChain->SetBranchAddress("mm_CH", &mm_CH, &b_mm_CH);
   fChain->SetBranchAddress("mm_PDO", &mm_PDO, &b_mm_PDO);
   fChain->SetBranchAddress("mm_TDO", &mm_TDO, &b_mm_TDO);
   fChain->SetBranchAddress("mm_BCID", &mm_BCID, &b_mm_BCID);
   fChain->SetBranchAddress("mm_MMFE8", &mm_MMFE8, &b_mm_MMFE8);
   fChain->SetBranchAddress("mm_FIFOcount", &mm_FIFOcount, &b_mm_FIFOcount);
   Notify();
}

inline Bool_t MMdataBase::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

inline void MMdataBase::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
inline Int_t MMdataBase::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}

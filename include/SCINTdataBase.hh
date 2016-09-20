//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Sep 20 13:35:13 2016 by ROOT version 5.34/25
// from TTree SCINT_data/SCINT_data
// found on file: test.root
//////////////////////////////////////////////////////////

#ifndef SCINTdataBase_h
#define SCINTdataBase_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include <vector>

// Fixed size dimensions of array or collections stored in the TTree if any.
using namespace std;

class SCINTdataBase {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   Int_t           EventNum;
   Int_t           RunNum;
   Int_t           Time_sec;
   Int_t           Time_nsec;
   Int_t           N_sci;
   vector<int>     *sci_CH;
   vector<int>     *sci_count;

   // List of branches
   TBranch        *b_EventNum;   //!
   TBranch        *b_RunNum;   //!
   TBranch        *b_Time_sec;   //!
   TBranch        *b_Time_nsec;   //!
   TBranch        *b_N_sci;   //!
   TBranch        *b_sci_CH;   //!
   TBranch        *b_sci_count;   //!

   SCINTdataBase(TTree *tree=0);
   virtual ~SCINTdataBase();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

inline SCINTdataBase::SCINTdataBase(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("test.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("test.root");
      }
      f->GetObject("SCINT_data",tree);

   }
   Init(tree);
}

inline SCINTdataBase::~SCINTdataBase()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

inline Int_t SCINTdataBase::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
inline Long64_t SCINTdataBase::LoadTree(Long64_t entry)
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

inline void SCINTdataBase::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   sci_CH = 0;
   sci_count = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("EventNum", &EventNum, &b_EventNum);
   fChain->SetBranchAddress("RunNum", &RunNum, &b_RunNum);
   fChain->SetBranchAddress("Time_sec", &Time_sec, &b_Time_sec);
   fChain->SetBranchAddress("Time_nsec", &Time_nsec, &b_Time_nsec);
   fChain->SetBranchAddress("N_sci", &N_sci, &b_N_sci);
   fChain->SetBranchAddress("sci_CH", &sci_CH, &b_sci_CH);
   fChain->SetBranchAddress("sci_count", &sci_count, &b_sci_count);
   Notify();
}

inline Bool_t SCINTdataBase::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

inline void SCINTdataBase::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
inline Int_t SCINTdataBase::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}

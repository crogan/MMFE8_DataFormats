//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Feb 28 17:02:15 2017 by ROOT version 6.06/00
// from TTree TP_data/TP_data
// found on file: TP_R3516.root
//////////////////////////////////////////////////////////

#ifndef TPdataBase_h
#define TPdataBase_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"

class TPdataBase {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Int_t           EventNum;
   Int_t           Time_sec;
   Int_t           Time_nsec;
   Int_t           BCID;
   vector<int>     *tp_VMM;
   vector<int>     *tp_CH;
   vector<int>     *tp_MMFE8;

   // List of branches
   TBranch        *b_EventNum;   //!
   TBranch        *b_Time_sec;   //!
   TBranch        *b_Time_nsec;   //!
   TBranch        *b_BCID;   //!
   TBranch        *b_tp_VMM;   //!
   TBranch        *b_tp_CH;   //!
   TBranch        *b_tp_MMFE8;   //!

   TPdataBase(TTree *tree=0);
   virtual ~TPdataBase();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

inline TPdataBase::TPdataBase(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("TP_R3516.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("TP_R3516.root");
      }
      f->GetObject("TP_data",tree);

   }
   Init(tree);
}

inline TPdataBase::~TPdataBase()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

inline Int_t TPdataBase::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
inline Long64_t TPdataBase::LoadTree(Long64_t entry)
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

inline void TPdataBase::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   tp_VMM = 0;
   tp_CH = 0;
   tp_MMFE8 = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("EventNum", &EventNum, &b_EventNum);
   fChain->SetBranchAddress("Time_sec", &Time_sec, &b_Time_sec);
   fChain->SetBranchAddress("Time_nsec", &Time_nsec, &b_Time_nsec);
   fChain->SetBranchAddress("BCID", &BCID, &b_BCID);
   fChain->SetBranchAddress("tp_VMM", &tp_VMM, &b_tp_VMM);
   fChain->SetBranchAddress("tp_CH", &tp_CH, &b_tp_CH);
   fChain->SetBranchAddress("tp_MMFE8", &tp_MMFE8, &b_tp_MMFE8);
   Notify();
}

inline Bool_t TPdataBase::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

inline void TPdataBase::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
inline Int_t TPdataBase::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}

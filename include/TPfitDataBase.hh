//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu May 11 18:32:56 2017 by ROOT version 6.06/00
// from TTree TPfit_data/TPfit_data
// found on file: test.root
//////////////////////////////////////////////////////////

#ifndef TPfitDataBase_h
#define TPfitDataBase_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"

class TPfitDataBase {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Int_t           EventNum;
   Int_t           cntr;
   Int_t           Time_sec;
   Int_t           Time_nsec;
   Int_t           BCID;
   Float_t         mxlocal;
   vector<int>     *tpfit_VMM;
   vector<int>     *tpfit_CH;
   vector<int>     *tpfit_MMFE8;
   Int_t           tpfit_n;

   // List of branches
   TBranch        *b_EventNum;   //!
   TBranch        *b_cntr;   //!
   TBranch        *b_Time_sec;   //!
   TBranch        *b_Time_nsec;   //!
   TBranch        *b_BCID;   //!
   TBranch        *b_mxlocal;   //!
   TBranch        *b_tpfit_VMM;   //!
   TBranch        *b_tpfit_CH;   //!
   TBranch        *b_tpfit_MMFE8;   //!
   TBranch        *b_tpfit_n;   //!

   TPfitDataBase(TTree *tree=0);
   virtual ~TPfitDataBase();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

inline TPfitDataBase::TPfitDataBase(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("test.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("test.root");
      }
      f->GetObject("TPfit_data",tree);

   }
   Init(tree);
}

inline TPfitDataBase::~TPfitDataBase()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

inline Int_t TPfitDataBase::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
inline Long64_t TPfitDataBase::LoadTree(Long64_t entry)
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

inline void TPfitDataBase::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   tpfit_VMM = 0;
   tpfit_CH = 0;
   tpfit_MMFE8 = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("EventNum", &EventNum, &b_EventNum);
   fChain->SetBranchAddress("cntr", &cntr, &b_cntr);
   fChain->SetBranchAddress("Time_sec", &Time_sec, &b_Time_sec);
   fChain->SetBranchAddress("Time_nsec", &Time_nsec, &b_Time_nsec);
   fChain->SetBranchAddress("BCID", &BCID, &b_BCID);
   fChain->SetBranchAddress("mxlocal", &mxlocal, &b_mxlocal);
   fChain->SetBranchAddress("tpfit_VMM", &tpfit_VMM, &b_tpfit_VMM);
   fChain->SetBranchAddress("tpfit_CH", &tpfit_CH, &b_tpfit_CH);
   fChain->SetBranchAddress("tpfit_MMFE8", &tpfit_MMFE8, &b_tpfit_MMFE8);
   fChain->SetBranchAddress("tpfit_n", &tpfit_n, &b_tpfit_n);
   Notify();
}

inline Bool_t TPfitDataBase::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

inline void TPfitDataBase::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
inline Int_t TPfitDataBase::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}

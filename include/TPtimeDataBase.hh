//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri Jun  9 18:04:56 2017 by ROOT version 6.08/06
// from TTree TPtime_data/TPtime_data
// found on file: /Users/annwang/atlas/octuplet_analysis/data/Run3522/intermediate/Run3522_TIME_decoded.root
//////////////////////////////////////////////////////////

#ifndef TPtimeDataBase_h
#define TPtimeDataBase_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.

class TPtimeDataBase {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Int_t           EventNum;
   Int_t           Time_sec;
   Int_t           Time_nsec;
   Int_t           BCID;
   Int_t           ph;
   Int_t           N_trig;
   Int_t           overflow;

   // List of branches
   TBranch        *b_EventNum;   //!
   TBranch        *b_Time_sec;   //!
   TBranch        *b_Time_nsec;   //!
   TBranch        *b_BCID;   //!
   TBranch        *b_ph;   //!
   TBranch        *b_N_trig;   //!
   TBranch        *b_overflow;   //!

   TPtimeDataBase(TTree *tree=0);
   virtual ~TPtimeDataBase();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

inline TPtimeDataBase::TPtimeDataBase(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("/Users/annwang/atlas/octuplet_analysis/data/Run3522/intermediate/Run3522_TIME_decoded.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("/Users/annwang/atlas/octuplet_analysis/data/Run3522/intermediate/Run3522_TIME_decoded.root");
      }
      f->GetObject("TPtime_data",tree);

   }
   Init(tree);
}

inline TPtimeDataBase::~TPtimeDataBase()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

inline Int_t TPtimeDataBase::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
inline Long64_t TPtimeDataBase::LoadTree(Long64_t entry)
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

inline void TPtimeDataBase::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("EventNum", &EventNum, &b_EventNum);
   fChain->SetBranchAddress("Time_sec", &Time_sec, &b_Time_sec);
   fChain->SetBranchAddress("Time_nsec", &Time_nsec, &b_Time_nsec);
   fChain->SetBranchAddress("BCID", &BCID, &b_BCID);
   fChain->SetBranchAddress("ph", &ph, &b_ph);
   fChain->SetBranchAddress("N_trig", &N_trig, &b_N_trig);
   fChain->SetBranchAddress("overflow", &overflow, &b_overflow);
   Notify();
}

inline Bool_t TPtimeDataBase::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

inline void TPtimeDataBase::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
inline Int_t TPtimeDataBase::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}

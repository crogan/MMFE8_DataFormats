//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Wed Jun 29 14:04:01 2016 by ROOT version 5.34/25
// from TTree PDO_calib/PDO_calib
// found on file: PDO_test_PDOfit_PDOcalib.root
//////////////////////////////////////////////////////////

#ifndef PDOcalibBase_h
#define PDOcalibBase_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.

class PDOcalibBase {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   Double_t        MMFE8;
   Double_t        VMM;
   Double_t        CH;
   Double_t        c0;
   Double_t        A2;
   Double_t        t02;
   Double_t        d21;
   Double_t        chi2;
   Double_t        prob;

   // List of branches
   TBranch        *b_MMFE8;   //!
   TBranch        *b_VMM;   //!
   TBranch        *b_CH;   //!
   TBranch        *b_c0;   //!
   TBranch        *b_A2;   //!
   TBranch        *b_t02;   //!
   TBranch        *b_d21;   //!
   TBranch        *b_chi2;   //!
   TBranch        *b_prob;   //!

   PDOcalibBase(TTree *tree=0);
   virtual ~PDOcalibBase();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

inline PDOcalibBase::PDOcalibBase(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("PDO_test_PDOfit_PDOcalib.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("PDO_test_PDOfit_PDOcalib.root");
      }
      f->GetObject("PDO_calib",tree);

   }
   Init(tree);
}

inline PDOcalibBase::~PDOcalibBase()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

inline Int_t PDOcalibBase::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
inline Long64_t PDOcalibBase::LoadTree(Long64_t entry)
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

inline void PDOcalibBase::Init(TTree *tree)
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

   fChain->SetBranchAddress("MMFE8", &MMFE8, &b_MMFE8);
   fChain->SetBranchAddress("VMM", &VMM, &b_VMM);
   fChain->SetBranchAddress("CH", &CH, &b_CH);
   fChain->SetBranchAddress("c0", &c0, &b_c0);
   fChain->SetBranchAddress("A2", &A2, &b_A2);
   fChain->SetBranchAddress("t02", &t02, &b_t02);
   fChain->SetBranchAddress("d21", &d21, &b_d21);
   fChain->SetBranchAddress("chi2", &chi2, &b_chi2);
   fChain->SetBranchAddress("prob", &prob, &b_prob);
   Notify();
}

inline Bool_t PDOcalibBase::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

inline void PDOcalibBase::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
inline Int_t PDOcalibBase::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}

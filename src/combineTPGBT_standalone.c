/* 
Program in C to combine TPFit data and GBT data into one file based on the BCIDS
Written by Joseph Farah on June 19, 2017
Last updated by [Joseph Farah] on: [June 23, 2017]

Notes
- for some reason this code causes the root intepreter to segfault when .q is run
- if you do it (type .q) twice the problem just goes away
- how to print anythin that has type vector<int>: use * upon declaration and get
	value with gbtMMFE8->at(0)
- the above notes should be outdated as this is now a standalone script and not a root macro
- just fucking die --KH
*/

// header imports
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <string>
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include "TSystem.h"

#define TRUE 1
#define FALSE 0


void progress(double time_diff, int nprocessed, int ntotal){
  double rate = (double)(nprocessed+1)/time_diff;
  std::cout.precision(1);
  std::cout << "\r > " << nprocessed << " / " << ntotal 
  << " | "   << std::fixed << 100*(double)(nprocessed)/(double)(ntotal) << "%"
  << " | "   << std::fixed << rate << "Hz"
  << " | "   << std::fixed << time_diff/60 << "m elapsed"
  << " | "   << std::fixed << (double)(ntotal-nprocessed)/(rate*60) << "m remaining"
  << std::flush;
  std::cout.precision(6);
}

void get_time()
{
    time_t now = time(0);
    char* dt = ctime(&now);
    std::cout << dt << std::endl;
}



// begin main function 
int main( int argc, char *argv[] )
{
    std::cout << "TIME BEGIN" << std::endl;
    get_time();
	// check the number of args, it should be 3 for proper execution
	// there will always be 4 args because the file name is the first arg
    if( argc != 4 ) { std::cout << argv[0] << " needs three args" << std::endl; return 0; }

	// make all the filename strings constants to use in the TFile constructor
    const char* gfile = argv[1];
    const char* tfile = argv[2];
    const char* ofile = argv[3];

    std::cout << "File arguments provided: ";
    std::cout << gfile << " " << tfile <<  " " << ofile << std::endl;

	// initialize the GBT file using TFile and using the GBT file as the argument
	// create new fileobject for GBT
    TFile *gbt_file_object = new TFile(gfile);
	// pull the tree from the file
    TTree *gbt_tree = (TTree*)gbt_file_object->Get("GBT_data");
    
	// do the same thing now with the TPfit file
	// create new file object for TPfit
    TFile *tpfit_file_object = new TFile(tfile);
	// pull the tpfit tree fromt he file
    TTree *tpfit_tree = (TTree*)tpfit_file_object->Get("TPfit_data");

	// get the number of all the entries in both of the trees
    long ngbt = gbt_tree->GetEntries();
    long nfit = tpfit_tree->GetEntries();

	// create the output file, overwriting the old one if necessary
    TFile *output_file = new TFile(ofile, "RECREATE");

	// this will create the combined branches data tree
	// argument setup: Branch(branchname address, leaflist)
	// for vectors, use the address of the vector instead of the name
    TTree *combdata = new TTree("TPcomb_data", "TPcomb_data");

    // here i'm going to start creating the branches for the new file
    // this is going to require me initiliazing various variables to use
    // as addresses, a TTree, and the branches themsleves
    int                 t_eventnum;
    int                 t_cntr;
    int                 t_timesec;
    int                 t_timensec;
    int                 t_bcid;
    int                 t_mxlocal;
    int                 t_tpfit_n;
    std::vector<int>    *t_tpfit_VMM = 0;
    std::vector<int>    *t_tpfit_CH = 0;
    std::vector<int>    *t_tpfit_MMFE8 = 0;
    std::vector<int>    *t_tpfit_BCID = 0;

    combdata->Branch("EventNum", &t_eventnum);
    combdata->Branch("cntr",  &t_cntr);
    combdata->Branch("Time_sec",  &t_timesec);
    combdata->Branch("Time_nsec",  &t_timensec);    
    combdata->Branch("BCID",  &t_bcid);
    combdata->Branch("mxlocal", &t_mxlocal);
    combdata->Branch("tpfit_VMM", &t_tpfit_VMM);
    combdata->Branch("tpfit_CH", &t_tpfit_CH);
    combdata->Branch("tpfit_MMFE8", &t_tpfit_MMFE8);
    combdata->Branch("tpfit_BCID", &t_tpfit_BCID);
    combdata->Branch("tpfit_n",&t_tpfit_n);

    // get the addresses of all the branches in the GBT and TPFit trees
    // gbt tree address declaration
    int                 gbtTime_sec;
    int                 gbtTime_nsec;
    int                 gEventNum;
    std::vector<int> 	*gbtMMFE8 = 0;
    std::vector<int> 	*gbt_VMM = 0;
    std::vector<int> 	*gbt_CH = 0;
    std::vector<int>	*gbt_BCID = 0;

    // List of branches which may be unnecessary but whatever
    TBranch        *b_EventNum = 0;   
    TBranch        *b_Time_sec = 0; 
    TBranch        *b_Time_nsec = 0;   
    TBranch        *b_gbt_VMM = 0;   
    TBranch        *b_gbt_CH = 0;   
    TBranch        *b_gbt_MMFE8 = 0;   
    TBranch        *b_gbt_BCID = 0;   

   // initialize all the branches using the pointers just created
    gbt_tree->SetBranchAddress("EventNum", &gEventNum, &b_EventNum);
    gbt_tree->SetBranchAddress("Time_sec", &gbtTime_sec, &b_Time_sec);
    gbt_tree->SetBranchAddress("Time_nsec", &gbtTime_nsec, &b_Time_nsec);
    gbt_tree->SetBranchAddress("gbt_VMM", &gbt_VMM, &b_gbt_VMM);
    gbt_tree->SetBranchAddress("gbt_CH", &gbt_CH, &b_gbt_CH);
    gbt_tree->SetBranchAddress("gbt_MMFE8", &gbtMMFE8, &b_gbt_MMFE8);
    gbt_tree->SetBranchAddress("gbt_BCID", &gbt_BCID, &b_gbt_BCID);


    // tpfit tree address declaration, FFS initialize your pointers
    int EventNum, nevent;
    int tpTime_sec;
    int tpTime_nsec;
    int BCID, bcid;
    float mxlocal, mxloc;
    int tpfit_n, nhit;
    int cntr, spec_cntr;
    std::vector<int>    *tpfit_MMFE8 = 0;
    std::vector<int>    *tpfit_VMM = 0;
    std::vector<int>    *tpfit_CH = 0;

    // initialize all the tpfit branches using the pointers just creates
    tpfit_tree->SetBranchAddress("EventNum", &EventNum);
    tpfit_tree->SetBranchAddress("Time_sec", &tpTime_sec);
    tpfit_tree->SetBranchAddress("Time_nsec", &tpTime_nsec);
    tpfit_tree->SetBranchAddress("BCID", &BCID);
    tpfit_tree->SetBranchAddress("mxlocal", &mxlocal);
    tpfit_tree->SetBranchAddress("tpfit_n", &tpfit_n);
    tpfit_tree->SetBranchAddress("cntr", &cntr);
    tpfit_tree->SetBranchAddress("tpfit_MMFE8", &tpfit_MMFE8);
    tpfit_tree->SetBranchAddress("tpfit_VMM", &tpfit_VMM);
    tpfit_tree->SetBranchAddress("tpfit_CH", &tpfit_CH);

    // initialize any variables that will be used within the loop
    double gbttime;
    double tpfittime;
    double time_difference;
    int i = 0;
    int num_changes_per_loop = 1;
    int counter = 0;
    int j;
    int currifitpk = 0;
    int tmpcount = 0;
    int tsize, gsize;
    int gtmp;
    // vectors that need to be created that will hold everything
    const int GBT_COORDINATE_LENGTH = 9;
    std::vector<int> gbtmmfes = {};
    std::vector<int> gbtvmms = {};
    std::vector<int> gbtchs = {};
    std::vector<int> gbtbcids = {};

    std::vector<int> tpmmfes ={};
    std::vector<int> tpvmms = {};
    std::vector<int> tpchs = {};
    std::vector<int> tpbcids = {};

    // this vector will store the hit information for each track
    // use i to compare to the gbt data
    std::vector<int> tp_tmp_hit_storage = {};
    std::vector<int> gbt_tmp_hit_storage = {};


    // preallocating all the vector slots on the heap--slower compile times
    // but significantly faster run times
    gbtmmfes.reserve(GBT_COORDINATE_LENGTH);
    gbtvmms.reserve(GBT_COORDINATE_LENGTH);
    gbtchs.reserve(GBT_COORDINATE_LENGTH);
    gbtbcids.reserve(GBT_COORDINATE_LENGTH);
    tpmmfes.reserve(GBT_COORDINATE_LENGTH);
    tpvmms.reserve(GBT_COORDINATE_LENGTH);
    tpchs.reserve(GBT_COORDINATE_LENGTH);
    tpbcids.reserve(GBT_COORDINATE_LENGTH);
    tp_tmp_hit_storage.reserve(3);
    tp_tmp_hit_storage.reserve(3);



    // board IPs go here, note its an array not a vector
    int boards[] = {118,116,102,119,106,107,117,105};
    
    //loop through the GBT set
    tpfit_tree->Print();


    //progress bar stuff
    std::chrono::time_point<std::chrono::system_clock> time_start;
    std::chrono::duration<double> elapsed_seconds;
    time_start = std::chrono::system_clock::now();

    i = 0;
    while(gbt_tree->GetEntry(i))
    {
        gbtmmfes.clear();
        gbtvmms.clear();
        gbtchs.clear();
        gbtbcids.clear();
        gtmp = gEventNum;

        // get the exact tiem of the event in nanoseconds by adding the time
        // in seconds to the time in nanoseconds
        gbttime = gbtTime_sec + gbtTime_nsec/pow(10.,9);

        // if the time exceeds a certain limit, break
        // eventually when we're out of the testing phase we'll remove the i limitation
        if(gbttime > 1495040000) { std::cout << "done" << std::endl;break; }

        /* creating a list containing the coordinates for each part of the event*/
        for(counter = 0; counter < gbtMMFE8->size(); counter++)
        {
            gbtmmfes.push_back(gbtMMFE8->at(counter));
            gbtvmms.push_back(gbt_VMM->at(counter));
            gbtchs.push_back(gbt_CH->at(counter));
            gbtbcids.push_back(gbt_BCID->at(counter));
        }
        counter = 0;

        // check to see if there are any boards that did not fire, if thats the case
        // replace their values with zeros by adding a new "event" to the array
        for(counter=0; counter<8; counter++)
            {;
                if( std::find(gbtMMFE8->begin(), gbtMMFE8->end(), boards[counter])==gbtMMFE8->end() ) 
                {
                    gbtmmfes.push_back(boards[counter]);
                    gbtvmms.push_back(0);
                    gbtchs.push_back(0);
                    gbtbcids.push_back(0);
                }
            }

        // begin trying to align the TPfit packet with the GBT equivalent
            j = currifitpk;
            while(TRUE) {

            //get trigger process across #j, whatever j happens to be
                int nmatch = 0;
                tpfit_tree->GetEntry(j);
                tpmmfes.clear();
                tpvmms.clear();
                tpchs.clear();
                tpbcids.clear();

            // if j is greater than the number of things in the fit tp file, break the loop
                if( j == nfit ) { break; }

            // start declaring all the track information from tpfittree
                nevent = EventNum;

            //  std::cout << "Eventnum " << nevent <<  std::endl; 
                tpfittime = tpTime_sec + tpTime_nsec/pow(10.,9);
                bcid = BCID;
                mxloc = mxlocal;
                nhit = tpfit_n;
                spec_cntr = cntr;

            // create the vectors that will hold all the TPfit event information
                for(counter = 0; counter < tpfit_MMFE8->size(); counter++)
                {
                    tpmmfes.push_back(tpfit_MMFE8->at(counter));
                    tpvmms.push_back(tpfit_VMM->at(counter));
                    tpchs.push_back(tpfit_CH->at(counter));

                }
                counter = 0;

            // check the time difference betwen the tp and gbt packet
            // they should ideally be very close together, some difference is acceptable
            // just fucking die --KH but if the difference is greater than 0.2 we are misaligning the data
                time_difference = tpfittime - gbttime;

            // break if we've gone to far
                if( time_difference>0.2 ) { break; }

            // if the time difference between the tp packet and the gbt packet
            // is within reasonable error, do the following
                time_difference = fabs(time_difference);

            // if the time difference is within reasonable error
                if( time_difference<0.2 )
                {
                // is the time of the tpfit packet in the list of the gbt packet BCIDS
                // we created earlier? if so, continue
                    if( std::find(gbtbcids.begin(), gbtbcids.end(), bcid) != gbtbcids.end() )
                    {
                    // fill a vector with the single hit information to compare to the gbt stuff
                        for(int counter1=0; counter1<gbtmmfes.size(); counter1++)
                        {
                            gbt_tmp_hit_storage.clear();
                            gbt_tmp_hit_storage.push_back(gbtmmfes[counter1]);
                            gbt_tmp_hit_storage.push_back(gbtvmms[counter1]);
                            gbt_tmp_hit_storage.push_back(gbtchs[counter1]);
                            gsize = gbt_tmp_hit_storage.size();

                            for(int counter2=0; counter2<tpmmfes.size(); counter2++)
                            {
                                tp_tmp_hit_storage.clear();
                                tp_tmp_hit_storage.push_back(tpmmfes[counter2]);
                                tp_tmp_hit_storage.push_back(tpvmms[counter2]);
                                tp_tmp_hit_storage.push_back(tpchs[counter2]);
                                gsize = gbt_tmp_hit_storage.size();
                                tsize = tp_tmp_hit_storage.size();

                            // are the coordinates int eh gbt set the same as the 
                            // coordinates recorded by the trigger process? if yes
                            // continue, if no, don't do anything
                                if( gbt_tmp_hit_storage == tp_tmp_hit_storage )
                                {
                                // increment nmatch too add to the lsit of entries
                                // that have corresponding trigger events
                                    nmatch += 1;   
                                    t_tpfit_MMFE8->push_back(tpmmfes[counter2]);
                                    t_tpfit_VMM->push_back(tpvmms[counter2]);
                                    t_tpfit_CH->push_back(tpchs[counter2]);
                                    t_tpfit_BCID->push_back(gbtbcids[counter1]);
                                    tmpcount = counter1;;  
                                }

                            }

                        }

                    // update the combined branches with all the new data
                        t_mxlocal = mxloc;
                        t_bcid = gbtbcids[tmpcount];
                        t_tpfit_n = nhit;
                        t_cntr = spec_cntr;
                        t_eventnum = nevent;
                        t_timesec = tpTime_sec;
                        t_timensec = tpTime_nsec;

                        if( nmatch == nhit )
                        {
                            currifitpk = j + 1;
                            combdata->Fill();

                        }
                    // clear the current branch entries to make room for the next ones
                        t_tpfit_MMFE8->clear();
                        t_tpfit_VMM->clear();
                        t_tpfit_CH->clear();
                        t_tpfit_BCID->clear();
                    }
                }

                j++;

            // UNCOMMENT THIS IF YOU WANT TO ESTIMATE HOW LONG THE PROGRAM WILL TAKE TO FINISH  
            // elapsed_seconds = (std::chrono::system_clock::now() - time_start);
            // progress(elapsed_seconds.count(), i, gbt_tree->GetEntries());
            } 

            i++;
        }


        output_file->Write();
        output_file->Close();
        std::cout <<  "TIME END" << std::endl;
        get_time();
        return 0;
    }
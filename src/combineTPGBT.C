/*/////////////////////////////////////////////////////////////////////////////////////////
Program in C to combine TPFit data and GBT data into one file based on the BCIDS
attempting to steer by TPFit
Written by Joseph Farah on June 19, 2017
Last updated by [Joseph Farah] on: [July 10, 2017]

Notes
- for some reason this code causes the root intepreter to segfault when .q is run
- if you do it (type .q) twice the problem just goes away
- how to print anythin that has type vector<int>: use * upon declaration and get
    value with gbtMMFE8->at(0)
- the above notes should be outdated as this is now a standalone script and not a root macro
- just f@$%ing die --KH
///////////////////////////////////////////////////////////////////////////////////////////*/

// header imports
#include <iostream>
#include <vector>
#include <algorithm>
#include <typeinfo>
#include <chrono>
#include <string>
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include "TSystem.h"

#define TRUE  1
#define FALSE 0

#define pass ;

void progress(double time_diff, int nprocessed, int ntotal)
{
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
    time_t now  = time(0);
    char* dt    = ctime(&now);
    std::cout << dt << std::endl;
}

void print_vector(std::vector<double> vec) // only pass vectors by value, not reference
{
    for(int s = 0; s < vec.size(); s++)
    {
        std::cout << vec[s] << std::endl;
    }
}



/* begin main function */
int main( int argc, char *argv[] )
{
    std::cout << "TIME BEGIN (VM TEST, TPFIT STEER)" << std::endl;
    get_time();

    /*define them beforehand--C++ doesn't like it when you construct things in if statements*/
    char* tmp1 = 0;
    char* tmp2 = 0;
    char* tmp3 = 0;
    char* tmp4 = 0;
    char* tmp5 = 0;

    int has_limit       = 0;
    int limit           = 0;
    int check_for_limit = 0;
    
    /* argument stuff */
    if(argc != 11 && argc != 9){
        std::cout << "!Not enough or invalid arguments, please try again.\n";
        std::cout << "Example syntax: ./combineTPGBT -g Run3522_GBT_decoded.root -t Run3522_FIT_decoded.root -o combined.root -r 3522" << std::endl;
        return 0;
    }
    /*go through all arguments and examine next relevant arg, adjust settings accordingly */
    if(argc == 11) { check_for_limit = 1; }                                        
    for(int i = 1; i < argc; i++) {                                                 
        if (i + 1 != argc)  {
            if (strcmp(argv[i], "-g") == 0)      {  tmp1 = argv[i + 1]; i++;   } 
            else if (strcmp(argv[i], "-t") == 0) {  tmp2 = argv[i + 1]; i++;   } 
            else if (strcmp(argv[i], "-o") == 0) {  tmp3 = argv[i + 1]; i++;   }
            else if (strcmp(argv[i], "-r") == 0) {  tmp4 = argv[i + 1]; i++;   }
            else {
                if(check_for_limit == 1) {
                    if (strcmp(argv[i], "-l") == 0) {
                        if(strcmp(argv[i+1], "0")==0) {
                            continue;
                        }
                        else {
                            /* let the program know it has to deal with a limit later on */
                            has_limit = 1;
                            tmp5 = argv[i+1];
                        }
                    }
                }
                else {
                    std::cout << "This argument caused an error: " << argv[i] << std::endl;
                    std::cout << "Not enough or invalid arguments, please try again.\n";
                    std::cout << "Example syntax: ./combineTPGBT -g Run3522_GBT_decoded.root -t Run3522_FIT_decoded.root -o combined.root -r 3522 -l 0" << std::endl;
                    return 0;
                }
            }
            
        }
    }

    /* ROOT needs const chars as file names */
    const char* gfile   = tmp1;
    const char* tfile   = tmp2;
    const char* ofile   = tmp3;
    const char* run_num = tmp4;

    /* tmp5 will only have a value if limit was defined as existing */
    if(has_limit != 0)
    {
        limit = std::stoi(tmp5);
        std::cout << "limit" << limit << std::endl;
    }

    /* print out provided file arguments */
    std::cout << "File arguments provided: ";
    std::cout << gfile << std::endl << tfile << std::endl << ofile << std::endl;

    /* initialize the GBT file using TFile and using the GBT file as the argument */
    /* create new fileobject for GBT */
    TFile *gbt_file_object = new TFile(gfile);
    /* pull the tree from the file */
    TTree *gbt_tree = (TTree*)gbt_file_object->Get("GBT_data");    
    
    /* do the same thing now with the TPfit file */
    /* create new file object for TPfit */
    TFile *tpfit_file_object = new TFile(tfile);
    
    /* pull the tpfit tree fromt the file */
    TTree *tpfit_tree = (TTree*)tpfit_file_object->Get("TPfit_data");
    
    /* check to make sure the TTrees exist */
    if (!gbt_tree){
        std::cout << "Error: GBT tree is a null pointer!" << std::endl;
        return 0;
    }
    if (!tpfit_tree){
        std::cout << "Error: TPFIT tree is a null pointer!" << std::endl;
        return 0;
    }

    /* get the number of all the entries in both of the trees */
    long nfit = tpfit_tree->GetEntries();
    long ngbt = gbt_tree->GetEntries();

    /* create the output file, overwriting the old one if necessary */
    TFile *output_file = new TFile(ofile, "RECREATE");

    /* 
    this will create the combined branches data tree 
    argument setup: Branch(branchname address, leaflist) 
    for vectors, use the address of the vector instead of the name 
    */
    TTree *combdata = new TTree("TPcomb_data", "TPcomb_data");

    /*
    here i'm going to start creating the branches for the new file
    this is going to require me initiliazing various variables to use
    as addresses, a TTree, and the branches themsleves
    */
    int                 t_eventnum;
    int                 t_gbteventnum;
    int                 t_gts = 0;
    int                 t_gtns = 0;
    int                 t_cntr;
    int                 t_timesec;
    int                 t_timensec;
    int                 t_bcid;
    float               t_mxlocal;
    int                 t_tpfit_n;
    std::vector<int>    *t_tpfit_VMM = 0;
    std::vector<int>    *t_tpfit_CH = 0;
    std::vector<int>    *t_tpfit_MMFE8 = 0;
    std::vector<int>    *t_tpfit_BCID = 0;

    combdata->Branch("EventNum", &t_eventnum);
    combdata->Branch("EventNumGBT", &t_gbteventnum);
    combdata->Branch("gbtTime_sec", &t_gts);
    combdata->Branch("gbtTime_nsec", &t_gtns);
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

    /*get the addresses of all the branches in the GBT and TPFit trees */

    /* gbt tree address declaration */
    int                 gbtTime_sec;
    int                 gbtTime_nsec;
    int                 gEventNum;
    std::vector<int>    *gbtMMFE8 = 0;
    std::vector<int>    *gbt_VMM = 0;
    std::vector<int>    *gbt_CH = 0;
    std::vector<int>    *gbt_BCID = 0;

    /* List of branches which may be unnecessary but whatever */
    TBranch        *b_EventNum = 0;   
    TBranch        *b_Time_sec = 0; 
    TBranch        *b_Time_nsec = 0;   
    TBranch        *b_gbt_VMM = 0;   
    TBranch        *b_gbt_CH = 0;   
    TBranch        *b_gbt_MMFE8 = 0;   
    TBranch        *b_gbt_BCID = 0;   

    /* initialize all the branches using the pointers just created */
    gbt_tree->SetBranchAddress("EventNum", &gEventNum, &b_EventNum);
    gbt_tree->SetBranchAddress("Time_sec", &gbtTime_sec, &b_Time_sec);
    gbt_tree->SetBranchAddress("Time_nsec", &gbtTime_nsec, &b_Time_nsec);
    gbt_tree->SetBranchAddress("gbt_VMM", &gbt_VMM, &b_gbt_VMM);
    gbt_tree->SetBranchAddress("gbt_CH", &gbt_CH, &b_gbt_CH);
    gbt_tree->SetBranchAddress("gbt_MMFE8", &gbtMMFE8, &b_gbt_MMFE8);
    gbt_tree->SetBranchAddress("gbt_BCID", &gbt_BCID, &b_gbt_BCID);


    /* tpfit tree address declaration, FFS initialize your pointers */
    int                 EventNum;
    int                 nevent;
    int                 gbt_event_num;
    int                 tpTime_sec;
    int                 tpTime_nsec;
    int                 BCID;
    int                 bcid;
    float               mxlocal;
    float               mxloc;
    int                 tpfit_n;
    int                 nhit;
    int                 cntr;
    int                 spec_cntr;
    std::vector<int>    *tpfit_MMFE8 = 0;
    std::vector<int>    *tpfit_VMM = 0;
    std::vector<int>    *tpfit_CH = 0;

    /* initialize all the tpfit branches using the pointers just creates */
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

    /* initialize any variables that will be used within the loop */
    double  gbttime;
    double  tpfittime;
    double  time_difference;
    int     j;
    int     gtmp;
    int i           = 0;
    int counter     = 0;
    int currifitpk  = 0;
    int fill        = 1;
    int num_missing = 0;

    /* vectors that need to be created that will hold everything */
    const int           GBT_COORDINATE_LENGTH = 9;
    std::vector<int>    gbtmmfes = {};
    std::vector<int>    gbtvmms = {};
    std::vector<int>    gbtchs = {};
    std::vector<int>    gbtbcids = {};

    std::vector<int>    tpmmfes ={};
    std::vector<int>    tpvmms = {};
    std::vector<int>    tpchs = {};
    std::vector<int>    tpbcids = {};

    /* this vector will store the hit information for each track */
    std::vector<double> tp_tmp_hit_storage  = {};
    std::vector<double> gbt_tmp_hit_storage = {};


    /* preallocating all the vector slots on the heap--slower compile times
    but significantly faster run times */
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


    /* board IPs go here, its a vector, HAVE I MADE SENPAI PROUD */
    std::vector<int> boards;

    /* selecting the board configuration based on the arguments provided */
    int run_num_int = std::stoi(run_num);
    std::cout << "run num: " << run_num_int << std::endl;
    if     (run_num_int >= 3515 && run_num_int <=3517) { boards = {105, 118, 107, 106, 119, 117, 116, 111}; }
    else if(run_num_int >= 3518 && run_num_int <=3523) { boards = {105, 117, 107, 106, 119, 102, 116, 118}; }
    else if(run_num_int == 3524)                       { boards = {105, 101, 107, 106, 119, 102, 116, 118}; }
    else if(run_num_int == 3525)                       { boards = {105, 101, 107, 106, 119, 120, 116, 118}; }
    else { std::cout << "ERROR: Board configuration unknown for run number " << run_num_int << std::endl; return 0;}


    /* progress bar stuff */
    std::chrono::time_point<std::chrono::system_clock> time_start;
    std::chrono::duration<double> elapsed_seconds;
    time_start = std::chrono::system_clock::now();

    i = 0;
    while(tpfit_tree->GetEntry(i)) /*iterate through tree while an entry in tpfit_tree exists */
    {
        /* if limit exists, break after that limit (see program arguments) */
        if(has_limit != 0 && i >= limit) {  break;  }

        /* clear all necessary vectors */
        tpmmfes.clear();
        tpvmms.clear();
        tpchs.clear(); 
        tpbcids.clear();

        /* grab all necessary data from the tpfit_tree */
        nevent      = EventNum; 
        tpfittime   = tpTime_sec + tpTime_nsec/pow(10.,9);
        bcid        = BCID;
        mxloc       = mxlocal;
        nhit        = tpfit_n;
        spec_cntr   = cntr;

        /* remove time break if iterating over entire dataset */
        if(tpfittime > 1495040000) { break; }

        /* fill vectors with packet information, to be compared to GBT */
        for(counter = 0; counter < tpfit_MMFE8->size(); counter++)
        {
            tpmmfes.push_back(tpfit_MMFE8->at(counter));
            tpvmms.push_back(tpfit_VMM->at(counter));
            tpchs.push_back(tpfit_CH->at(counter));
        }
        counter = 0;

        /* bootstrapping part 1 */
        j = currifitpk;
        fill = 1;

        /* begin looking in GBT dataset for matching packets */
        while(TRUE)
        {
            /* if j == ngbt, we've reache the end of the dataset; stop looking, and don't bootstrap */
            if(j == ngbt) { break; }

            /* snag current entry from GBT tree */
            gbt_tree->GetEntry(j);

            /* clear all necessary GBT branches in preparation of the addition of more dataa */
            gbtmmfes.clear();
            gbtvmms.clear();
            gbtchs.clear();
            gbtbcids.clear();
            t_tpfit_BCID->clear();

            /* get gbt time (in sec and nsec) */
            gbttime = gbtTime_sec + gbtTime_nsec/pow(10.,9);
            gbt_event_num = gEventNum;

            /* fill vectors with GBT data, to be compared to TPfit data */
            for(counter = 0; counter < gbtMMFE8->size(); counter++)
            {
                gbtmmfes.push_back(gbtMMFE8->at(counter));
                gbtvmms.push_back(gbt_VMM->at(counter));
                gbtchs.push_back(gbt_CH->at(counter));
                gbtbcids.push_back(gbt_BCID->at(counter));
            }
            counter = 0;

            /* if any boards did not fire during the entry, fill the sublevel coordinates with zeroes) */
            for(counter=0; counter<8; counter++)
            {
                if( std::find(gbtMMFE8->begin(), gbtMMFE8->end(), boards[counter])==gbtMMFE8->end() ) 
                {
                    gbtmmfes.push_back(boards[counter]);
                    gbtvmms.push_back(0);
                    gbtchs.push_back(0);
                    gbtbcids.push_back(0);
                }

                /* if a bad board IP is found, kill everything. PURGE */
                if(std::find(boards.begin(), boards.end(), gbtmmfes[counter]) == boards.end())
                {
                    std::cout << "WARNING! BOARD IP IN DATA NOT FOUND IN IP LIST" << std::endl;
                    return 0;
                }
            }

            /* no matter what happens, this data is going in the end file
            why not add them to the entry now? */
            t_mxlocal       = mxloc;
            t_bcid          = BCID;
            t_gts           = -1;
            t_gtns          = -1;
            t_gbteventnum   = -1;
            t_tpfit_n       = nhit;
            t_cntr          = spec_cntr;
            t_eventnum      = nevent;
            t_timesec       = tpTime_sec;
            t_timensec      = tpTime_nsec;

            /* see how far away the packets are */
            time_difference = gbttime - tpfittime;
            
            /* if they are too far away, not point in getting farther away */
            if(time_difference > 0.2) { break; }

            time_difference = fabs(time_difference);

            /* reset the match counter */
            int nmatch = 0;
            t_tpfit_MMFE8->clear();
            t_tpfit_VMM->clear();
            t_tpfit_CH->clear();

            /* if the time difference is acceptable, begin packet comparison */
            if(time_difference < 0.2)
            {
                /* check if the trigger processor output is in the GBT packets */
                if(std::find(gbtbcids.begin(), gbtbcids.end(), bcid) != gbtbcids.end())
                {
                    for(int counter2 = 0; counter2 < tpmmfes.size(); counter2++)
                    {
                        tp_tmp_hit_storage.clear();
                        tp_tmp_hit_storage.push_back(tpmmfes[counter2]);
                        tp_tmp_hit_storage.push_back(tpvmms[counter2]);
                        tp_tmp_hit_storage.push_back(tpchs[counter2]); 

                        for(int counter1 = 0; counter1 < gbtmmfes.size(); counter1++)
                        {
                            gbt_tmp_hit_storage.clear();
                            gbt_tmp_hit_storage.push_back(gbtmmfes[counter1]);
                            gbt_tmp_hit_storage.push_back(gbtvmms[counter1]);
                            gbt_tmp_hit_storage.push_back(gbtchs[counter1]);

                            /* if the two packets match each other, add them to the entry */
                            if(gbt_tmp_hit_storage == tp_tmp_hit_storage)
                            {
                                nmatch += 1;
                                t_tpfit_MMFE8->push_back(tpmmfes[counter2]);
                                t_tpfit_VMM->push_back(tpvmms[counter2]);
                                t_tpfit_CH->push_back(tpchs[counter2]);
                                t_tpfit_BCID->push_back(gbtbcids[counter1]); 
                            }
                        }
                    }

                    /* check if enough matches have been made */
                    if(nmatch == nhit)
                    {
                        /* bootstrapping part two (do NOT do j + 1, it breaks everything) */
                        currifitpk      = j;

                        /* replace the -1s with good stuff (data) */
                        t_gbteventnum   = gbt_event_num;
                        t_gts           = gbtTime_sec;
                        t_gtns          = gbtTime_nsec;

                        /* fill the combined output tree! and let the program know it 
                        doesn't need to fill it twice */
                        combdata->Fill();
                        fill = 0; 
                    }
                }
            }
            /* increment j--move to the next gbt entry */
            j++;
        }
        /* check if the tree was filled; if not fill it with -1 for GBT data */
        if(fill == 1) { 
            /* add the missings to the milk carton */
            num_missing += 1;
            t_tpfit_BCID->clear();
            t_tpfit_MMFE8->clear();
            t_tpfit_VMM->clear();
            t_tpfit_CH->clear();

            /* fill vectors with necessary TPfit informationt to be added to the output file */
            for(int counter2 = 0; counter2 < tpmmfes.size(); counter2++)
            {
                t_tpfit_MMFE8->push_back(tpmmfes[counter2]);
                t_tpfit_VMM->push_back(tpvmms[counter2]);
                t_tpfit_CH->push_back(tpchs[counter2]);
                t_tpfit_BCID->push_back(-1); 
            }
            /* fill the tree with the current entry */
            combdata->Fill();

            /* you can never be too safe */
            t_tpfit_BCID->clear();
        }

        /* increment i--move to the next tpfit entry */
        i++;

        /* progress bar stuff */
        if(i%500==0) 
        {    
            elapsed_seconds = (std::chrono::system_clock::now() - time_start);
            progress(elapsed_seconds.count(), i, nfit);
        } 
    }

    /* write to and close combined output file */
    output_file->Write();
    output_file->Close();
    std::cout << "Num missing: " << num_missing << std::endl;
    std::cout  << std::endl <<"TIME END" << std::endl;
    get_time();
    return 0;
}

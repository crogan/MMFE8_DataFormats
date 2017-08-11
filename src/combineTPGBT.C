/*/////////////////////////////////////////////////////////////////////////////////////////
Program in C to combine TPFit data and GBT data into one file based on the BCIDS
attempting to steer by TPFit
Written by Joseph Farah on August 9th, 2017
Last updated by [Joseph Farah] on: [August 9, 2017]

Notes
- Using TTree clone instead of steering by either file
- just f@$%ing die --KH
///////////////////////////////////////////////////////////////////////////////////////////*/

/* header imports */
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

#define TRUE 1
#define FALSE 0

#define pass ;

typedef std::vector<int> int_v;

/* declare boards as a global variable so it can be edited globally */
int_v boards;

void progress(double time_diff, int nprocessed, int ntotal) {
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

void get_time() {
    time_t now  = time(0);
    char* dt    = ctime(&now);
    std::cout << dt << std::endl;
}

/* only pass vectors by value, not reference */
void print_vector_int(int_v vec) {
    for(unsigned int s = 0; s < vec.size(); s++) {
        std::cout << vec[s] << std::endl;
    }
}

void print_vector_double(std::vector<double> vec) {
    for(unsigned int s = 0; s < vec.size(); s++) {
        std::cout << std::fixed << vec[s] << std::endl;
    }
}

int get_board_configuration(const char* rnum) {
    /* selecting the board configuration based on the arguments provided */
    int success = 1;
    int run_num_int = std::stoi(rnum);
    std::cout << "run num: " << run_num_int << std::endl;
    if     (run_num_int >= 3515 && run_num_int <=3517) { boards = {105, 118, 107, 106, 119, 117, 116, 111}; }
    else if(run_num_int >= 3518 && run_num_int <=3523) { boards = {105, 117, 107, 106, 119, 102, 116, 118}; }
    else if(run_num_int == 3524)                       { boards = {105, 101, 107, 106, 119, 102, 116, 118}; }
    else if(run_num_int == 3525)                       { boards = {105, 101, 107, 106, 119, 120, 116, 118}; }
    else { std::cout << "ERROR: Board configuration unknown for run number " << run_num_int << std::endl; success = 0;}
    return success;
}

/* begin main function */
int main( int argc, char *argv[] )  {
    std::cout << "TIME BEGIN (TPTREE CLONE)" << std::endl;
    get_time();

    /* define argument holders before hand--C++ doesn't like it when you construct things in if statements */
    char *tmp1 = 0;
    char *tmp2 = 0;
    char *tmp3 = 0;
    char *tmp4 = 0;
    char *tmp5 = 0;

    int has_limit       = 0;
    int limit           = 0;
    int check_for_limit = 0;

    /* argument stuff */
    if(argc != 11 && argc != 9) {
        std::cout << "ERROR: not enough or invalid arguments, please try again." << std::endl;
        std::cout << "Example syntax: ./combineTPGBT -g Run3522_GBT_decoded.root -t Run3522_FIT_decoded.root -o combined.root -r 3522" << std::endl;
        return 0;
    }

    /* go through all arguments and examine relevant args, adjust settings accordingly */
    if(argc==11) { check_for_limit = 1; }
    for(int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-g") == 0) { tmp1 = argv[i+1]; i++; }
        else if (strcmp(argv[i], "-t") == 0) { tmp2 = argv[i+1]; i++; }
        else if (strcmp(argv[i], "-o") == 0) { tmp3 = argv[i+1]; i++; }
        else if (strcmp(argv[i], "-r") == 0) { tmp4 = argv[i+1]; i++; }
        else {
            if(check_for_limit == 1) {
                if(strcmp(argv[i], "-l") == 0) {
                    has_limit = 1;
                    tmp5 = argv[i+1];
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

    /* ROOT needs const chars as file names */
    const char* gfile = tmp1;
    const char* tfile = tmp2;
    const char* ofile = tmp3;
    const char* run_num = tmp4;

    /* tmp5 will only have a value if limit was defined as existing */
    if (has_limit != 0) {
        limit = std::stoi(tmp5);
        std::cout << "The program will cease after this TPFIT entry: " << limit << std::endl;
    }

    /* print out provided file arguments */
    std::cout << "File arguments provided: " << std::endl;
    std::cout << gfile << std::endl 
              << tfile << std::endl 
              << ofile << std::endl;

    /* initiliaze the GBT file using TFile and using the GBT file as the argument */
    /* crete new file object for GBT */
    TFile *gbt_file_object = new TFile(gfile);

    /* pull the tree from the file */
    TTree *gbt_tree = (TTree*)gbt_file_object->Get("GBT_data");

    /* initialize the TPfit file */
    /* create new file object for TPfit */
    TFile *tpfit_file_object = new TFile(tfile);

    /* pull the tpfit tree from the file */
    TTree *tpfit_tree_old = (TTree*)tpfit_file_object->Get("TPfit_data");

    /* create the output file first to avoid any fill errors */
    TFile *output_file = new TFile(ofile, "RECREATE");

    /* clone the TPfit tree */
    TTree *combdata = tpfit_tree_old->CloneTree(0);


    /* check to make sure all appropriate TTrees exist */
    if (!gbt_tree) {
        std::cout << "Error: GBT tree is a null pointer!" << std::endl;
        return 0;
    }
    if (!tpfit_tree_old) {
        std::cout << "Error TPfit tree (raw) is a null pointer!" << std::endl;
        if(!combdata) {
            std::cout << "Error TPfit tree (clone) is a null pointer!" << std::endl;
            return 0;
        }
    }
    if (!combdata) {
        std::cout << "Error: TPfit tree (clone) is a null pointer!" << std::endl;
        return 0;
    }

    /* get the number of entries from both trees */
    long nfit = tpfit_tree_old->GetEntries();
    long ngbt = gbt_tree->GetEntries();

    /*get the addresses of all the branches in the GBT and TPFit trees */

    /* gbt tree address declaration */
    int     gbtTime_sec  = 0;
    int     gbtTime_nsec = 0;
    int     gEventNum    = 0;
    int_v   *gbtMMFE8    = 0;
    int_v   *gbt_VMM     = 0;
    int_v   *gbt_CH      = 0;
    int_v   *gbt_BCID    = 0;

    /* List of branches which may be unnecessary but whatever */
    TBranch *b_EventNum  = 0;   
    TBranch *b_Time_sec  = 0; 
    TBranch *b_Time_nsec = 0;   
    TBranch *b_gbt_VMM   = 0;   
    TBranch *b_gbt_CH    = 0;   
    TBranch *b_gbt_MMFE8 = 0;   
    TBranch *b_gbt_BCID  = 0;   

    /* initialize all the branches using the pointers just created */
    gbt_tree->SetBranchAddress("EventNum", &gEventNum, &b_EventNum);
    gbt_tree->SetBranchAddress("Time_sec", &gbtTime_sec, &b_Time_sec);
    gbt_tree->SetBranchAddress("Time_nsec", &gbtTime_nsec, &b_Time_nsec);
    gbt_tree->SetBranchAddress("gbt_VMM", &gbt_VMM, &b_gbt_VMM);
    gbt_tree->SetBranchAddress("gbt_CH", &gbt_CH, &b_gbt_CH);
    gbt_tree->SetBranchAddress("gbt_MMFE8", &gbtMMFE8, &b_gbt_MMFE8);
    gbt_tree->SetBranchAddress("gbt_BCID", &gbt_BCID, &b_gbt_BCID);


    /* tpfit tree address declaration, FFS initialize your pointers */
    int    EventNum        = 0;
    int    gbt_event_num   = 0;
    int    tpTime_sec      = 0;
    int    tpTime_nsec     = 0;
    int    BCID            = 0;
    float  mxlocal         = 0;
    int    tpfit_n         = 0;
    int    cntr            = 0;
    int_v  *tpfit_MMFE8    = 0;
    int_v  *tpfit_VMM      = 0;
    int_v  *tpfit_CH       = 0;

    /* initialize all the tpfit branches using the pointers just creates */
    tpfit_tree_old->SetBranchAddress("EventNum", &EventNum);
    tpfit_tree_old->SetBranchAddress("Time_sec", &tpTime_sec);
    tpfit_tree_old->SetBranchAddress("Time_nsec", &tpTime_nsec);
    tpfit_tree_old->SetBranchAddress("BCID", &BCID);
    tpfit_tree_old->SetBranchAddress("mxlocal", &mxlocal);
    tpfit_tree_old->SetBranchAddress("tpfit_n", &tpfit_n);
    tpfit_tree_old->SetBranchAddress("cntr", &cntr);
    tpfit_tree_old->SetBranchAddress("tpfit_MMFE8", &tpfit_MMFE8);
    tpfit_tree_old->SetBranchAddress("tpfit_VMM", &tpfit_VMM);
    tpfit_tree_old->SetBranchAddress("tpfit_CH", &tpfit_CH);

    /* combdata branch additions */

    int   t_gbteventnum = 0;
    int   t_gts = 0;
    int   t_gtns = 0;
    int_v *t_tpfit_BCID = 0;

    combdata->Branch("EventNumGBT", &t_gbteventnum);
    combdata->Branch("gbtTime_sec", &t_gts);
    combdata->Branch("gbtTime_nsec", &t_gtns);
    combdata->Branch("tpfit_BCID", &t_tpfit_BCID);

    /* initialize any variables that will be used within the loop */
    double  gbttime;
    double  tpfittime;
    double  time_difference;

    int j           = 0;
    int i           = 0;
    int counter     = 0;
    int currifitpk  = 0;
    int fill        = 1;
    int num_missing = 0;

    /* vectors that need to be created that will hold everything */
    int_v    gbtmmfes = {};
    int_v    gbtvmms  = {};
    int_v    gbtchs   = {};
    int_v    gbtbcids = {};

    int_v    tpmmfes = {};
    int_v    tpvmms  = {};
    int_v    tpchs   = {};
    int_v    tpbcids = {};

    /* this vector will store the hit information for each track */
    std::vector<double> tp_tmp_hit_storage  = {};
    std::vector<double> gbt_tmp_hit_storage = {};

    /* select correct board configuration */
    int board_success = get_board_configuration(run_num);
    if(board_success == 0) {
        return 0;
    }


    /* progress bar stuff */
    std::chrono::time_point<std::chrono::system_clock> time_start;
    std::chrono::duration<double> elapsed_seconds;
    time_start = std::chrono::system_clock::now();

    i = 0;

    /* iterate through tree while an entry exists */
    while(tpfit_tree_old->GetEntry(i)) {

        /* change stuff here if you don't want the program to break */
        if(has_limit != 0 && i >= limit) { break; }

        /* grab tpfittime */
        tpfittime = tpTime_sec + tpTime_nsec/pow(10.,9);

        /* just in case a match isn't been found */
        t_gbteventnum   = -1;
        t_gts           = -1;
        t_gtns          = -1;
        t_tpfit_BCID->clear();

        /* remove time break if iterating over entire dataset */
        if(tpfittime > 1495040000) { break; }

         /* fill vectors with packet information, to be compared to GBT */
        for(unsigned int counter = 0; counter < tpfit_MMFE8->size(); counter++)  {
            tpmmfes.push_back(tpfit_MMFE8->at(counter));
            tpvmms.push_back(tpfit_VMM->at(counter));
            tpchs.push_back(tpfit_CH->at(counter));
        }

        /* bootstrapping, part 1 */
        j = currifitpk;
        fill = 1;

        /* begin looking in GBT dataset for matching packets */
        while(TRUE) {
            /* if j == ngbt, we've reached the end of the dataset */
            if(j == ngbt) { break; }

            /* grab current entry from the GBT tree */
            gbt_tree->GetEntry(j);

            /*clear all necessary GBT branches in preparation of the addition of more data */
            gbtmmfes.clear();
            gbtvmms.clear();
            gbtchs.clear();
            gbtbcids.clear();

            /* get gbt time (in sec and nsec) */
            gbttime = gbtTime_sec + gbtTime_nsec/pow(10.,9);
            gbt_event_num = gEventNum;

            /* fill vectors with GBT data, to be compared TPfit data */
            for(unsigned int counter = 0; counter < gbtMMFE8->size(); counter++) {
                gbtmmfes.push_back(gbtMMFE8->at(counter));
                gbtvmms.push_back(gbt_VMM->at(counter));
                gbtchs.push_back(gbt_CH->at(counter));
                gbtbcids.push_back(gbt_BCID->at(counter));
            }

            /* if any boards did not fire during the entry, fill the sublevel coordinates with zeroes) */
            for(counter=0; counter<8; counter++) {
                if( std::find(gbtMMFE8->begin(), gbtMMFE8->end(), boards[counter])==gbtMMFE8->end() ) {
                    gbtmmfes.push_back(boards[counter]);
                    gbtvmms.push_back(0);
                    gbtchs.push_back(0);
                    gbtbcids.push_back(0);
                }

                /* if a bad board IP is found, kill everything. PURGE */
                if(std::find(boards.begin(), boards.end(), gbtmmfes[counter]) == boards.end())  {
                    std::cout << "WARNING! BOARD IP IN DATA NOT FOUND IN IP LIST" << std::endl;
                    return 0;
                }
            }

            /* see how far away the packets are */
            time_difference = gbttime - tpfittime;

            /* if they are too far away, no point in getting farther away */
            if(time_difference > 0.2) { break; }

            time_difference = fabs(time_difference);

            /* reset the match counter, clear some vectors */
            int nmatch = 0;

            /* if the time difference is acceptable, begin packet comparison */
            if(time_difference < 0.2) {
                /* check if the trigger processor output is in the GBT packets */
                if(std::find(gbtbcids.begin(), gbtbcids.end(), BCID) != gbtbcids.end()) {
                    
                    for(unsigned int counter2 = 0; counter2 < tpfit_MMFE8->size(); counter2++) {
                        tp_tmp_hit_storage.clear();
                        tp_tmp_hit_storage.push_back(tpfit_MMFE8->at(counter2));
                        tp_tmp_hit_storage.push_back(tpfit_VMM->at(counter2));
                        tp_tmp_hit_storage.push_back(tpfit_CH->at(counter2)); 

                        for(unsigned int counter1 = 0; counter1 < gbtmmfes.size(); counter1++) {
                            gbt_tmp_hit_storage.clear();
                            gbt_tmp_hit_storage.push_back(gbtmmfes[counter1]);
                            gbt_tmp_hit_storage.push_back(gbtvmms[counter1]);
                            gbt_tmp_hit_storage.push_back(gbtchs[counter1]);

                            /* if the two packets match each other, add them to the entry */
                            if(gbt_tmp_hit_storage == tp_tmp_hit_storage) {
                                nmatch += 1;
                                t_tpfit_BCID->push_back(gbtbcids[counter1]); 
                            }
                        }
                    }


                    /* check if enough matches have been made */
                    if(nmatch == tpfit_n)
                    {
                        /* replace the -1s with good stuff (data) */
                        t_gbteventnum   = gbt_event_num;
                        t_gts           = gbtTime_sec;
                        t_gtns          = gbtTime_nsec;

                        /* fill the combined output tree! and let the program know it 
                        doesn't need to fill it twice */
                        fill = 0; 
                        
                        /* bootstrapping part two (do NOT do j + 1, it breaks everything) */
                        currifitpk = j;
                    }
                }
            }
            
            /* increment j--move to the next gbt entry */
            j++;
        }

        if(fill == 1) {
            num_missing += 1;
            t_tpfit_BCID->clear();
            for(unsigned int counter2 = 0; counter2 < tpfit_MMFE8->size(); counter2++) {
                t_tpfit_BCID->push_back(-1); 
            }
        }
        combdata->Fill();
        t_tpfit_BCID->clear();

        /* progress bar stuff */
        if(i%1000==0) {    
            elapsed_seconds = (std::chrono::system_clock::now() - time_start);
            progress(elapsed_seconds.count(), i, nfit);
        } 

        /* increment i--move to the next tpfit entry */
        i++;
    }


    output_file->Write();
    output_file->Close();
    std::cout << std::endl << "Num missing: " << num_missing << std::endl;
    std::cout  << std::endl <<"TIME END" << std::endl;
    get_time();
    return 0;
}
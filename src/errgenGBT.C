/* 
Program in C to analye and compare the gbterr and good gbt entries for a given run
Written by Joseph Farah on July 5, 2017
Last updated by [Joseph Farah] on: [July 10, 2017]

Notes
- 
*/

// header imports
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <typeinfo>
#include <algorithm>
#include <chrono>
#include <string>
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include "TSystem.h"
#include "TMath.h"
#include "TH1D.h"
#include "TRandom3.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TApplication.h"



std::vector<std::string> explode(const std::string& str, const char& ch) {

    /* This function explodes a string into a vector, like split() in python*/

    std::string next;
    std::vector<std::string> result;

    // For each character in the string
    for (std::string::const_iterator it = str.begin(); it != str.end(); it++) {
        // If we've hit the terminal character
        if (*it == ch) {
            // If we have some characters accumulated
            if (!next.empty()) {
                // Add them to the result vector
                result.push_back(next);
                next.clear();
            }
        } else {
            // Accumulate the next character into the sequence
            next += *it;
        }
    }
    if (!next.empty())
        result.push_back(next);
    return result;
}

void progress(double time_diff, int nprocessed, int ntotal){

    /*Alex's progress bar; C implentation*/

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




int main( int argc, char *argv[] )
{   

    if( argc != 3 ) { std::cout << argv[0] << " needs two args. First arg is always GBT file, and second is GBTerr file.\n\n" << std::endl 
        << "example usage: ./errgenGBT /path/to/gbtfile /path/to/gbterr/file" << std::endl; return 0; }
    
    std::chrono::time_point<std::chrono::system_clock> time_start;
    std::chrono::duration<double> elapsed_seconds;
    time_start = std::chrono::system_clock::now();

    const char* gbt_file_path = argv[1];
    std::string gbterr_file_path = argv[2];
    
    // Define a new canvas
    TApplication app("app", nullptr, nullptr);
    TCanvas *canvas = new TCanvas("c1", "testing");
    //change canvas height
    canvas->SetWindowSize(900, 600);
    canvas->Divide(1,1);

    /////////////////////////////////////////////////////////////////
    //COMMENT THIS IF YOU WANT THE GRAPH TO BE ON A LINEAR/LOG SCALE//
    /////////////////////////////////////////////////////////////////
    // canvas->SetLogy();

    
    const int   NUM_OF_CRAP = 1979885/1000;
    int         line_num = 0;

    TH1 *hist1 = new TH1D("hist1", "Error in GBT packets (red) and successful packets (white) vs time)", NUM_OF_CRAP, 0, 5);
    TH1 *hist2 = new TH1D("hist2", "Error in GBT packets (red) and successful packets (white) vs time)", NUM_OF_CRAP, 0, 5);

    std::cout << "1" << std::endl;
    std::vector<std::string>    exploded_line;
    std::string                 line;
    std::ifstream               gbterr_file;
    std::cout << "2" << std::endl;
    gbterr_file.open(gbterr_file_path);
    std::cout << 3 << std::endl;
    // creation of the first histogram
    if(gbterr_file.is_open())
    {
        while(getline(gbterr_file, line))
        {   
            line_num++;
            std::stringstream ss(line);
            line.erase(0,1);
            line.erase(line.size()-1);
            exploded_line = explode(line, ',');

            double rawtime = (stod(exploded_line[2])/(pow(10,9))-1498073258.177727)/86400.0;
            hist1->Fill(rawtime);
            if(line_num%10000==0) {    
                elapsed_seconds = (std::chrono::system_clock::now() - time_start);
                progress(elapsed_seconds.count(), line_num, 1979885);
                // std::cout << x_val << std::endl;
            }
        }
    }

    time_start = std::chrono::system_clock::now();

    // create new fileobject for GBT
    TFile *gbt_file_object = new TFile(gbt_file_path);
    // pull the tree from the file
    TTree *gbt_tree = (TTree*)gbt_file_object->Get("GBT_data");

    if (!gbt_tree){
        std::cout << "Error: GBT tree is a null pointer!" << std::endl;
        return 0;
    }

    // get the addresses of all the branches in the GBT and TPFit trees
    // gbt tree address declaration
    int                 gbtTime_sec;
    int                 gbtTime_nsec;
    int                 gEventNum;
    std::vector<int>    *gbtMMFE8 = 0;
    std::vector<int>    *gbt_VMM = 0;
    std::vector<int>    *gbt_CH = 0;
    std::vector<int>    *gbt_BCID = 0;

    // List of branches which may be unnecessary but whatever
    TBranch             *b_EventNum = 0;   
    TBranch             *b_Time_sec = 0; 
    TBranch             *b_Time_nsec = 0;   
    TBranch             *b_gbt_VMM = 0;   
    TBranch             *b_gbt_CH = 0;   
    TBranch             *b_gbt_MMFE8 = 0;   
    TBranch             *b_gbt_BCID = 0;   

    // initialize all the branches using the pointers just created
    gbt_tree->SetBranchAddress("EventNum", &gEventNum, &b_EventNum);
    gbt_tree->SetBranchAddress("Time_sec", &gbtTime_sec, &b_Time_sec);
    gbt_tree->SetBranchAddress("Time_nsec", &gbtTime_nsec, &b_Time_nsec);
    gbt_tree->SetBranchAddress("gbt_VMM", &gbt_VMM, &b_gbt_VMM);
    gbt_tree->SetBranchAddress("gbt_CH", &gbt_CH, &b_gbt_CH);
    gbt_tree->SetBranchAddress("gbt_MMFE8", &gbtMMFE8, &b_gbt_MMFE8);
    gbt_tree->SetBranchAddress("gbt_BCID", &gbt_BCID, &b_gbt_BCID);

    int     counter = 0;
    double    gbttime;

    while(gbt_tree->GetEntry(counter++))
    {
        gbttime = (double)(gbtTime_sec) + gbtTime_nsec/pow(10.,9)-1498073258.177727;
        gbttime = gbttime/86400.0;
        // std::cout << std::fixed << gbttime << std::endl;;

        hist2->Fill(gbttime);

        if(counter%10000==0) {    
            elapsed_seconds = (std::chrono::system_clock::now() - time_start);
            progress(elapsed_seconds.count(), counter, gbt_tree->GetEntries());
        }
    }



    hist2->Draw("hist");
    hist2->GetXaxis()->SetTitle("Days since beginning of experiment");
    hist2->GetYaxis()->SetTitle("N(GBT Buffer)");
    hist1->Draw("same hist");
    hist1->SetFillColor(kRed);
    canvas->Update();
    
    app.Run();
    return 0;
}

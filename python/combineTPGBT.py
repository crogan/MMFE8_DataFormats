#!/usr/bin/python

# combines GBT and tp root files to one
# cannabilized code from Alex

import ROOT, sys
import time as tm
from array import array
import argparse

def main(argv):
    inputfile = ''
    outputfile = ''

    parser = argparse.ArgumentParser()
    parser.add_argument("--gfile", default="", help="input GBT file")
    parser.add_argument("--tfile", default="", help="input TP file")
    parser.add_argument("-o", default="", help="output root file")
    args = parser.parse_args()

    gbtf = ROOT.TFile(args.gfile)
    gbt_tree = gbtf.Get("GBT_data")
    gbt_branches = gbt_tree.GetListOfBranches()

    tpf = ROOT.TFile(args.tfile)
    tpfit_tree = tpf.Get("TPfit_data")
    tpfit_branches = tpfit_tree.GetListOfBranches()

    i = 0

    gbtpks = []
    fitpks = []

    # get all gbt information
    tstart = tm.time()
    ngbt = gbt_tree.GetEntries()
    while gbt_tree.GetEntry(i):
        i += 1
        time = gbt_tree.Time_sec + gbt_tree.Time_nsec/pow(10.,9)
        #if (i % 100000 == 0):
        #    print "Unpacking GBT Event",i

        gbtpk = GBTPacket(time)

        mmfes = [im for im in gbt_tree.gbt_MMFE8]
        vmms = [iv for iv in gbt_tree.gbt_VMM]
        chs = [ich for ich in gbt_tree.gbt_CH]
        bcids = [ib for ib in gbt_tree.gbt_BCID]

        for j in range(len(mmfes)):
            gbtpk.add(mmfes[j],vmms[j],chs[j],bcids[j])

        gbtpk.pad()
        gbtpks.append(gbtpk)
        progress(tm.time()-tstart, i, ngbt)

    print "\nLoaded GBT Events"
    i = 0
    # get all fit information
    tstart = tm.time()
    nfit = tpfit_tree.GetEntries()
    while tpfit_tree.GetEntry(i):
        i += 1
#        if (i % 100000 == 0):
#            print "Unpacking FIT event",i
        time = tpfit_tree.Time_sec + tpfit_tree.Time_nsec/pow(10.,9)
        bcid = tpfit_tree.BCID
        mxloc = tpfit_tree.mxlocal
        nevent = tpfit_tree.EventNum
        nhit = tpfit_tree.tpfit_n
        cntr = tpfit_tree.cntr

        fitpk = FitPacket(nevent,time,bcid,mxloc,nhit,cntr)
        
        mmfes = [im for im in tpfit_tree.tpfit_MMFE8]
        vmms = [iv for iv in tpfit_tree.tpfit_VMM]
        chs = [ich for ich in tpfit_tree.tpfit_CH]

        for j in range(len(mmfes)):
            fitpk.add(mmfes[j],vmms[j],chs[j])
        fitpks.append(fitpk)
        progress(tm.time()-tstart, i, nfit)

    print "\nLoaded FIT events"
    ngbtpks = len(gbtpks)
    nfitpks = len(fitpks)
    print "Nfit events", nfitpks
    print "Ngbt events", ngbtpks
    currifitpk = 0

    output = ROOT.TFile(args.o,"recreate")

    t_eventnum = array("i",[0])
    t_cntr = array("i",[0])
    t_timesec = array("i",[0])
    t_timensec = array("i",[0])
    t_bcid = array("i",[0])
    t_mxlocal = array("f",[0])
    t_tpfit_VMM = ROOT.vector("int")()
    t_tpfit_CH = ROOT.vector("int")()
    t_tpfit_MMFE8 = ROOT.vector("int")()
    t_tpfit_BCID = ROOT.vector("int")()
    t_tpfit_n = array("i",[0])
    
    t = ROOT.TTree("TPcomb_data","TPcomb_data")
    t.Branch("EventNum", t_eventnum, "EventNum/I")
    t.Branch("cntr",  t_cntr, "cntr/I")
    t.Branch("Time_sec",  t_timesec, "Time_sec/I" )
    t.Branch("Time_nsec",  t_timensec, "Time_nsec/I")
    t.Branch("BCID",  t_bcid, "BCID/I")
    t.Branch("mxlocal", t_mxlocal, "mxlocal/F")
    t.Branch("tpfit_VMM", t_tpfit_VMM)
    t.Branch("tpfit_CH", t_tpfit_CH)
    t.Branch("tpfit_MMFE8", t_tpfit_MMFE8)
    t.Branch("tpfit_BCID", t_tpfit_BCID)
    t.Branch("tpfit_n",t_tpfit_n, "tpfit_n/I")

    tstart = tm.time()
    matchedFitEvents = []
    for ig, gbtpk in enumerate(gbtpks):
        j = currifitpk
        while True:
            if (j == nfitpks):
                break
            if (fitpks[j].time - gbtpk.time) > 0.1: #we've gone too far
                break
            if abs(fitpks[j].time - gbtpk.time) < 0.1:
                if fitpks[j].bcid in gbtpk.bcids:
                    matchedFitEvents.append(j)
                    for (mmfe, vmm, ch, bcid) in gbtpk.hits:
                        for (tpmmfe, tpvmm, tpch) in fitpks[j].hits:
                            if (mmfe, vmm, ch) == (tpmmfe,tpvmm,tpch):
                                fitpks[j].addmod(tpmmfe, tpvmm, tpch, bcid)
                                t_tpfit_VMM.push_back(tpvmm)
                                t_tpfit_CH.push_back(tpch)
                                t_tpfit_MMFE8.push_back(tpmmfe)
                                t_tpfit_BCID.push_back(bcid)
                    currifitpk = j+1
                    t_mxlocal[0] = fitpks[j].mxl
                    t_bcid[0] = fitpks[j].bcid
                    t_tpfit_n[0] = fitpks[j].nhit
                    t_cntr[0] = fitpks[j].cntr
                    t_eventnum[0] = fitpks[j].nevent
                    t_timesec[0] = fitpks[j].timesec
                    t_timensec[0] = fitpks[j].timensec
                    t.Fill()
                    t_tpfit_VMM.clear()
                    t_tpfit_CH.clear()
                    t_tpfit_MMFE8.clear()
                    t_tpfit_BCID.clear()
            j += 1
        if (j == nfitpks):
            continue
        progress(tm.time()-tstart, ig, ngbtpks)
    print "\nChecking for missed fit packets"
    nmiss = 0
    tstart = tm.time()
    setmatched = set(matchedFitEvents)
    setall = set(range(nfitpks))
    missing = setall.difference(setmatched)
    #print "Missing events: ", missing
    # loop through missing events to add them with BCID -1
    for k in missing:
        nmiss += 1
        for (tpmmfe, tpvmm, tpch) in fitpks[k].hits:
            t_tpfit_VMM.push_back(tpvmm)
            t_tpfit_CH.push_back(tpch)
            t_tpfit_MMFE8.push_back(tpmmfe)
            t_tpfit_BCID.push_back(-1)
        t_mxlocal[0] = fitpks[k].mxl
        t_bcid[0] = fitpks[k].bcid
        t_tpfit_n[0] = fitpks[k].nhit
        t_cntr[0] = fitpks[k].cntr
        t_eventnum[0] = fitpks[k].nevent
        t_timesec[0] = fitpks[k].timesec
        t_timensec[0] = fitpks[k].timensec
        t.Fill()
        t_tpfit_VMM.clear()
        t_tpfit_CH.clear()
        t_tpfit_MMFE8.clear()
        t_tpfit_BCID.clear()
        progress(tm.time()-tstart, k, len(missing))

        
    output.Write()
    output.Close()

    print "\nMissing:", nmiss
        
def progress(time_diff, nprocessed, ntotal):
    nprocessed, ntotal = float(nprocessed), float(ntotal)
    rate = (nprocessed+1)/time_diff
    msg = "\r > %6i / %6i | %2i%% | %8.2fHz | %6.1fm elapsed | %6.1fm remaining"
    msg = msg % (nprocessed, ntotal, 100*nprocessed/ntotal, rate, time_diff/60, (ntotal-nprocessed)/(rate*60))
    sys.stdout.write(msg)
    sys.stdout.flush()

class GBTPacket(object):
    def __init__(self, time):
        self.time  = time
        self.hits  = []
        self.boards = [118, 116, 102, 119, 106, 107, 117, 105]
        self.bcids = []
        
    def add(self, mmfe, vmm, ch, bcid):
        self.hits.append((int(mmfe), int(vmm), int(ch), int(bcid)))
        self.bcids.append(int(bcid))
        self.hits.sort()
            
    def pad(self):
        for ibo in self.boards:
            if not any([ibo==mmfe for (mmfe, vmm, ch, bcid) in self.hits]):
                self.hits.append((ibo, 0, 0, 0))
        self.hits.sort()
        
class FitPacket(object):
    def __init__(self, n, time, bcid, mxl, nhit, cntr):
        self.nevent   = n
        self.time     = time
        self.timesec  = int(time)/pow(10,9)
        self.timensec = int(time)%pow(10,9)
        self.hits     = []
        self.modhits  = []
        self.bcid     = bcid
        self.mxl      = mxl
        self.nhit     = nhit
        self.cntr     = cntr

    def add(self, mmfe, vmm, ch):
        self.hits.append((int(mmfe), int(vmm), int(ch)))
        self.hits.sort()

    def addmod(self, mmfe, vmm, ch, bcid):
        self.modhits.append((int(mmfe), int(vmm), int(ch), int(bcid)))
        self.modhits.sort()

if __name__ == "__main__":
    main(sys.argv[1:])

"""
combineTPSC.py -- a script to combine the output of combineTPMM and tptime2root.

Run like:
> python combineTPTIME.py --mmtp mmtp.root --tptime tptime.root --out comb.root
"""
import argparse
import array
import copy
import sys
import time
import ROOT

ints    = ["tpsci_EventNum","tpsci_BCID","tpsci_ph","tpsci_overflow","tpsci_Time_sec","tpsci_Time_nsec"]

deltat_max = 0.5

def main():

    ops = options()

    # gather ye inputs
    fTPTIME = ops.tptime or fatal("Please provide a --tptime input ROOT file")
    fMMTP = ops.mmtp or fatal("Please provide a --mmtp input ROOT file")

    fTPTIME = ROOT.TFile.Open(fTPTIME)
    fMMTP = ROOT.TFile.Open(fMMTP)

    trTPTIME = fTPTIME.Get("TPtime_data")
    trMMTP = fMMTP.Get("COMB_data")

    if not trTPTIME: fatal("Couldnt find TTree (TPtime_data) within %s" % (ops.tptime))
    if not trMMTP: fatal("Couldnt find TTree (COMB_data) within %s"      % (ops.mmtp))

    entsTPTIME = trTPTIME.GetEntries()
    entsMMTP = trMMTP.GetEntries() if not ops.max else min(int(ops.max), trMMTP.GetEntries())

    print
    print "input MM+TP  : %s" % (fMMTP.GetName())
    print "input TP time  : %s" % (fTPTIME.GetName())
    print "output    : %s" % (ops.out)
    print
    print "MM+TP events : %i" % (entsMMTP)
    print "TPTIME events : %i" % (entsTPTIME)
    print

    start_time = time.time()
    entTPTIME = 0
    entTPTIME_prev = 0

    nMMTPmatch = 0

    # configure output
    # add branches to our cloned tree
    outfile = ROOT.TFile(ops.out, "recreate")
    clonetree = trMMTP.CloneTree(0)
    treedict = {}
    for var in ints:
        treedict[var] = array.array("i", [0])
        clonetree.Branch(var, treedict[var], "%s/%s" % (var, "I"))

    # MM loop
    entMMTP = 0
    offset = 1
    while entMMTP < entsMMTP: 
        
        _ = trMMTP.GetEntry(entMMTP)
        if entMMTP % 100 == 0 and entMMTP > 0:
            progress(time.time() - start_time, entMMTP, entsMMTP)

        # clear the TP branches
        reset(treedict)

        # gather time and hits
        tMMTP = (trMMTP.sci_Time_sec + trMMTP.sci_Time_nsec/pow(10, 9.0))

        ntr = 0
        maxshared = 0
        entsTP = trTPTIME.GetEntries()
        all_shared = True

        # look for triggers
        while True:
            _ = trTPTIME.GetEntry(trMMTP.sci_EventNum-offset)
            
            # time difference
            tTPTIME = trTPTIME.Time_sec + trTPTIME.Time_nsec/pow(10, 9.0)
            deltat = abs(tTPTIME - tMMTP)
            debug = "  MMTPe = %s TPTIMEe = %s dt = %7.4f "  % (trMMTP.sci_EventNum, trTPTIME.EventNum, deltat)

            if deltat > deltat_max:
                print
                verbose(color.RED + debug + (" No match! Skipping MMTP EventNum = %s " %(trMMTP.sci_EventNum) + color.END))
                print
                offset -= 1
                entTPTIME = entTPTIME_prev
                #sys.exit()
                break

            else:
                treedict["tpsci_EventNum"][0]  = trTPTIME.EventNum
                treedict["tpsci_Time_sec"][0] = trTPTIME.Time_sec
                treedict["tpsci_Time_nsec"][0] = trTPTIME.Time_nsec
                treedict["tpsci_BCID"][0]      = trTPTIME.BCID
                treedict["tpsci_overflow"][0]  = trTPTIME.overflow
                treedict["tpsci_ph"][0]        = trTPTIME.ph

                entMMTP += 1
                entTPTIME += 1
                entTPTIME_prev = entTPTIME
                nMMTPmatch += 1

                break
            
        # write
        clonetree.Fill()

    # save
    clonetree.GetCurrentFile().Write()
    clonetree.GetCurrentFile().Close()

    print
    print
    print "Found %i MMTP events" % (entsMMTP)
    print "Found %i MMTP events with a matched trigger" % (nMMTPmatch)
    print
    print "Done! >^.^<"
    print

def verbose(msg):
    if "--verbose" in sys.argv:
        print msg

def fatal(msg):
    sys.exit("Fatal error: %s" % (msg))

def reset(treedict):
    for this in ints:
        treedict[this][0] = 0

class color:
    BLUE  = "\033[94m"
    GREEN = "\033[92m"
    RED   = "\033[91m"
    GRAY  = "\033[90m"
    END   = "\033[0m"

def options():
    parser = argparse.ArgumentParser(usage=__doc__, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--mmtp",      help="input MM+TP file")
    parser.add_argument("--tptime",      help="input TP time file")
    parser.add_argument("--max",     help="max number of MM events")
    parser.add_argument("--out",     help="output ROOT file", default="test.root")
    parser.add_argument("--verbose", help="crank up the verbosity", action="store_true")
    return parser.parse_args()

def progress(time_diff, nprocessed, ntotal):
    nprocessed, ntotal = float(nprocessed), float(ntotal)
    rate = (nprocessed+1)/time_diff
    msg = "\r > %6i / %6i | %2i%% | %8.2fHz | %6.1fm elapsed | %6.1fm remaining"
    msg = msg % (nprocessed, ntotal, 100*nprocessed/ntotal, rate, time_diff/60, (ntotal-nprocessed)/(rate*60))
    sys.stdout.write(msg)
    sys.stdout.flush()


if __name__ == "__main__":
    main()

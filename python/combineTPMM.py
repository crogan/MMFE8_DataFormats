"""
combineTPMM.py -- a script to combine MM and TP ntuples.
                  The MM input can be MM-only or MM+scintillator.
                  The TP input can be fit-only or fit+GBT.

Run like:
> python combineTPMM.py --mm mm.root --tp tp.root --out comb.root
"""
import argparse
import array
import copy
import sys
import time
import ROOT

ints    = ["tp_n"]
vecints = ["tp_EventNum", "tp_cntr", "tp_Time_sec", "tp_Time_nsec", "tp_BCID", "tp_hit_n"]
vecdubs = ["tp_mxlocal"]
vecvecs = ["tp_hit_MMFE8", "tp_hit_VMM", "tp_hit_CH", "tp_hit_BCID"]

def main():

    ops = options()

    # gather ye inputs
    fTP = ops.tp or fatal("Please provide a --tp input ROOT file")
    fMM = ops.mm or fatal("Please provide a --mm input ROOT file")

    fTP = ROOT.TFile.Open(fTP)
    fMM = ROOT.TFile.Open(fMM)

    trTP = fTP.Get("TPfit_data") or fTP.Get("TPcomb_data")
    trMM = fMM.Get("MM_data")    or fMM.Get("COMB_data")

    if not trTP: fatal("Couldnt find TTree (TPfit_data or TPcomb_data) within %s" % (ops.tp))
    if not trMM: fatal("Couldnt find TTree (MM_data or COMB_data) within %s"      % (ops.mm))

    # assess what inputs were dealing with
    combMM = "comb" in trMM.GetName().lower()
    combTP = "comb" in trTP.GetName().lower()
    if not combTP:
        vecvecs.remove("tp_hit_BCID")

    entsTP = trTP.GetEntries()
    entsMM = trMM.GetEntries() if not ops.max else min(int(ops.max), trMM.GetEntries())

    print
    print "input MM  : %s" % (fMM.GetName())
    print "input TP  : %s" % (fTP.GetName())
    print "output    : %s" % (ops.out)
    print
    print "MM: MM+Scint." if combMM else "MM: MM-only"
    print "TP: Fit+GBT"   if combTP else "TP: Fit-only"
    print
    print "MM events : %i" % (entsMM)
    print "TP events : %i" % (entsTP)
    print

    start_time = time.time()
    entTP = 0
    entTP_prev = 0

    nMMcands = 0
    nMMmatch = 0
    nMMtrigs = 0

    # configure output
    # add branches to our cloned tree
    outfile = ROOT.TFile(ops.out, "recreate")
    clonetree = trMM.CloneTree(0)
    treedict = {}
    for var in ints:
        treedict[var] = array.array("i", [0])
        clonetree.Branch(var, treedict[var], "%s/%s" % (var, "I"))
    for var in vecints:
        treedict[var] = ROOT.vector("int")()
        clonetree.Branch(var, treedict[var])
    for var in vecdubs:
        treedict[var] = ROOT.vector("double")()
        clonetree.Branch(var, treedict[var])
    for var in vecvecs:
        treedict[var] = ROOT.vector(ROOT.vector("int"))()
        clonetree.Branch(var, treedict[var])

    # MM loop
    for entMM in xrange(entsMM):

        _ = trMM.GetEntry(entMM)
        if entMM % 100 == 0 and entMM > 0:
            progress(time.time() - start_time, entMM, entsMM)

        # clear the TP branches
        reset(treedict)

        # gather time and hits
        tMM = (trMM.mm_Time_sec + trMM.mm_Time_nsec/pow(10, 9.0)) if combMM else (trMM.Time_sec + trMM.Time_nsec/pow(10, 9.0))
        hits_mm = zip(list(trMM.mm_MMFE8), list(trMM.mm_VMM), list(trMM.mm_CH))
        verbose("Event %i :: %i hits" % (entMM, len(hits_mm)))

        # triggerable?
        if not is_triggerable(hits_mm):
            verbose("  Not triggerable!")
            triggerable = True
        else:
            nMMcands += 1
            triggerable = False

        ntr = 0
        maxshared = 0
        entsTP = trTP.GetEntries()

        # look for triggers
        while True:

            _ = trTP.GetEntry(entTP)

            # time difference
            tTP = trTP.Time_sec + trTP.Time_nsec/pow(10, 9.0)
            deltat = tTP - tMM
            debug = "  entMM = %s  entTP = %s  dt = %7.4f "  % (entMM, entTP, deltat)

            # at the end!
            if entTP == entsTP:
                verbose(color.RED + debug + (" No more triggers! Resetting to TP = %s " % (entTP_prev)) + color.END)
                entTP = entTP_prev
                break

            # too soon
            if deltat < -0.2:
                verbose(color.GRAY + debug + " Found unrelated trigger." + color.END)
                entTP += 1
                continue

            # passed it -- this MM event has no triggers
            if deltat > 1.0 and ntr == 0:
                verbose(color.RED + debug + (" No triggers for this event. Resetting to TP = %s " % (entTP_prev)) + color.END)
                entTP = entTP_prev
                break

            # evaluate goodness of matching
            hits_tp = zip(list(trTP.tpfit_MMFE8), list(trTP.tpfit_VMM), list(trTP.tpfit_CH))
            nshared = len(filter(lambda hit: hit in hits_mm, hits_tp))
            debug += "nhits = %s  nshar = %s " % (len(hits_tp), nshared)

            if nshared == len(hits_tp) or nshared == len(hits_tp)-1:
                verbose(color.GREEN + debug + " Found matching trigger." + color.END)
                ntr += 1
                entTP += 1
                entTP_prev = entTP
                nMMtrigs += 1
                treedict["tp_n"][0] += 1
                treedict["tp_EventNum"]    .push_back(trTP.EventNum)
                treedict["tp_cntr"]        .push_back(trTP.cntr)
                treedict["tp_Time_sec"]    .push_back(trTP.Time_sec)
                treedict["tp_Time_nsec"]   .push_back(trTP.Time_nsec)
                treedict["tp_BCID"]        .push_back(trTP.BCID)
                treedict["tp_mxlocal"]     .push_back(trTP.mxlocal)
                treedict["tp_hit_n"]       .push_back(trTP.tpfit_n)
                treedict["tp_hit_MMFE8"]   .push_back(copy.deepcopy(trTP.tpfit_MMFE8))
                treedict["tp_hit_VMM"]     .push_back(copy.deepcopy(trTP.tpfit_VMM))
                treedict["tp_hit_CH"]      .push_back(copy.deepcopy(trTP.tpfit_CH))
                if combTP:
                    treedict["tp_hit_BCID"].push_back(copy.deepcopy(trTP.tpfit_BCID))

            elif ntr == 0:
                verbose(color.GRAY + debug + " Found unrelated trigger.")
                entTP += 1
                continue
            else:
                # dont touch entTP! start next MM event from here.
                verbose(color.BLUE + debug + " Next trigger doesnt match! Done with this MM. " + color.END)
                nMMmatch += 1
                break

        # write
        clonetree.Fill()

    # save
    clonetree.GetCurrentFile().Write()
    clonetree.GetCurrentFile().Close()

    print
    print
    print "Found %i MM events" % (entsMM)
    print "Found %i MM events, triggerable" % (nMMcands)
    print "Found %i MM events with a matched trigger" % (nMMmatch)
    print " %4.2f triggers per matched MM"     % (float(nMMtrigs) / float(nMMmatch))
    print
    print "Done! >^.^<"
    print

def is_triggerable(hits):
    # a simple superset of actual triggerability
    if len(hits) < 4:
        return False
    bos = [bo for (bo, vmm, ch) in hits]
    bos = list(set(bos))
    return len(bos) >= 4

def verbose(msg):
    if "verbose" in sys.argv:
        print msg

def fatal(msg):
    sys.exit("Fatal error: %s" % (msg))

def reset(treedict):
    for this in ints:
        treedict[this][0] = 0
    for this in vecints + vecdubs:
        treedict[this].clear()
    for this in vecvecs:
        for vec in treedict[this]:
            vec.clear()
        treedict[this].clear()

class color:
    BLUE  = "\033[94m"
    GREEN = "\033[92m"
    RED   = "\033[91m"
    GRAY  = "\033[90m"
    END   = "\033[0m"

def options():
    parser = argparse.ArgumentParser(usage=__doc__, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--tp",  help="input TP file")
    parser.add_argument("--mm",  help="input MM file")
    parser.add_argument("--max", help="max number of MM events")
    parser.add_argument("--out", help="output ROOT file", default="test.root")
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

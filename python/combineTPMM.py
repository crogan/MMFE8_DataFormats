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
vecints = ["tp_EventNum", "tp_EventNumGBT", "tp_cntr", "tp_Time_sec", "tp_Time_nsec", "tp_BCID", "tp_hit_n", "tp_BCID_L0"]
vecdubs = ["tp_mxlocal"]
vecvecs = ["tp_hit_MMFE8", "tp_hit_VMM", "tp_hit_CH", "tp_hit_BCID"]

deltat_min = -1
deltat_max = 1

ignoreMMFE8 = [6, 7, 8, 9]

def main():

    ops = options()

    # gather ye inputs
    fTP = ops.tp or fatal("Please provide a --tp input ROOT file")
    fMM = ops.mm or fatal("Please provide a --mm input ROOT file")

    fTP = ROOT.TFile.Open(fTP)
    fMM = ROOT.TFile.Open(fMM)

    trTP = fTP.Get("TPfit_data") or fTP.Get("TPcomb_data")
    trMM = fMM.Get("vmm")

    if not trTP: fatal("Couldnt find TTree (TPfit_data or TPcomb_data) within %s" % (ops.tp))
    if not trMM: fatal("Couldnt find TTree (vmm) within %s"                       % (ops.mm))

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
    nNotShared = 0

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
        evMM = trMM.triggerCounter[0]

        # clear the TP branches
        reset(treedict)

        # gather time and hits
        tMM = (trMM.daq_timestamp_s[0] + trMM.daq_timestamp_ns[0]/pow(10, 9.0))
        hits_mm = []
        mm_chip    = list(trMM.chip)
        mm_boardId = list(trMM.boardId)
        mm_channel = list(trMM.channel)
        mm_channel = [list(ch) for ch in mm_channel]
        mm_bcidL0  = list(trMM.bcid)
        mm_bcidL0  = [list(bc) for bc in mm_bcidL0]
        mm_relbcid = list(trMM.relbcid)
        mm_relbcid = [list(bc) for bc in mm_relbcid]
        for (chs, vmm, mmfe, bcL0s, relbcs) in zip(mm_channel, mm_chip, mm_boardId, mm_bcidL0, mm_relbcid):
            if mmfe in ignoreMMFE8:
                continue
            for (ch, bcL0, relbc) in zip(chs, bcL0s, relbcs):
                hits_mm.append( (mmfe, vmm, ch) )

        verbose("Event %i :: %i hits" % (entMM, len(hits_mm)))

        # triggerable?
        if is_triggerable(hits_mm):
            nMMcands += 1
            triggerable = True
        else:
            verbose("  Not triggerable!")
            triggerable = False

        ntr = 0
        maxshared = 0
        entsTP = trTP.GetEntries()
        all_shared = True

        # look for triggers
        while triggerable:

            _ = trTP.GetEntry(entTP)
            evTP = trTP.EventNum

            # time difference
            tTP = trTP.Time_sec + trTP.Time_nsec/pow(10, 9.0)
            deltat = tTP - tMM
            debug = "  evMM = %s  entMM = %s  evTP = %s  entTP = %s  dt = %7.4f "  % (evMM, entMM, evTP, entTP, deltat)

            # at the end!
            if entTP == entsTP-1:
                verbose(color.RED + debug + (" No more triggers! Resetting to TP = %s " % (entTP_prev)) + color.END)
                entTP = entTP_prev - 500 # this was not derived scientifically
                break

            # too soon
            if deltat < deltat_min:
                verbose(color.GRAY + debug + " Found unrelated trigger." + color.END)
                entTP += 1
                continue

            # passed it
            if deltat > deltat_max:
                verbose(color.GRAY + debug + (" Passed the time window. Next MM event.") + color.END)
                entTP = entTP_prev - 500 # this was not derived scientifically
                break

            # evaluate goodness of matching
            hits_tp = zip(list(trTP.tpfit_MMFE8), list(trTP.tpfit_VMM), list(trTP.tpfit_CH))
            nshared = len(filter(lambda hit: hit in hits_mm, hits_tp))
            debug += "nhits = %s  nshar = %s " % (len(hits_tp), nshared)

            # accept everything in the time window
            if nshared >= 3:
                verbose(color.GREEN + debug + " Found matching trigger. " + color.END)
                if nshared == len(hits_tp)-1 and all_shared:
                    all_shared = False
                ntr += 1
                entTP += 1
                entTP_prev = entTP
                nMMtrigs += 1
                treedict["tp_n"][0] += 1
                treedict["tp_EventNum"]    .push_back(trTP.EventNum)
                treedict["tp_EventNumGBT"] .push_back(trTP.EventNumGBT)
                treedict["tp_cntr"]        .push_back(trTP.cntr)
                treedict["tp_Time_sec"]    .push_back(trTP.Time_sec)
                treedict["tp_Time_nsec"]   .push_back(trTP.Time_nsec)
                treedict["tp_BCID"]        .push_back(trTP.BCID)
                treedict["tp_BCID_L0"]     .push_back(mm_bcidL0[0][0])
                treedict["tp_mxlocal"]     .push_back(trTP.mxlocal)
                treedict["tp_hit_n"]       .push_back(trTP.tpfit_n)
                treedict["tp_hit_MMFE8"]   .push_back(copy.deepcopy(trTP.tpfit_MMFE8))
                treedict["tp_hit_VMM"]     .push_back(copy.deepcopy(trTP.tpfit_VMM))
                treedict["tp_hit_CH"]      .push_back(copy.deepcopy(trTP.tpfit_CH))
                if combTP:
                    treedict["tp_hit_BCID"].push_back(copy.deepcopy(trTP.tpfit_BCID))

            else:
                verbose(color.GRAY + debug + " Found unrelated trigger.")
                entTP += 1
                continue

        # write
        if not all_shared:
            nNotShared += 1
        if triggerable and ntr > 0:
            nMMmatch += 1
        clonetree.Fill()

    # save
    clonetree.GetCurrentFile().Write()
    clonetree.GetCurrentFile().Close()

    print
    print
    print "Found %i MM events" % (entsMM)
    print "Found %i MM events, triggerable" % (nMMcands)
    print "Found %i MM events with a matched trigger" % (nMMmatch)
    try:
        print " %4.2f triggers per matched MM"     % (float(nMMtrigs) / float(nMMmatch))
    except ZeroDivisionError:
        pass
    print "Found %i MM events with a matched trigger where at least 1 trigger has a made up hit" % (nNotShared)
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
    if "--verbose" in sys.argv:
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
    parser.add_argument("--tp",      help="input TP file")
    parser.add_argument("--mm",      help="input MM file")
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

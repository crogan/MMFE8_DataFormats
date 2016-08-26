#!/usr/bin/python

# Decoded output format usable by dat2root                                                                                                                                                                            
# Columns of TimeStamp, FIFO count, Cycle, BCID of trigger, Trigger number, Channel, PDO, TDO, BCID of hit, VMM number, and MMFE8 Board ID                                                                           

# A.Wang, last edited Aug 11, 2016   

import sys, getopt,binstr


def main(argv):
    inputfile = ''
    outputfile = ''
    try:
        opts, args = getopt.getopt(argv, "hi:o:", ["ifile=", "ofile="])
    except getopt.GetoptError:
        print 'decode.py -i <inputfile> -o <outputfile>'
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print 'decode.py -i <inputfile> -o <outputfile>'
            sys.exit()
        elif opt in ("-i", "--ifile"):
            inputfile = arg
        elif opt in ("-o", "--ofile"):
            outputfile = arg

    datafile = open(inputfile, 'r')
    decodedfile = open(outputfile, 'w')

    for line in datafile:
        thisline = line.split()
        if len(thisline) < 2:
            continue
        if thisline[4]=='!Err':
            print "skip"
            continue
        boardid = int(thisline[0])
        machinetime = float(thisline[1])
        fifocount = int(thisline[2])
        cycle = int(thisline[3])
        fifotrig = int(thisline[4], 16)
#        if num_trig != int(fifotrig & 1048575):
#            numwordsread = 0
        num_trig = int(fifotrig & 1048575)
        # print "num_trig: ",num_trig,"\n"
        fifotrig = fifotrig >> 20
        bcid_trig = int(fifotrig & 4095)
        for iword in xrange(7, (len(thisline)), 2): #get rid of peak command/address and fifo bcid/num trig
            word0 = int(thisline[iword],   16)
            word1 = int(thisline[iword+1], 16)
            nonzero = False
            if word0 > 0:
                nonzero = True
                
            # if not word0 > 0:
            #     print "Out of order or no data."
            #     continue
            
            word0 = word0 >> 2       # get rid of first 2 bits (threshold)
            addr  = (word0 & 63) + 1 # get channel number as on GUI
            if not nonzero:
                addr = 0 # if the whole word is zero, make ch num 0 as well
            word0 = word0 >> 6       # get rid of address
            amp   = word0 & 1023     # get amplitude
            
            word0  = word0 >> 10     # ?
            timing = word0 & 255     #
            word0  = word0 >> 8      # we will later check for vmm number
            vmm    = word0 &  7      # get vmm number
        
            bcid_gray = int(word1 & 4095)
            bcid_bin  = binstr.b_gray_to_bin(binstr.int_to_b(bcid_gray, 16))
            bcid_int  = binstr.b_to_int(bcid_bin)
            
            word1 = word1 >> 12      # later we will get the turn number
            word1 = word1 >> 4       # 4 bits of zeros?
            immfe = int(word1 & 255) # do we need to convert this?
            
            to_print = "WORD0=%s WORD1=%s CHword=%s PDO=%s TDO=%s BCID=%s BCIDgray=%s VMM=%s MMFE8=%s"
            header = "MachineTime=%0.f FIFO=%s Cycle=%s BCIDtrig=%s Ntrig=%s "
            decodedfile.write(header % (machinetime, fifocount, cycle, bcid_trig, num_trig) + to_print % (thisline[iword], thisline[iword+1],
                                                                                      str(addr),     str(amp), str(timing),
                                                                                      str(bcid_int), str(bcid_gray), 
                                                                                      str(vmm), str(boardid)) + '\n')

    decodedfile.close()
    datafile.close()
    print "done decoding, exiting \n"
    

if __name__ == "__main__":
    main(sys.argv[1:])

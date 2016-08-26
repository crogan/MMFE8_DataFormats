#!/usr/bin/python

# Simpler decoded output format
# HEADER: "EventNum", trigger number (with cycle), Timestamp in sec, Timestamp in nsec, BCID of trigger
# Columns of VMM Channel, PDO, TDO, BCID of hit, MMFE8 Board ID, FIFO count from that Board

# A.Wang, last edited Aug 18, 2016


import sys, getopt,binstr


def main(argv):
    inputfile = ''
    outputfile = ''
    try:
        opts, args = getopt.getopt(argv, "hi:o:", ["ifile=", "ofile="])
    except getopt.GetoptError:
        print 'decodeSimple.py -i <inputfile> -o <outputfile>'
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print 'decodeSimple.py -i <inputfile> -o <outputfile>'
            sys.exit()
        elif opt in ("-i", "--ifile"):
            inputfile = arg
        elif opt in ("-o", "--ofile"):
            outputfile = arg

    datafile = open(inputfile, 'r')
    decodedfile = open(outputfile, 'w')

    eventnum = 0
#    decodedfile.write('TimeStamp\tFIFO\tCycle\tBCIDtrig\tNtrig\tCH\tPDO\tTDO\tBCID\tVMM\tMMFE8\n')


    for line in datafile:
        thisline = line.split()
        if len(thisline) < 2:
            continue
        if thisline[4]=='!Err':
            continue
        boardid = int(thisline[0])
        machinetime = float(thisline[1])
        fifocount = int(thisline[2])
        if fifocount%2 != 0:
            print "LOST A WORD!!!!!!!!!"
        cycle = int(thisline[3])
        fifotrig = int(thisline[4], 16)
        num_trig = int(fifotrig & 1048575)
        fifotrig = fifotrig >> 20
        bcid_trig = int(fifotrig & 4095)
        for iword in xrange(7, (len(thisline)), 2): #get rid of peak command/address and fifo bcid/num trig
            word0 = int(thisline[iword],   16)
            word1 = int(thisline[iword+1], 16)
            if not word0 > 0:
                print "Out of order or no data."
                continue
            
            word0 = word0 >> 2       # get rid of first 2 bits (threshold)
            addr  = (word0 & 63) + 1 # get channel number as on GUI
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

            timestamp = "%.f"%(machinetime) #convert machine time to right format
            if int(num_trig) != int(eventnum):
                decodedfile.write("EventNum " + str((num_trig)+pow(2,20)*cycle) + " Sec " + timestamp[:len(timestamp)-9] + " NS " + timestamp[len(timestamp)-9:len(timestamp)] + " BCIDtrig " + str(bcid_trig) + '\n')
            eventnum = num_trig
            decodedfile.write(str(vmm) + ' ' + str(addr) + ' ' + str(amp) + ' ' + str(timing) + ' ' + str(bcid_int) + ' '+ str(boardid)+ ' ' + str(fifocount/2) + '\n')

            # old format
            # decodedfile.write("%0.f"%(machinetime) + '\t' + str(fifocount) + '\t' + str(cycle) + '\t' + str(bcid_trig) + '\t' + str(num_trig) + '\t' + '\t'+ str(addr) + '\t' + str(amp) + '\t' + str(timing) + '\t' + str(bcid_int) + '\t'+ str(vmm) +'\t'+ str(boardid)+'\n')

    decodedfile.close()
    datafile.close()
    print "done decoding, exiting \n"
    

if __name__ == "__main__":
    main(sys.argv[1:])

#! /usr/bin/env python

import sys

SC_LINE_LENGTH = 5

def open_file(name, mode):
	'''
	Attempts to open file and prints warning if the file doesn't exist
	'''
	f = open(name, mode)

	if f is None:
		print "Unable to open file %s" % (name)

	return f

def safe_next(file_ptr):
	'''
	Calls next() function of file and catches any StopIteration exception
	that would indicate the end of file
	'''
	ret = None
	try:
		ret = file_ptr.next()
	except StopIteration:
		return None

	return ret

def time_diff(mm_line, scint_line):
	'''
	Return the time difference
	If the difference is more than a second, return the number of seconds
	Else, return the fraction of second difference
	'''
	mm_entries = mm_line.split()
	sc_entries = scint_line.split()

	if mm_entries[0] != 'EventNum' or len(sc_entries) != SC_LINE_LENGTH:
		return 10000000.

	mm_time_s = float(mm_entries[3])
	mm_time_ns = float(mm_entries[5])*1e-9

	sc_time_s = float(sc_entries[2])
	sc_time_ns = float(sc_entries[3])*1e-9
#	print (sc_time_s+sc_time_ns)
#	print (mm_time_s+mm_time_ns)

	return abs((sc_time_s+sc_time_ns)-(mm_time_s+mm_time_ns))


def evt_nums(mm_line, scint_line):
	mm_entries = mm_line.split()
	sc_entries = scint_line.split()

	if mm_entries[0] != 'EventNum' or len(sc_entries) != SC_LINE_LENGTH:
		return 10000000, 0

	mm_evt = int(mm_entries[1])
	sc_evt = int(sc_entries[1])

	return sc_evt, mm_evt

def line_length(line):
	'''
	Return number of entries in a log line
	'''
	return len(line.split())

def write_buffers(sc_buff, mm_buff, out_file):
	'''
	Write out stored buffers of scintillator and MM info 
	with a summary header line
	'''
	mm_evt = 0
	mm_time_s = 0
	mm_time_ns = 0
	mm_hits = 0

	sc_evt = 0
	sc_time_s = 0
	sc_time_ns = 0
	sc_hits = 0
	sc_run = 0

	if ((sc_buff is None and mm_buff is None) or (sc_buff is not None and mm_buff is not None and len(sc_buff) == 0 and len(mm_buff) == 0)):
		return

	if (sc_buff is not None and len(sc_buff) == 0 and mm_buff is None):
		return

	if mm_buff is not None and len(mm_buff) > 0:
		entries = mm_buff[0].split()
		mm_evt = entries[1]
		mm_time_s = entries[3]
		mm_time_ns = entries[5]
		mm_trig_bcid = entries[7]
		mm_hits = len(mm_buff) - 1
	else:
		mm_buff = []

	if sc_buff is not None and len(sc_buff) > 0:
		entries = sc_buff[0].split()
		sc_run = entries[0]
		sc_evt = entries[1]
		sc_time_s = entries[2]
		sc_time_ns = entries[3]
		sc_hits = entries[4]
#		art_hits = entries[5]
	else:
		sc_buff = []

	# out_string = "Start event: " + str(sc_run) + " " + str(sc_evt) + " " + str(mm_evt) + " " + str(sc_time_s) + " " + str(mm_time_s) \
	# + " " + str(sc_time_ns) + " " + str(mm_time_ns) + " " + str(sc_hits) + " " + str(mm_hits) + " " + str(art_hits) + "\n"

        if (len(sc_buff) < 1  or len(mm_buff) < 1):
                out_string = "Start event: " + str(sc_run) + " " + str(sc_evt) + " " + str(mm_evt) + " " + str(sc_time_s) + " " + str(mm_time_s) \
	                     + " " + str(sc_time_ns) + " " + str(mm_time_ns) + " " + "-1" + " " + str(sc_hits) + " " + str(mm_hits) + "\n"
        else:
                out_string = "Start event: " + str(sc_run) + " " + str(sc_evt) + " " + str(mm_evt) + " " + str(sc_time_s) + " " + str(mm_time_s) \
	                     + " " + str(sc_time_ns) + " " + str(mm_time_ns) + " " + str(mm_trig_bcid) + " " + str(sc_hits) + " " + str(mm_hits) + "\n"

	out_file.write(out_string)

	for i in range(1,len(sc_buff)):
		out_file.write(sc_buff[i])

	for i in range(1, len(mm_buff)):
		out_file.write(" MM " + mm_buff[i])

	out_file.write("========= \n")



def write_scint(scint_file, out_file, mm_line, next_event, offset):
	'''
	Write scintillator log lines until you find an event
	which matches the time of the MM event given
	'''
	line_buffer = []
	match_line = None
	if next_event is None:
		next_event = safe_next(scint_file)
	
	while next_event is not None:
		line_buffer.append(next_event)
		#Found an event that matches the MM event
		sc_evt, mm_evt = evt_nums(mm_line, next_event)
		diff = sc_evt - mm_evt
		sc_entries = next_event.split()

		if diff == offset and int(sc_entries[-2]) >= 2:
			#Found a match, so clear the buffer
			write_buffers(line_buffer[0:-1], None, out_file)
			line_buffer = [next_event]

			next_event = safe_next(scint_file)

			# Loop until event is done
			while next_event is not None:
				l = line_length(next_event)

				#Reached the next event
				if l == SC_LINE_LENGTH:
					if time_diff(mm_line, line_buffer[0]) > 2:
						print "WARNING: MM event %d and SC event %d have a time difference bigger than 2 seconds" % (mm_evt, sc_evt)
						print time_diff(mm_line, line_buffer[0])
					return line_buffer, next_event, offset
				else:
					line_buffer.append(next_event)
					next_event = safe_next(scint_file)
		else:

			sc_entries = next_event.split()
			if int(sc_entries[-2]) < 2 and line_length(next_event) == SC_LINE_LENGTH:
				offset += 1

			#Found a new event without finding a match, so clear the buffer
			if line_length(next_event) == SC_LINE_LENGTH:
				write_buffers(line_buffer[0:-1], None, out_file)
				line_buffer = [next_event]

			next_event = safe_next(scint_file)

	return line_buffer, next_event, offset		


def merge_events(mm_name, scint_name, out_name, offset):
	'''
	Loop through MM file and merge scintillator and MM events
	'''
	mm_file = open_file(mm_name, 'r')
	scint_file = open_file(scint_name, 'r')
	out_file = open_file(out_name, 'w')

	if (mm_file is None) or (scint_file is None) or (out_file is None):
		return -1

	mm_line = safe_next(mm_file)
	next_event = None
	mm_buffer = []
	sc_buffer = [] 
	skipped = False

        header= "Start event: " + "run"  + " " + "sc_evt" + " " + "mm_evt" + " " + "sc_time_s" + " " + "mm_time_s" \
                + " " + "sc_time_ns" + " " + "mm_time_ns" + " " + "mm_trig_bcid" + " " + "sc_hits" + " " + "mm_hits" + "\n"

#        out_file.write(header)

	while mm_line is not None:
		if mm_line[0] == '#':
			mm_line = safe_next(mm_file)
			continue

		entries = mm_line.split()

		if (entries[0] == 'EventNum' and (entries[1] == '0')) or not ((entries[0] == 'EventNum') or len(entries) == 7):
			if (entries[0] == 'EventNum' and (entries[1] == '0')):
				skipped = True

			mm_line = safe_next(mm_file)
			continue

		#Found start of MM event
		if entries[0] == 'EventNum':
			if skipped:
				skipped = False
			else:
				write_buffers(sc_buffer, mm_buffer, out_file)
				
			sc_buffer, next_event, offset = write_scint(scint_file, out_file, mm_line, next_event, offset)
			mm_buffer = [mm_line]
			mm_line = safe_next(mm_file)
		else:
			mm_buffer.append(mm_line)
			mm_line = safe_next(mm_file)


if __name__ == '__main__':
	if len(sys.argv) != 5:
		print "Usage: merge_events.py mm_file scintillator_file output_name event_offset"
	else:
		mm_name = sys.argv[1]
		scint_name = sys.argv[2]
		out_name = sys.argv[3]
		offset = int(sys.argv[4])
		print "Merging files %s and %s using offset %d" % (mm_name, scint_name, offset)
		merge_events(mm_name, scint_name, out_name, offset)


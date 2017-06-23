ROOTCFLAGS    = $(shell root-config --cflags)
ROOTGLIBS     = $(shell root-config --glibs)

CXX            = g++
CXXFLAGS       = -fPIC -Wall -O3 -g
CXXFLAGS       += $(filter-out -stdlib=libc++ -pthread , $(ROOTCFLAGS))
GLIBS          = $(filter-out -stdlib=libc++ -pthread , $(ROOTGLIBS))

INCLUDEDIR       = ./include/
SRCDIR           = ./src/
CXX             += -I$(INCLUDEDIR) -I.
OUTOBJ	         = ./obj/

CC_FILES := $(wildcard src/*.cc)
HH_FILES := $(wildcard include/*.hh)
OBJ_FILES := $(addprefix $(OUTOBJ),$(notdir $(CC_FILES:.cc=.o)))
DICT_FILES := $(wildcard include/*.pcm)

all: VectorDict.cxx xADCcalib2dat PDOcalib2dat TDOcalib2dat dat2root raw2dat raw2evtdat raw2root scint2root tp2root tpfit2root gbt2root tptime2root combine2root combine2dat

VectorDict.cxx: $(INCLUDEDIR)VectorDict.hh
	rootcint -f VectorDict.cxx -c $(CXXFLAGS) -p $ $<
	touch VectorDict.cxx

xADCcalib2dat:  $(SRCDIR)xADCcalib2dat.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o xADCcalib2dat $ $< $(GLIBS)
	touch xADCcalib2dat	

PDOcalib2dat:  $(SRCDIR)PDOcalib2dat.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o PDOcalib2dat $ $< $(GLIBS)
	touch PDOcalib2dat

TDOcalib2dat:  $(SRCDIR)TDOcalib2dat.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o TDOcalib2dat $ $< $(GLIBS)
	touch TDOcalib2dat

dat2root:  $(SRCDIR)dat2root.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o dat2root $ $< $(GLIBS)
	touch dat2root

raw2dat:  $(SRCDIR)raw2dat.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o raw2dat $ $< $(GLIBS)
	touch raw2dat

raw2evtdat:  $(SRCDIR)raw2evtdat.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o raw2evtdat $ $< $(GLIBS)
	touch raw2evtdat

raw2root:  $(SRCDIR)raw2root.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o raw2root $ $< $(GLIBS)
	touch raw2root

scint2root:  $(SRCDIR)scint2root.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o scint2root $ $< $(GLIBS)
	touch scint2root

tp2root:  $(SRCDIR)tp2root.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o tp2root $ $< $(GLIBS)
	touch tp2root

tpfit2root:  $(SRCDIR)tpfit2root.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o tpfit2root $ $< $(GLIBS)
	touch tpfit2root

gbt2root:  $(SRCDIR)gbt2root.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o gbt2root $ $< $(GLIBS)
	touch gbt2root

tptime2root:  $(SRCDIR)tptime2root.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o tptime2root $ $< $(GLIBS)
	touch tptime2root

combine2root:  $(SRCDIR)combine2root.C $(OBJ_FILES) $(HH_FILES) $(DICT_FILES)
	$(CXX) $(CXXFLAGS) -o combine2root $ $< $(GLIBS)
	touch combine2root

combine2dat:  $(SRCDIR)combine2dat.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o combine2dat $ $< $(GLIBS)
	touch combine2dat

$(OUTOBJ)%.o: src/%.cc include/%.hh
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OUTOBJ)*.o
	rm -rf *.dSYM
	rm -f VectorDict.cxx
	rm -f VectorDict_rdict.pcm
	rm -f dat2root
	rm -f xADCcalib2dat
	rm -f PDOcalib2dat
	rm -f TDOcalib2dat
	rm -f raw2dat
	rm -f raw2evtdat
	rm -f raw2root
	rm -f scint2root
	rm -f combine2root
	rm -f combine2dat

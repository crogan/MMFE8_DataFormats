ROOTCFLAGS    = $(shell root-config --cflags)
ROOTGLIBS     = $(shell root-config --glibs)

CXX            = g++
CXXFLAGS       = -fPIC -Wall -O3 -g
CXXFLAGS       += $(filter-out -stdlib=libc++ -pthread , $(ROOTCFLAGS))
GLIBS          = $(filter-out -stdlib=libc++ -pthread , $(ROOTGLIBS))

INCLUDEDIR       = ./include/
SRCDIR           = ./src/
CXX	         += -I$(INCLUDEDIR) -I.
OUTOBJ	         = ./obj/

CC_FILES := $(wildcard src/*.cc)
HH_FILES := $(wildcard include/*.hh)
OBJ_FILES := $(addprefix $(OUTOBJ),$(notdir $(CC_FILES:.cc=.o)))

all: xADCcalib2dat PDOcalib2dat TDOcalib2dat dat2root

xADCcalib2dat:  $(SRCDIR)xADCcalib2dat.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o xADCcalib2dat $(GLIBS) $ $<
	touch xADCcalib2dat	

PDOcalib2dat:  $(SRCDIR)PDOcalib2dat.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o PDOcalib2dat $(GLIBS) $ $<
	touch PDOcalib2dat

TDOcalib2dat:  $(SRCDIR)TDOcalib2dat.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o TDOcalib2dat $(GLIBS) $ $<
	touch TDOcalib2dat

dat2root:  $(SRCDIR)dat2root.C $(OBJ_FILES) $(HH_FILES)
	$(CXX) $(CXXFLAGS) -o dat2root $(GLIBS) $ $<
	touch dat2root

$(OUTOBJ)%.o: src/%.cc include/%.hh
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OUTOBJ)*.o
	rm -rf *.dSYM
	rm -f dat2root
	rm -f xADCcalib2dat
	rm -f PDOcalib2dat
	rm -f TDOcalib2dat


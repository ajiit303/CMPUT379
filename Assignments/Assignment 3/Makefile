# ------------------------------------------------------------
# Makefile for CMPUT 379 A3
#
# Usage: make  // compile the programs
#        make tar // create a 'tar.gz' archive of 'allFiles'
#        make clean // remove unneeded files
# ------------------------------------------------------------
#-------------------------------------------------------------
target = submit
allFiles = Makefile Report.pdf main.cpp *.h
#-------------------------------------------------------------

starter:main.cpp main.h message.h
	g++ main.cpp -o a3w22 -pthread

tar:
	tar -cvf $(target).tar $(allFiles)
	gzip $(target).tar

clean:
	rm a3w22

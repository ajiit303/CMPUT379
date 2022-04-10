#-------------------------------------------------------------
# Makefile for CMPUT 379 A4
#
# Usage: make  // compile the programs
#        make tar // create a 'tar.gz' archive of 'allFiles'
#        make clean // remove unneeded files
# ------------------------------------------------------------
#-------------------------------------------------------------
object=a4w22.o
dataFile=jobs.dat
monitorTime=100
NITER=20
allFiles=Makefile main.cc jobs.dat Report.pdf
#-------------------------------------------------------------
a4w22.o:
	g++ -c main.cc

a4w22:$(object)
	g++ main.cc -o a4w22 -lpthread

tar:
	touch gupta-a4.tar.gz
	tar -czvf gupta-a4.tar.gz $(allFiles)

test1:
	./a4w22 $(dataFile) $(monitorTime) $(NITER)

clean:
	rm -rf $(object) a4w22

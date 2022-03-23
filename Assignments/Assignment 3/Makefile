# ------------------------------------------------------------
# Makefile for CMPUT 379 A3
#
# Usage: make  // compile the programs
#        make tar // create a 'tar.gz' archive of 'allFiles'
#        make clean // remove unneeded files
# ------------------------------------------------------------
#-------------------------------------------------------------
target = submit
allFiles = Makefile Report.pdf ex1.dat ex2.dat ex3.dat main.cpp *.h
portNumber = 9060
serverName = localhost
#-------------------------------------------------------------

starter:main.cpp main.h message.h
	g++ main.cpp -o a3w22 -pthread

tar:
	tar -cvf $(target).tar $(allFiles)
	gzip $(target).tar

switch1:
	./a3w22 psw1 ex3.dat null psw2 100-110 $(serverName) $(portNumber)

switch2:
	./a3w22 psw2 ex3.dat psw1 null 200-210 $(serverName) $(portNumber)

master:
	./a3w22 master 2 $(portNumber)

clean:
	rm a3w22
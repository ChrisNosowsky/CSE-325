# Make for Project 10
# Chris Nosowsky
#
# Project 10 Executable

proj10:	proj10.student.o
				g++ proj10.student.o -o proj10

proj10.student.o:	proj10.student.c
				g++ -Wall -c proj10.student.c

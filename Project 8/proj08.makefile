# Make for Project 8
# Chris Nosowsky
#
# Project 8 Executable

proj08:	proj08.student.o
				g++ proj08.student.o -o proj08

proj08.student.o:	proj08.student.c
				g++ -Wall -c proj08.student.c

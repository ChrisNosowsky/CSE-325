# Make for Project 3
# Chris Nosowsky
#
# Project 3 Executable

proj03:	proj03.student.o
				g++	proj03.student.o	-o	proj03

proj03.student.o:	proj03.student.c
				g++	-Wall -c proj03.student.c

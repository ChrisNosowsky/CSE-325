# Make for Project 7
# Chris Nosowsky
#
# Project 7 Executable

proj07:	proj07.student.o
				g++ proj07.student.o -o proj07

proj07.student.o:	proj07.student.c
				g++ -Wall -c proj07.student.c

# Make for Project 9
# Chris Nosowsky
#
# Project 9 Executable

proj09:	proj09.student.o
				g++ proj09.student.o -o proj09

proj09.student.o:	proj09.student.c
				g++ -Wall -c proj09.student.c

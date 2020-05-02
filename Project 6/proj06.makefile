# Make for Project 6
# Chris Nosowsky
#
# Project 6 Executable

proj06:	proj06.student.o
				g++ proj06.student.o -pthread -o proj06

proj06.student.o:	proj06.student.c
				g++ -Wall -lpthread -c proj06.student.c

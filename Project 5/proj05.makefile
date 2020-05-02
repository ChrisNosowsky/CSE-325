# Make for Project 5
# Chris Nosowsky
# 
# Project 5 Executable

proj05:	proj05.student.o
				g++ proj05.student.o -pthread -o proj05

proj05.student.o:	proj05.student.c
				g++ -Wall -lpthread -c proj05.student.c


bib: main.c dyad.c
	gcc main.c dyad.c -o bib -g -Wall

clean:
	rm bib


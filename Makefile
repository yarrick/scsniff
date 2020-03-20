default: scsniff

clean:
	rm -f scsniff

scsniff: scsniff.c
	cc -o $@ $<

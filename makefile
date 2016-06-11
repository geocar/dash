s=$(shell uname -s | tr A-Z a-z)

d: d.$(s)
	cp $< $@.tmp
	chmod +x $@.tmp
	mv $@.tmp $@

d.darwin: d.c
	gcc-6 -pthread -O3 -s -DKXVER=3 -I$(HOME)/q/c -o $@ $< $(HOME)/q/c/m64/c.o 

d.linux: d.c
	gcc -pthread -DMAIN -O3 -s -DKXVER=3 -I$(HOME)/q/c -o $@ $< $(HOME)/q/c/l64/c.o 

main.exe: main.cpe
	cpe2x main.cpe

main.cpe: main.obj renderer.obj
	ccpsx -Xm -Xo$80010000 -O2 main.obj renderer.obj -o main.cpe

main.obj: main.c
	ccpsx -c main.c -o main.obj
renderer.obj: renderer.c
	ccpsx -c renderer.c -o renderer.obj

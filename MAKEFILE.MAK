main.exe: main.cpe
	cpe2x main.cpe

main.cpe: main.obj mesh.obj io.obj input.obj renderer.obj
	ccpsx -Xm -Xo$80010000 -O2 main.obj mesh.obj io.obj input.obj renderer.obj -o main.cpe

main.obj: main.c
	ccpsx -c main.c -o main.obj
mesh.obj: mesh.c
	ccpsx -c mesh.c -o mesh.obj
io.obj: io.c
	ccpsx -c io.c -o io.obj
input.obj: input.c
	ccpsx -c input.c -o input.obj
renderer.obj: renderer.c
	ccpsx -c renderer.c -o renderer.obj

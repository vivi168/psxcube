# 96x96 body.bmp
# 96x48 head.bmp
# 48x48 helmet.bmp
# 48x48 lantern.bmp
# 48x48 lanttop.bmp

# 96x64 cube.bmp

all:
	python3 ../md5.py Bob.md5mesh BOB.MD5M
	python3 ../md5.py --anim Bob.md5anim BOB.MD5A
	bmp2tim -org 640 0 -plt 0 480 body.bmp -o BODY.TIM
	bmp2tim -org 736 0 -plt 0 481 head.bmp -o HEAD.TIM
	bmp2tim -org 736 48 -plt 0 482 helmet.bmp -o HELMET.TIM
	bmp2tim -org 784 48 -plt 0 483 lantern.bmp -o LANTERN.TIM
	bmp2tim -org 832 48 -plt 0 484 lanttop.bmp -o LANTTOP.TIM

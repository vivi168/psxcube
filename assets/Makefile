#  640    736 784 832 880
#     +-----+-----+
#     |     |     |
# 48  |     +--+--+--+
#     |     |  |  |  |
# 96  +-----+-----+--+
#     |     |     |
#     |     |     |
# 160 +-----+-----+
#     |           |
#     +-----------+

all:
	python3 obj.py model2.obj CUBE.M3D
	python3 obj.py house.obj HOUSE.M3D
	python3 md5.py cubeguy.md5mesh CUBEGUY.MD5M
	python3 md5.py --anim running.md5anim RUNNING.MD5A
	bmp2tim -org 640 96 -plt 0 485 cube.bmp -o CUBE.TIM
	bmp2tim -org 736 96 -plt 0 486 house.png -o HOUSE.TIM
	bmp2tim -org 640 160 -plt 0 487 terrain.bmp -o TERRAIN.TIM
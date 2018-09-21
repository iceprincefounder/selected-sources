
# ARNOLD_PATH="/mnt/work/software/arnold/arnold-4.2.10.0"
ARNOLD_PATH=/mnt/work/software/arnold/arnold-5.0.1.0
OUT=out
SRC=src
all:
	c++ ${SRC}/lc_triplanar.cpp ${SRC}/lc_triplanar3d.cpp ${SRC}/shader_loader.cpp -o ${OUT}/lc_triplanar_plus.so -Wall -O2 -shared -fPIC -I${ARNOLD_PATH}/include -L${ARNOLD_PATH}/bin -lai
install:
	@echo "----------------------"	
	@echo "Install ... "
	cp -f ${OUT}/lc_triplanar_plus.so /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders
	cp -f ${SRC}/lc_triplanar_plus.mtd /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders
	cp -f ${SRC}/lc_triplanar.args /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders/Args
	cp -f ${SRC}/lc_triplanar3d.args /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders/Args
	@echo "Done!"
clean:
	rm -f ${OUT}/lc_triplanar_plus.os
 
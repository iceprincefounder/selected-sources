OUT_PATH = out/
INCLUDE_PATH=src/*.h
ARNOLD_PATH = /mnt/work/software/arnold/arnold-5.0.1.0

all:
	@echo "Start Compiling..."
	g++ -o ${OUT_PATH}tempShader1.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/lc_texture_repetition.cpp
	g++ -o ${OUT_PATH}tempShader2.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/lc_texture_blend.cpp
	g++ -o ${OUT_PATH}tempShader3.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/lc_texture_entrance.cpp
	g++ -o ${OUT_PATH}loader.so -c -fPIC -D_LINUX -I${ARNOLD_PATH}/include src/shader_loader.cpp
	g++ -o ${OUT_PATH}lc_tex_repetition.so -shared ${OUT_PATH}tempShader1.so ${OUT_PATH}tempShader2.so ${OUT_PATH}tempShader3.so ${OUT_PATH}loader.so -I{INCLUDE_PATH} -L${ARNOLD_PATH}/bin -lai
	@echo "Done!"

install:
	@echo "Install Shader lc_tex_repetition ... "
	cp -f ${OUT_PATH}lc_tex_repetition.so /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders
	@echo "Done!"
	@echo "Install Args ... "
	cp -f src/lc_texture_repetition.args /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders/Args
	cp -f src/lc_texture_blend.args /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders/Args
	cp -f src/lc_texture_entrance.args /home/xukai/Git/git_repo/katana/shader/5.x/lcaShaders/Args
	@echo "Done!"
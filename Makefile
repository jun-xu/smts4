# for mac os x compile css4.0.
#
LINUX_APP_NAME = smts
PWD = $(shell pwd)
ROOT_DIR = $(PWD)/
# binary dir
BIN_DIR = $(ROOT_DIR)bin/
# *.so dir
LIB_PATH = $(BIN_DIR)
# include dir
INCLUDE_DIR = $(ROOT_DIR)include/
# src dir
SRC_DIR = $(ROOT_DIR)src/
TEST_DIR = $(ROOT_DIR)test/
SRC_FILES = $(foreach dir,$(SRC_DIR),$(wildcard $(dir)*.c))
# deps dir
DEPS_ROOT_DIRS = $(ROOT_DIR)deps/

EXE = $(BIN_DIR)$(LINUX_APP_NAME).boot
EXE_TEST = $(BIN_DIR)$(LINUX_APP_NAME).test.boot

# --------------------------------------------------------

# libuv && gyp.
LIBUV_NAME_APP = uv
LIBUV_GIT_ADDR = git@192.168.203.211:libuv.git
LIBUV_NAME = lib$(LIBUV_NAME_APP)
LIBUV_SOURCE_DIR = $(DEPS_ROOT_DIRS)$(LIBUV_NAME)/
LIBUV_SOURCE_BUILD_DIR = $(LIBUV_SOURCE_DIR)build/
LIBUV_INCLUDE_SOURCE_DIR = $(LIBUV_SOURCE_DIR)include/
LIBUV_INCLUDE_DIR = $(INCLUDE_DIR)$(LIBUV_NAME_APP)/

GYP_NAME_APP = gyp
GYP_GIT_ADDR = git@192.168.203.211:gyp.git
GYP_SOURCE_DEPS_DIR = $(DEPS_ROOT_DIRS)$(GYP_NAME_APP)/
#--------------------------------------------------------

CXX   =  gcc
OPTI  = -o2
CXXFLAGS = -Wall -I. -fPIC -fprofile-arcs -ftest-coverage
INCPATHS = -I$(SRC_DIR) -I$(INCLUDE_DIR)
LIBS =-l$(LIBUV_NAME_APP)

GIT_SUBMODULES_EXISTED=$(wildcard .gitmodules)

all:compile_test


init_smts:
	mkdir -pv bin
	mkdir -pv src
	mkdir -pv test
	mkdir -pv include
	
ifneq ($(GIT_SUBMODULES_EXISTED),.gitmodules)
	@echo "init submodules"
	rm -rf deps/
	mkdir -pv deps
	git submodule add -f git@192.168.203.211:gyp deps/gyp
	git submodule add git@192.168.203.211:libuv deps/libuv
else
	@echo "already init submodules"
endif
	

get_deps:
	@echo init libs.
	@git submodule init && git submodule update
	mkdir -p $(LIBUV_SOURCE_BUILD_DIR) && cp -R $(GYP_SOURCE_DEPS_DIR)../gyp $(LIBUV_SOURCE_BUILD_DIR)
	
	
gen_doc:
	@echo docxy...
	doxygen smts_docxy.cfg
		
compile_uv:
	@echo compile libuv lib ...
	rm -rf $(BIN_DIR)$(LIBUV_NAME)
	mkdir -p $(INCLUDE_DIR)
	mkdir -p $(BIN_DIR)
	rm -rf $(LIBUV_INCLUDE_DIR)
	mkdir -p $(LIBUV_INCLUDE_DIR)
	cp -r $(LIBUV_INCLUDE_SOURCE_DIR) $(LIBUV_INCLUDE_DIR)
	
	cd $(LIBUV_SOURCE_DIR)&&./gyp_uv.py -f xcode&&xcodebuild -ARCHS="x86_64" -project uv.xcodeproj -configuration Release -target All
	cp -r $(LIBUV_SOURCE_BUILD_DIR)Release/libuv.a $(BIN_DIR)

compile_linux_uv:
	@echo compile libuv lib ...
	rm -rf $(BIN_DIR)$(LIBUV_NAME)
	mkdir -p $(INCLUDE_DIR)
	mkdir -p $(BIN_DIR)
	rm -rf $(LIBUV_INCLUDE_DIR)
	mkdir -p $(LIBUV_INCLUDE_DIR)
	cp -r $(LIBUV_INCLUDE_SOURCE_DIR) $(LIBUV_INCLUDE_DIR)
	
	cd $(LIBUV_SOURCE_DIR)&&sh autogen.sh&&./configure --prefix=$(LIBUV_SOURCE_BUILD_DIR)&&make&&make install\
					&&cp -r $(LIBUV_SOURCE_BUILD_DIR)lib/libuv.so* $(BIN_DIR)
	
compile:
	rm -fr $(EXE)
	mkdir -p $(BIN_DIR)
	$(CXX) -o $(EXE) $(SRC_FILES) $(INCPATHS) $(LIBS) -L$(LIB_PATH)
	
compile_linux:
	rm -fr $(EXE)
	mkdir -p $(BIN_DIR)
	$(CXX) -o $(EXE) $(SRC_FILES) $(INCPATHS) $(LIBS) -L$(LIB_PATH)   -Wl,-rpath=$(LIB_PATH)
	
compile_test:
	@echo building compile test...
	rm -fr $(EXE_TEST)
	mkdir -p $(BIN_DIR)
	$(CXX) -g -o $(EXE_TEST) -DMEM_GUARD -DSMTS_TEST  $(SRC_FILES) $(INCPATHS) $(LIBS) -L$(LIB_PATH)
	#cd $(BIN_DIR)&&$(EXE_TEST)

compile_linux_test:
	@echo building compile test...
	rm -fr $(EXE_TEST)
	mkdir -p $(BIN_DIR)
	$(CXX) -g -o $(EXE_TEST) -DMEM_GUARD -DSMTS_TEST  $(SRC_FILES) $(INCPATHS) $(LIBS) -L$(LIB_PATH)   -Wl,-rpath=$(LIB_PATH)
	#cd $(BIN_DIR)&&$(EXE_TEST)


compile_test_macro:
	$(CXX) -g -E  -DSMTS_TEST  $(SRC_FILES) $(INCPATHS) $(LIBS) -L$(LIB_PATH) > $(LINUX_APP_NAME).c
	
clean_all:
	rm -rf $(BIN_DIR)*
	
clean:
	rm -rf $(BIN_DIR)$(LINUX_APP_NAME)*

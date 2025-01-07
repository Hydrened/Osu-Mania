CC = g++
CPP_VERSION = c++17
SRC_DIR = src
BIN_DIR = bin
INCLUDE_DIR = include
LIB_DIR = lib
SDL_TAGS = -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_gfx -lSDL2_ttf
APP_NAME = Osu!_Mania.exe

all:
	make game -j
	make run

game:
	$(CC) -std=$(CPP_VERSION) -m64 -o $(BIN_DIR)/$(APP_NAME) $(SRC_DIR)/*.cpp -I$(INCLUDE_DIR) -I$(INCLUDE_DIR)/game -L$(LIB_DIR) -lmingw32 $(SDL_TAGS) -lH2DE -lminiz

wl:
	$(CC) -std=$(CPP_VERSION) -m64 -mwindows -static-libgcc -static-libstdc++ -o $(BIN_DIR)/$(APP_NAME) $(SRC_DIR)/*.cpp -I$(INCLUDE_DIR) -I$(INCLUDE_DIR)/game -L$(LIB_DIR) -lmingw32 $(SDL_TAGS) -lH2DE -lminiz

run:
	cd $(BIN_DIR) && $(APP_NAME) -f 165

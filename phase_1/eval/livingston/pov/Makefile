INC_DIR:=common/include
SRC_DIR:=common/src
OBJ_DIR:=common/obj
BIN_DIR:=bin

CXX:= clang++

SRC:=$(wildcard $(SRC_DIR)/*.cpp)
OBJ:=$(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.o,$(SRC))

POV:=pov
POV_DIR:=.
POV_OBJ_DIR:=$(POV_DIR)/obj
POV_SRC:=$(wildcard $(POV_DIR)/*.cpp)
POV_OBJ:=$(patsubst $(POV_DIR)%.cpp,$(POV_OBJ_DIR)%.o,$(POV_SRC))

STRIP:=strip
STRIP_FLAGS:=--strip-unneeded

INCLUDE_PATHS:=-I$(INC_DIR)
CPPFLAGS:=
LDFLAGS:=
LIBS:=

PKG_DIR ?= ./pkg
VARIANTS_DIR ?= ./variants

.PHONY: all clean

all: $(POV)

$(POV): $(POV_OBJ) $(OBJ) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) -o $(BIN_DIR)/$@ $^ $(LIBS)

$(POV_OBJ_DIR)/%.o: $(POV_DIR)/%.cpp | $(POV_OBJ_DIR)
	$(CXX) $(INCLUDE_PATHS) $(CPPFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(INCLUDE_PATHS) $(CPPFLAGS) -c $< -o $@

$(POV_OBJ_DIR):
	mkdir $@

$(OBJ_DIR):
	mkdir $@

$(BIN_DIR):
	mkdir $@

clean:
	rm -Rf $(BIN_DIR)
	rm -Rf $(OBJ_DIR)
	rm -Rf $(POV_OBJ_DIR)

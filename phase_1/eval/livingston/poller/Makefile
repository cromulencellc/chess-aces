INC_DIR:=common/include
SRC_DIR:=common/src
OBJ_DIR:=common/obj
BIN_DIR:=bin

CXX:= clang++

SRC:=$(wildcard $(SRC_DIR)/*.cpp)
OBJ:=$(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.o,$(SRC))

POLLER:=poller
POLLER_DIR:=.
POLLER_OBJ_DIR:=$(POLLER_DIR)/obj
POLLER_SRC:=$(wildcard $(POLLER_DIR)/*.cpp)
POLLER_OBJ:=$(patsubst $(POLLER_DIR)%.cpp,$(POLLER_OBJ_DIR)%.o,$(POLLER_SRC))

STRIP:=strip
STRIP_FLAGS:=--strip-unneeded

INCLUDE_PATHS:=-I$(INC_DIR)
CPPFLAGS:=
LDFLAGS:=
LIBS:=

PKG_DIR ?= ./pkg
VARIANTS_DIR ?= ./variants

.PHONY: all clean

all: $(POLLER)

$(POLLER): $(POLLER_OBJ) $(OBJ) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) -o $(BIN_DIR)/$@ $^ $(LIBS)

$(POLLER_OBJ_DIR)/%.o: $(POLLER_DIR)/%.cpp | $(POLLER_OBJ_DIR)
	$(CXX) $(INCLUDE_PATHS) $(CPPFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(INCLUDE_PATHS) $(CPPFLAGS) -c $< -o $@

$(POLLER_OBJ_DIR):
	mkdir $@

$(OBJ_DIR):
	mkdir $@

$(BIN_DIR):
	mkdir $@

clean:
	rm -Rf $(BIN_DIR)
	rm -Rf $(OBJ_DIR)
	rm -Rf $(POLLER_OBJ_DIR)

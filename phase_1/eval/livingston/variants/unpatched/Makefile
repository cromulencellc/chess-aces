INC_DIR:=common/include
SRC_DIR:=common/src
OBJ_DIR:=common/obj
BIN_DIR:=bin

CXX:= clang++-7

SRC:=$(wildcard $(SRC_DIR)/*.cpp)
OBJ:=$(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.o,$(SRC))

CHALLENGE:=livingston
CHALLENGE_DIR:=.
CHALLENGE_OBJ_DIR:=$(CHALLENGE_DIR)/obj
CHALLENGE_SRC:=$(wildcard $(CHALLENGE_DIR)/*.cpp)
CHALLENGE_OBJ:=$(patsubst $(CHALLENGE_DIR)%.cpp,$(CHALLENGE_OBJ_DIR)%.o,$(CHALLENGE_SRC))

POV:=pov
POV_DIR:=pov
POV_OBJ_DIR:=$(POV_DIR)/obj
POV_SRC:=$(wildcard $(POV_DIR)/*.cpp)
POV_OBJ:=$(patsubst $(POV_DIR)%.cpp,$(POV_OBJ_DIR)%.o,$(POV_SRC))

POLLER:=poller
POLLER_DIR:=poller
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

.PHONY: all clean build_variants

all: $(CHALLENGE)

$(CHALLENGE): $(CHALLENGE_OBJ) $(OBJ) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) -o $(BIN_DIR)/$@ $^ $(LIBS)
	$(STRIP) $(STRIP_FLAGS) $(BIN_DIR)/$@

$(POV): $(POV_OBJ) $(OBJ) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) -o $(BIN_DIR)/$@ $^ $(LIBS)

$(POLLER): $(POLLER_OBJ) $(OBJ) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) -o $(BIN_DIR)/$@ $^ $(LIBS)

$(CHALLENGE_OBJ_DIR)/%.o: $(CHALLENGE_DIR)/%.cpp | $(CHALLENGE_OBJ_DIR)
	$(CXX) $(INCLUDE_PATHS) $(CPPFLAGS) -c $< -o $@

$(POV_OBJ_DIR)/%.o: $(POV_DIR)/%.cpp | $(POV_OBJ_DIR)
	$(CXX) $(INCLUDE_PATHS) $(CPPFLAGS) -c $< -o $@

$(POLLER_OBJ_DIR)/%.o: $(POLLER_DIR)/%.cpp | $(POLLER_OBJ_DIR)
	$(CXX) $(INCLUDE_PATHS) $(CPPFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(INCLUDE_PATHS) $(CPPFLAGS) -c $< -o $@

$(CHALLENGE_OBJ_DIR):
	mkdir $@

$(POV_OBJ_DIR):
	mkdir $@

$(POLLER_OBJ_DIR):
	mkdir $@

$(OBJ_DIR):
	mkdir $@

$(BIN_DIR):
	mkdir $@

clean:
	rm -Rf $(BIN_DIR)
	rm -Rf $(OBJ_DIR)
	rm -Rf $(CHALLENGE_OBJ_DIR)
	rm -Rf $(POV_OBJ_DIR)
	rm -Rf $(POLLER_OBJ_DIR)

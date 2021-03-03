PKG_DIR ?= ./pkg
VARIANTS_DIR ?= ./variants

.PHONY: all clean build_variants generate_variants

build_variants:
	for i in variants/*; do (cd $$i; make -j8; cd ..); done

generate_variants:
	docker run --rm -it -v `pwd`:/mnt aces_preproc aces_preproc -d /mnt/challenge -b /mnt/$(VARIANTS_DIR) --fix-chown

all: $(CHALLENGE) $(POV) $(POLLER)

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

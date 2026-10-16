# CS375 Lab 7 - build all programs and the report
CXX      = g++
CXXFLAGS = -pthread -std=c++11 -Wall
SRC      = src
BIN      = bin
PROGRAMS = thread_safe_data_structures priority_queue circular_buffer thread_safe_deque thread_safe_linked_list

all: $(addprefix $(BIN)/,$(PROGRAMS))

$(BIN)/%: $(SRC)/%.cpp | $(BIN)
	$(CXX) -o $@ $< $(CXXFLAGS)

$(BIN):
	mkdir -p $(BIN)

run: all
	@for p in $(PROGRAMS); do echo "===== $$p ====="; ./$(BIN)/$$p; echo; done

report:
	cd report && latexmk -pdf report.tex

clean:
	rm -rf $(BIN)
	cd report && latexmk -C report.tex

.PHONY: all run report clean

CXX      := g++
TARGET   := ThiTracNghiem.exe
SRC_DIR  := src
INC_DIR  := inc
OBJ_DIR  := obj

CXXFLAGS := -std=c++17 -I$(INC_DIR) -MMD -MP

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

run: $(TARGET)
	$(TARGET)

ifeq ($(shell echo %OS%),Windows_NT)
clean:
	-@del /Q obj\*.o obj\*.d $(TARGET) 2>NUL
else
clean:
	-@rm -f obj/*.o obj/*.d $(TARGET)
endif

.PHONY: all run clean

-include $(DEPS)
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g
INCLUDES = -Isrc
LDFLAGS = -lsqlite3

SRCDIR = src
OBJDIR = obj
BINDIR = bin

TARGET = $(BINDIR)/wallet_system

SOURCES = $(SRCDIR)/main.cpp \
          $(SRCDIR)/models/User.cpp \
          $(SRCDIR)/models/Wallet.cpp \
          $(SRCDIR)/security/OTPManager.cpp \
          $(SRCDIR)/security/SecurityUtils.cpp \
          $(SRCDIR)/storage/DatabaseManager.cpp \
          $(SRCDIR)/storage/OTPStorage.cpp \
          $(SRCDIR)/system/AuthSystem.cpp \
          $(SRCDIR)/system/WalletManager.cpp \
          $(SRCDIR)/ui/UserInterface.cpp \
          $(SRCDIR)/ui/UserValidator.cpp

OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

$(shell mkdir -p $(OBJDIR))
$(shell mkdir -p $(BINDIR))
$(shell mkdir -p $(OBJDIR)/models)
$(shell mkdir -p $(OBJDIR)/security)
$(shell mkdir -p $(OBJDIR)/storage)
$(shell mkdir -p $(OBJDIR)/system)
$(shell mkdir -p $(OBJDIR)/ui)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "Linking executable..."
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build completed successfully!"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "Cleaning build files..."
	-@rm -rf $(OBJDIR)
	-@rm -rf $(BINDIR)
	@echo "Clean completed!"

rebuild: clean all

run: $(TARGET)
	@echo "Running the program..."
	./$(TARGET)

debug: CXXFLAGS += -DDEBUG -g3
debug: $(TARGET)

release: CXXFLAGS += -DNDEBUG -O3
release: $(TARGET)

install-deps:
	@echo "Installing dependencies..."
	sudo apt-get update
	sudo apt-get install -y build-essential libssl-dev

setup-dirs:
	@echo "Creating data directories..."
	mkdir -p data backup logs

help:
	@echo "Available targets:"
	@echo "  all         - Build the project (default)"
	@echo "  clean       - Remove build files"
	@echo "  rebuild     - Clean and build"
	@echo "  run         - Build and run the program"
	@echo "  debug       - Build with debug information"
	@echo "  release     - Build optimized release version"
	@echo "  install-deps- Install required dependencies"
	@echo "  setup-dirs  - Create necessary directories"
	@echo "  help        - Show this help message"

.PHONY: all clean rebuild run debug release install-deps setup-dirs help

-include $(OBJECTS:.o=.d)

$(OBJDIR)/%.d: $(SRCDIR)/%.cpp
	@set -e; rm -f $@; \
	$(CXX) -M $(CXXFLAGS) $(INCLUDES) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJDIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$ 
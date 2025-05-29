# Compiler và flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g
INCLUDES = -Isrc
LDFLAGS =

# Thư mục
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# File thực thi
TARGET = $(BINDIR)/wallet_system

# Liệt kê tất cả file .cpp
SOURCES = $(SRCDIR)/main.cpp \
          $(SRCDIR)/models/User.cpp \
          $(SRCDIR)/models/Wallet.cpp \
          $(SRCDIR)/security/OTPManager.cpp \
          $(SRCDIR)/security/SecurityUtils.cpp \
          $(SRCDIR)/storage/DataManager.cpp \
          $(SRCDIR)/system/AuthSystem.cpp \
          $(SRCDIR)/system/WalletManager.cpp \
          $(SRCDIR)/ui/UserInterface.cpp

OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Tạo thư mục nếu chưa tồn tại (Windows compatible)
$(shell if not exist $(OBJDIR) mkdir $(OBJDIR))
$(shell if not exist $(BINDIR) mkdir $(BINDIR))
$(shell if not exist $(OBJDIR)\models mkdir $(OBJDIR)\models)
$(shell if not exist $(OBJDIR)\security mkdir $(OBJDIR)\security)
$(shell if not exist $(OBJDIR)\storage mkdir $(OBJDIR)\storage)
$(shell if not exist $(OBJDIR)\system mkdir $(OBJDIR)\system)
$(shell if not exist $(OBJDIR)\ui mkdir $(OBJDIR)\ui)

# Rule mặc định
all: $(TARGET)

# Rule build executable
$(TARGET): $(OBJECTS)
	@echo "Linking executable..."
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build completed successfully!"

# Rule build object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build files
clean:
	@echo "Cleaning build files..."
	-@if exist $(OBJDIR) rmdir /s /q $(OBJDIR)
	-@if exist $(BINDIR) rmdir /s /q $(BINDIR)
	@echo "Clean completed!"

# Rebuild everything
rebuild: clean all

# Run the program
run: $(TARGET)
	@echo "Running the program..."
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -DDEBUG -g3
debug: $(TARGET)

# Release build
release: CXXFLAGS += -DNDEBUG -O3
release: $(TARGET)

# Install dependencies (Ubuntu/Debian)
install-deps:
	@echo "Installing dependencies..."
	sudo apt-get update
	sudo apt-get install -y build-essential libssl-dev

# Create data directories
setup-dirs:
	@echo "Creating data directories..."
	mkdir -p data backup logs

# Show help
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

# Phony targets
.PHONY: all clean rebuild run debug release install-deps setup-dirs help

# Dependency tracking
-include $(OBJECTS:.o=.d)

$(OBJDIR)/%.d: $(SRCDIR)/%.cpp
	@set -e; rm -f $@; \
	$(CXX) -M $(CXXFLAGS) $(INCLUDES) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJDIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

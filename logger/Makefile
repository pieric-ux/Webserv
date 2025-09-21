#TODO: Don't forget to add 42 header !

OS = $(shell uname)

# Program name
NAME = logger.a

# Source and object directories
SRCDIR = srcs
OBJDIR = objs

# Compiler and flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -Wshadow -MMD -MP -std=c++98
DEBUG_FLAGS = -g3 -fno-omit-frame-pointer -fstack-protector-all

INCLUDES = -I includes

# vpath to specify where to find the .cpp files
vpath %.cpp \
	$(SRCDIR) \

# Sources and object files
SRCES = Logger.cpp

OBJS_SRCES = $(addprefix $(OBJDIR)/, $(SRCES:.cpp=.o))

# Default rule: make all and compile the program
all: $(NAME)

debug: CXXFLAGS = $(DEBUG_FLAGS)

# Rebuild with debug flags
debug: re

# Sanitize
sanitize: DEBUG_FLAGS += -fsanitize=address 
sanitize: debug 

# Rule to compile with Leaks check
leaks:
ifeq ($(OS), Darwin)
	MallocStackLogging=YES leaks --outputGraph=logger.memgraph --fullContent --fullStackHistory --atExit -- ./$(NAME)
else ifeq ($(OS), Linux)
	valgrind --leak-check=full --track-origins=yes --log-file=valgrind.log --show-leak-kinds=all --trace-children=yes --track-fds=all ./$(NAME)
endif

# Compile each .cpp file to .o
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


# Rule to compile the final executable
$(NAME): $(OBJS_SRCES)
	ar -rcs $(NAME) $(OBJS_SRCES)

# Rule to clean up object files
clean:
	@echo "Cleaning object files..."
	rm -rf $(OBJDIR)

# Rule to clean up object files and executable
fclean: clean
	@echo "Cleaning executable $(NAME)..."
	rm -f $(NAME)

# Rule to recompile everything
re: fclean all

.PHONY: all clean fclean re bonus debug sanitize

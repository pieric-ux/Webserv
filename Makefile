#TODO: Don't forget to add 42 header !

OS = $(shell uname)

# Program name
NAME = webserv

# Source and object directories
SRCDIR = srcs
OBJDIR = objs

# Libraries directories
LOGGERDIR = logger
COMMONDIR = common
ABNFDIR = abnf

# Compiler and flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -Wshadow -MMD -MP -std=c++98
DEBUG_FLAGS = -g3 -fno-omit-frame-pointer -fstack-protector-all

INCLUDES = -I includes -I $(ABNFDIR)/includes -I $(LOGGERDIR)/includes -I $(COMMONDIR)/includes

LIBS = -L $(ABNFDIR) -labnf -L $(COMMONDIR) -lcommon -L $(LOGGERDIR) -llogger

# vpath to specify where to find the .cpp files
vpath %.cpp \
	$(SRCDIR) \
	$(SRCDIR)/core \
	$(SRCDIR)/http \

# Sources and object files
SRCES = main.cpp

OBJS_SRCES = $(addprefix $(OBJDIR)/, $(SRCES:.cpp=.o))

# Default rule: make all and compile the program
all: $(NAME)

# Build each library
$(LOGGERDIR)/liblogger.a:
	$(MAKE) -C $(LOGGERDIR)

$(COMMONDIR)/libcommon.a: $(LOGGERDIR)/liblogger.a
	$(MAKE) -C $(COMMONDIR)

$(ABNFDIR)/libabnf.a: $(COMMONDIR)/libcommon.a $(LOGGERDIR)/liblogger.a
	$(MAKE) -C $(ABNFDIR)

debug: CXXFLAGS = $(DEBUG_FLAGS)

# Rebuild with debug flags
debug: re

# Sanitize
sanitize: DEBUG_FLAGS += -fsanitize=address 
sanitize: debug 

# Rule to compile with Leaks check
leaks:
ifeq ($(OS), Darwin)
	MallocStackLogging=YES leaks --outputGraph=webserv.memgraph --fullContent --fullStackHistory --atExit -- ./$(NAME)
else ifeq ($(OS), Linux)
	valgrind --leak-check=full --track-origins=yes --log-file=valgrind.log --show-leak-kinds=all --trace-children=yes --track-fds=all ./$(NAME)
endif

# Compile each .cpp file to .o
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Rule to compile the final executable
$(NAME): $(OBJS_SRCES) $(LOGGERDIR)/liblogger.a $(COMMONDIR)/libcommon.a $(ABNFDIR)/libabnf.a
	$(CXX) $(CXXFLAGS) $(OBJS_SRCES) $(LIBS) -o $(NAME)

# Rule to clean up object files
clean:
	@$(MAKE) clean -C $(LOGGERDIR)
	@$(MAKE) clean -C $(COMMONDIR)
	@$(MAKE) clean -C $(ABNFDIR)
	rm -rf $(OBJDIR)

# Rule to clean up object files and executable
fclean: clean
	@$(MAKE) fclean -C $(LOGGERDIR)
	@$(MAKE) fclean -C $(COMMONDIR)
	@$(MAKE) fclean -C $(ABNFDIR)
	rm -f $(NAME)

# Rule to recompile everything
re: fclean all

.PHONY: all clean fclean re bonus debug sanitize

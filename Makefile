#TODO: Don't forget to add 42 header !

OS = $(shell uname)

# Program name
NAME = webserv

# Source and object directories
SRCDIR = srcs
OBJDIR = objs

# Libraries directories
COMMON_PATH ?= libs/common
LOGGER_PATH ?= libs/logger
ABNF_PATH ?= libs/parser
COMMONDIR = $(COMMON_PATH)
LOGGERDIR = $(LOGGER_PATH)
ABNFDIR = $(ABNF_PATH)

# Compiler and flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -Wshadow -MMD -MP -std=c++98
DEBUG_FLAGS = -g3 -fno-omit-frame-pointer -fstack-protector-all

INCLUDES = -I includes -I $(ABNFDIR)/includes -I $(LOGGERDIR)/includes -I $(COMMONDIR)/includes

LIBS = -L $(ABNFDIR) -labnf -L $(LOGGERDIR) -llog42 -L $(COMMONDIR) -lcommon
# vpath to specify where to find the .cpp files
vpath %.cpp \
	$(SRCDIR) \
	$(SRCDIR)/client \
	$(SRCDIR)/config \
	$(SRCDIR)/handler \
	$(SRCDIR)/headers \
	$(SRCDIR)/MIMEtypes \
	$(SRCDIR)/parser \
	$(SRCDIR)/status \

# Sources and object files
SRCES =  main.cpp HTTPServer.cpp ServerFactory.cpp Server.cpp
		 # Config
SRCES += ErrorPage.cpp HTTPConfig.cpp LocationConfig.cpp ServerConfig.cpp
		 # Handler
SRCES += ClientHandler.cpp ExecutionHandler.cpp RequestHandler.cpp ResponseHandler.cpp
		 # Client
SRCES += Client.cpp Request.cpp Response.cpp HTTPError.cpp
		 # Headers
SRCES += HTTPHeader.cpp HTTPHeadersRegistry.cpp
		 # MIMETypes
SRCES += TypesRegistry.cpp
		 # Parser
SRCES += Parser.cpp
		 # Status
SRCES += StatusCode.cpp StatusCodeRegistry.cpp

OBJS_SRCES = $(addprefix $(OBJDIR)/, $(SRCES:.cpp=.o))

# Default rule: make all and compile the program
all: $(NAME)

# Build each library
$(COMMONDIR)/libcommon.a:
	$(MAKE) -C $(COMMONDIR)
$(LOGGERDIR)/liblog42.a:
	$(MAKE) -C $(LOGGERDIR) COMMON_PATH=$(abspath $(COMMONDIR))
$(ABNFDIR)/libabnf.a:
	$(MAKE) -C $(ABNFDIR) COMMON_PATH=$(abspath $(COMMONDIR)) LOGGER_PATH=$(abspath $(LOGGERDIR))

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
$(NAME): $(OBJS_SRCES) $(LOGGERDIR)/liblog42.a $(COMMONDIR)/libcommon.a $(ABNFDIR)/libabnf.a
	$(CXX) $(CXXFLAGS) $(OBJS_SRCES) $(LIBS) -o $(NAME)

# Rule to clean up object files
clean:
	@$(MAKE) clean -C $(abspath $(COMMONDIR))
	@$(MAKE) clean -C $(abspath $(LOGGERDIR)) COMMON_PATH=$(abspath $(COMMONDIR))
	@$(MAKE) clean -C $(ABNFDIR)
	rm -rf $(OBJDIR)

# Rule to clean up object files and executable
fclean: clean
	@$(MAKE) fclean -C $(abspath $(COMMONDIR))
	@$(MAKE) fclean -C $(abspath $(LOGGERDIR)) COMMON_PATH=$(abspath $(COMMONDIR))
	@$(MAKE) fclean -C $(abspath $(ABNFDIR)) COMMON_PATH=$(abspath $(COMMONDIR)) LOGGER_PATH=$(abspath $(LOGGERDIR))
	rm -f $(NAME)

# Rule to recompile everything
re: fclean all

.PHONY: all clean fclean re bonus debug sanitize

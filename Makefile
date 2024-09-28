CXX = c++

# If c++ isn't clang, use clang++ as CXX compiler
ifeq ($(shell c++ --version 2>&1 | grep -c "clang"),0)
    CXX = clang++
endif

NAME = ircserv
OBJS_DIR := objects
SRC_DIR := src
INCLUDE_DIR := include
CXXFLAGS = -Wall -Wextra -Werror -g3 -std=c++98

SRCS = $(addprefix $(SRC_DIR)/, main.cpp\
	   							Client.cpp\
								Channel.cpp\
								Server.cpp\
								Authenticate.cpp\
								Invite.cpp\
								Join.cpp\
								Kick.cpp\
								Mode.cpp\
								Part.cpp\
								Privmsg.cpp\
								Quit.cpp\
								Topic.cpp\
								Ping.cpp\
								Whois.cpp\
								helper_functions.cpp)
OBJS = $(patsubst $(SRC_DIR)%.cpp, $(OBJS_DIR)%.o, $(SRCS))

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@
	@echo -----------------------
	@echo Binary created
	@echo _______________________

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $? -o $@

.PHONY: all clean fclean re

all: $(NAME)

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -f $(NAME)
	rm -rf .cache
	rm -f compile_commands.json

re: fclean all

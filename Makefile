# **************************************************************************** #
#                                    ft_irc                                    #
# **************************************************************************** #

NAME		= ircserv

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98
INCLUDES	= -I include

SRC_DIR		= src
OBJ_DIR		= obj

SRCS		=	$(SRC_DIR)/main.cpp \
				$(SRC_DIR)/Server.cpp \
				$(SRC_DIR)/ServerHelpers.cpp \
				$(SRC_DIR)/Client.cpp \
				$(SRC_DIR)/Channel.cpp \
				$(SRC_DIR)/Message.cpp \
				$(SRC_DIR)/Utils.cpp \
				$(SRC_DIR)/commands/Cap.cpp \
				$(SRC_DIR)/commands/Pass.cpp \
				$(SRC_DIR)/commands/Nick.cpp \
				$(SRC_DIR)/commands/User.cpp \
				$(SRC_DIR)/commands/Ping.cpp \
				$(SRC_DIR)/commands/Quit.cpp \
				$(SRC_DIR)/commands/Join.cpp \
				$(SRC_DIR)/commands/Part.cpp \
				$(SRC_DIR)/commands/Privmsg.cpp \
				$(SRC_DIR)/commands/Kick.cpp \
				$(SRC_DIR)/commands/Invite.cpp \
				$(SRC_DIR)/commands/Topic.cpp \
				$(SRC_DIR)/commands/Mode.cpp

OBJS		= $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS		= $(OBJS:.o=.d)

all:		$(NAME)

$(NAME):	$(OBJS)
			@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
			@echo "$(NAME) is ready"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
			@mkdir -p $(dir $@)
			@$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

clean:
			@rm -rf $(OBJ_DIR)
			@echo "object files removed"

fclean:		clean
			@rm -f $(NAME)
			@echo "$(NAME) removed"

re:			fclean all

-include $(DEPS)

.PHONY:		all clean fclean re

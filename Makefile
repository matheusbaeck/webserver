# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: glacroix <PGCL>                            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/09/04 18:22:37 by math42            #+#    #+#              #
#    Updated: 2025/01/10 20:38:20 by glacroix         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME        = webserv
SRCSDIR     = .
INCLUDES    = .
OBJDIR      = .obj

SRCS        = $(SRCSDIR)/HttpRequest.cpp \
				$(SRCSDIR)/Selector.cpp \
				$(SRCSDIR)/main.cpp \
				$(SRCSDIR)/Server.cpp \
				$(SRCSDIR)/ConfigFile.cpp \
				$(SRCSDIR)/ServerManager.cpp \
				$(SRCSDIR)/Tokenizer.cpp \
				$(SRCSDIR)/HttpRequest.cpp \
				$(SRCSDIR)/CgiHandler.cpp

OBJS        = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

CC          = g++
CFLAGS      = -g3 -Wall -Wextra -Werror -std=c++98 #-fsanitize=address

LOG_LEVEL   ?= 6
DEFINES     = -DCOUT_LOG_LEVEL=$(LOG_LEVEL)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(DEFINES) -o $@ $^

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	find . -name "*.o" -exec rm -f {} +
	rm -rf $(NAME)
	

re: fclean all

.PHONY: all clean fclean re


# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: glacroix <PGCL>                            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/09/04 18:22:37 by math42            #+#    #+#              #
#    Updated: 2025/01/13 11:26:57 by glacroix         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME        = webserv
SRCSDIR     = src
INCLUDES    = include
OBJDIR      = obj

SRCS        = $(SRCSDIR)/HttpRequest.cpp \
              $(SRCSDIR)/Selector.cpp \
              $(SRCSDIR)/main.cpp \
              $(SRCSDIR)/Server.cpp \
              $(SRCSDIR)/ConfigFile.cpp \
              $(SRCSDIR)/ServerManager.cpp \
              $(SRCSDIR)/Tokenizer.cpp \
              $(SRCSDIR)/CgiHandler.cpp

OBJS = $(patsubst $(SRCSDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

CC          = g++
CFLAGS      = -Wall -Wextra -Werror -std=c++98  # -g3 -fsanitize=address

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCSDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -I$(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	find . -name "*.o" -exec rm -f {} +
	rm -rf $(NAME)
	
re: fclean all

.PHONY: all clean fclean re


# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: math <math@student.42.fr>                  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/09/04 18:22:37 by math42            #+#    #+#              #
#    Updated: 2024/08/30 18:37:26 by aabourri         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME        = webserver
SRCSDIR     = .
INCLUDES    = .
OBJDIR      = .obj

SRCS        = main.cpp\
			  ConfigFile.cpp\
			  HttpRequest.cpp\
			  Tokenizer.cpp\
			  Request.cpp\
			  Worker.cpp\
			  Selector.cpp
OBJS        = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

CC          = g++
CFLAGS      = #-Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	find . -name "*.o" -exec rm -f {} +
	rm -rf $(NAME)
	

re: fclean all

.PHONY: all clean fclean re test


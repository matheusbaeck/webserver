# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: math <math@student.42.fr>                  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/09/04 18:22:37 by math42            #+#    #+#              #
#    Updated: 2024/09/06 22:01:27 by math             ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME        = webserver
SRCSDIR     = .
INCLUDES    = .
OBJDIR      = .obj

SRCS        = $(wildcard $(SRCSDIR)/*.cpp)
OBJS        = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

CC          = g++
CFLAGS      = -g3 -Wall -Wextra -Werror -std=c++98

LOG_LEVEL   ?= 1
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

.PHONY: all clean fclean re test


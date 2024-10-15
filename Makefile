# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: math <math@student.42.fr>                  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/09/04 18:22:37 by math42            #+#    #+#              #
#    Updated: 2024/10/15 17:56:25 by aabourri         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME        = webserver
SRCSDIR     = .
INCLUDES    = .
OBJDIR      = .obj

SRCS        = $(wildcard $(SRCSDIR)/*.cpp)
OBJS        = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

CC          = g++
CFLAGS      = -g3 -Wall -Wextra -Werror -std=c++98 -fsanitize=address

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


# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: faharila <faharila@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/10/12 11:22:51 by faharila          #+#    #+#              #
#    Updated: 2026/10/13 14:35:21 by faharila         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = codexion

CC = cc

CFLAGS = -Wall -Werror -Wextra -pthread

DIRNAME = src

SRC = $(DIRNAME)/cleanup.c $(DIRNAME)/dongle_actions.c $(DIRNAME)/get_timestamp_ms.c $(DIRNAME)/init_coders.c \
	  $(DIRNAME)/init_dongles.c $(DIRNAME)/main.c $(DIRNAME)/monitor.c $(DIRNAME)/parsing.c $(DIRNAME)/release_dongle.c \
	  $(DIRNAME)/routine_coder.c $(DIRNAME)/simulation_state.c $(DIRNAME)/take_dongle.c \
	  $(DIRNAME)/waiting_queue.c

OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	ar rcs $(NAME) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o$@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

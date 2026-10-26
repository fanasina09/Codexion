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

HEADER = $(DIRNAME)/codexion.h $(DIRNAME)/heap.h

SRC = $(DIRNAME)/cleanup.c $(DIRNAME)/coder_state.c \
	  $(DIRNAME)/get_timestamp_ms.c $(DIRNAME)/heap.c \
	  $(DIRNAME)/heap_ops.c $(DIRNAME)/init_coders.c \
	  $(DIRNAME)/init_dongles.c $(DIRNAME)/main.c $(DIRNAME)/monitor.c \
	  $(DIRNAME)/parsing.c $(DIRNAME)/print.c $(DIRNAME)/release_dongle.c \
	  $(DIRNAME)/routine_coder.c $(DIRNAME)/simulation_state.c \
	  $(DIRNAME)/take_dongle.c $(DIRNAME)/waiting_queue.c

OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -I $(DIRNAME) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

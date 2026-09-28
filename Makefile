NAME = codexion

CC = cc

CFLAGS = -Wall -Werror -Wextra -pthread

SRC = cleanup.c dongle_actions.c get_timestamp_ms.c init_coders.c \
	  init_dongles.c main.c monitor.c parsing.c release_dongle.c \
	  routine_coder.c simulation_state.c take_dongle.c \
	  waiting_queue.c

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

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:46:07 by faharila          #+#    #+#             */
/*   Updated: 2026/11/04 08:04:56 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <limits.h>
# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <time.h>
# include <unistd.h>
# include "heap.h"

typedef enum e_scheduler
{
	SCHEDULER_FIFO,
	SCHEDULER_EDF,
	SCHEDULER_UNKNOWN
}					t_scheduler;

typedef struct s_dongle
{
	int				is_taken;
	t_heap			waiting;
	long			released_at;
}					t_dongle;

typedef struct s_coder
{
	int				id;
	int				nb_compiles;
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	pthread_t		thread;
	long			last_compile_and_start;
	long			first_seq;
	long			start_ms;
	pthread_mutex_t	data_lock;
	struct s_sim	*sim;
}					t_coder;

typedef struct s_sim
{
	int				number_of_coders;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	long			number_of_compiles_required;
	long			dongle_cooldown;
	t_scheduler		scheduler;
	int				running;
	long			start_time;
	pthread_mutex_t	running_lock;
	pthread_mutex_t	print_lock;
	pthread_mutex_t	table_lock;
	pthread_cond_t	table_cond;
	long			next_seq;
	t_dongle		*dongles;
	t_coder			*coders;
}					t_sim;

int					is_valid_number(char *str, int allow_zero);
long				ft_atol(char *str);
int					check_args(int ac, char *av[]);
void				fill_sim(t_sim *sim, char *av[]);
t_dongle			*init_dongles(int number_of_coders);
void				destroy_dongles(t_dongle *dongles, int count);
t_coder				*init_coders(int number_of_coders, t_dongle *dongles,
						t_sim *sim);
void				*routine_coder(void *arg);
int					launch_coders(t_sim *sim);
void				release_dongles(t_coder *coder);
long				get_timestamp_us(void);
long				ms_since(long since_us);
long				get_elapsed_ms(t_sim *sim);
int					take_dongles(t_coder *coder);
long				start_delay(t_coder *coder);
long				start_rank(t_coder *coder);
int					lone_coder(t_coder *coder);
int					is_running(t_sim *sim);
void				stop_simulation(t_sim *sim);
t_scheduler			get_scheduler(char *str);
void				smart_sleep(t_sim *sim, long duration_ms);
void				print_status(t_coder *coder, char *msg);
void				print_dongles(t_coder *coder);
void				print_burnout(t_sim *sim, int id);
long				get_last_compile(t_coder *coder);
int					set_last_compile(t_coder *coder);
int					get_compiles(t_coder *coder);
void				add_compile(t_coder *coder);
void				*monitor(void *arg);
void				cleanup(t_sim *sim);
void				destroy_sync(t_sim *sim);
int					enter_queues(t_coder *coder);
void				leave_queues(t_coder *coder);
long				get_deadline(t_coder *coder);

#endif

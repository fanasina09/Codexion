/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:46:07 by faharila          #+#    #+#             */
/*   Updated: 2026/10/20 12:46:17 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <time.h>
# include <unistd.h>
# include <limits.h>

typedef struct s_dongle
{
	int				is_taken;
	pthread_mutex_t	is_lock;
	pthread_cond_t	is_cond;
	struct s_coder	*waiting[2];
	int				nb_waiting;
	long			released_at;
}					t_dongle;

typedef struct s_coder
{
	int				id;
	int				nb_compliles;
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	pthread_t		thread;
	long			last_compile_and_start;
	struct s_sim	*sim;
}					t_coder;

typedef struct s_sim
{
	long				number_of_coders;
	long				time_to_burnout;
	long				time_to_compile;
	long				time_to_debug;
	long				time_to_refactor;
	long				number_of_compiles_required;
	long				dongle_cooldown;
	int				scheduler;
	int				running;
	long			start_time;
	pthread_mutex_t	running_lock;
	t_dongle		*dongles;
	t_coder			*coders;
}					t_sim;

long				get_timestamp_ms(void);
long				get_elapsed_ms(t_sim *sim);
int					is_valid_number(char *str);
int					check_args(int ac, char *av[]);
void				fill_sim(t_sim *sim, char *av[]);
t_dongle			*init_dongles(int number_of_coders);
t_coder				*init_coders(int number_of_coders, t_dongle *dongles,
						t_sim *sim);
void				*routine_coder(void *arg);
void				launch_coders(t_sim *sim);
void				release_dongle(t_dongle *dongle);
long				get_timestamp_ms(void);
void				take_dongle(t_dongle *dongle, t_coder *coder);
int					is_running(t_sim *sim);
long				ft_atol(char *str);
void				stop_simulation(t_sim *sim);
void				*monitor(void *arg);
void				cleanup(t_sim *sim);
void				smart_sleep(t_sim *sim, long duration_ms);
void				add_to_waiting(t_dongle *dongle, t_coder *coder);
void				remove_from_waiting(t_dongle *dongle, t_coder *coder);
long				get_deadline(t_coder *coder);
t_coder				*get_next_coder(t_dongle *dongle, int scheduler);

#endif

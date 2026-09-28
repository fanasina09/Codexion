/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:46:07 by faharila          #+#    #+#             */
/*   Updated: 2026/09/28 09:47:46 by faharila         ###   ########.fr       */
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

typedef struct s_dongle
{
	int is_taken;            // 0 = libre, 1 = pris
	pthread_mutex_t is_lock; //
	pthread_cond_t	is_cond;
	struct s_coder *waiting[2]; // liste des coders qui attendent ce dongle
	int nb_waiting;             // combien de coders attendent actuellement
	long released_at;           // moment ou il a ete relache (ms)
}					t_dongle;

typedef struct s_coder
{
	int id;           // numero de coder
	int nb_compliles; // combien de fois il a compile
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	pthread_t thread;            // le thread qui le fait tourner
	long last_compile_and_start; // debut de sa dernier compile en ()
	struct s_sim	*sim;
}					t_coder;

typedef struct s_sim
{
	int				number_of_coders;
	int				time_to_burnout;
	int				time_to_compile;
	int				time_to_debug;
	int				time_to_refactor;
	int				number_of_compiles_required;
	int				dongle_cooldown;
	int				scheduler;
	int				running;
	pthread_mutex_t	running_lock;
	t_dongle		*dongles;
	t_coder			*coders;
}					t_sim;

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
void				stop_simulation(t_sim *sim);
void				*monitor(void *arg);
void				cleanup(t_sim *sim);
void				add_to_waiting(t_dongle *dongle, t_coder *coder);
void				remove_from_waiting(t_dongle *dongle, t_coder *coder);
long				get_deadline(t_coder *coder);
t_coder				*get_next_coder(t_dongle *dongle, int scheduler);

#endif

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_type.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nfitahin <nfitahin@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 06:47:27 by nfitahin          #+#    #+#             */
/*   Updated: 2026/10/24 12:10:32 by nfitahin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_TYPE_H
# define FT_TYPE_H

# include <pthread.h>

typedef enum e_scheduler
{
	SCHEDULER_FIFO,
	SCHEDULER_EDF,
	SCHEDULER_LIFO,
	SCHEDULER_UNKNOWN
}						t_scheduler;

typedef struct s_heap_entry
{
	long				priority;
	unsigned long		seq;
	int					coder_id;
}						t_heap_entry;

typedef struct s_heap
{
	struct s_heap_entry	*data;
	int					size;
	int					capacity;
	unsigned long		next_seq;
}						t_heap;

typedef struct s_dongle
{
	int					id;
	pthread_mutex_t		state_lock;
	pthread_cond_t		state_cond;
	int					is_available;
	long long			available_at;
	struct s_heap		queue;
	struct s_env		*env;
	struct s_dongle		*next;
}						t_dongle;

typedef struct s_coder
{
	int					id;
	struct s_dongle		*left_dongle;
	struct s_dongle		*right_dongle;
	long long			time_burnout;
	long long			last_compile;
	int					code_compiled;
	int					have_finished;
	pthread_mutex_t		state_lock;
	struct s_env		*env;
	struct s_coder		*next;
}						t_coder;

typedef struct s_env
{
	struct s_args		*args;
	struct s_coder		*coders;
	struct s_dongle		*dongles;
	long long			start_time;
	int					is_running;
	pthread_mutex_t		print_mutex;
	pthread_mutex_t		state_mutex;
}						t_env;

typedef struct s_args
{
	int					nb_coders;
	int					burnout_time;
	int					compile_time;
	int					debug_time;
	int					refactor_time;
	int					nb_compiles;
	int					dongle_cooldown;
	t_scheduler			scheduler;
}						t_args;

#endif

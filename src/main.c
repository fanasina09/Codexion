/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:46:50 by faharila          #+#    #+#             */
/*   Updated: 2026/10/12 10:00:20 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	init_start(t_sim *sim)
{
	int	i;

	sim->start_time = get_timestamp_us();
	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->coders[i].last_compile_and_start = sim->start_time;
		i++;
	}
}

static int	start_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, routine_coder,
				&sim->coders[i]))
		{
			stop_simulation(sim);
			break ;
		}
		i++;
	}
	return (i);
}

int	launch_coders(t_sim *sim)
{
	pthread_t	monitor_thread;
	int			started;
	int			i;

	init_start(sim);
	if (pthread_create(&monitor_thread, NULL, monitor, sim))
		return (1);
	started = start_coders(sim);
	i = 0;
	while (i < started)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	pthread_join(monitor_thread, NULL);
	return (started != sim->number_of_coders);
}

static int	init_sim(t_sim *sim)
{
	sim->running = 1;
	pthread_mutex_init(&sim->running_lock, NULL);
	pthread_mutex_init(&sim->print_lock, NULL);
	pthread_mutex_init(&sim->table_lock, NULL);
	pthread_cond_init(&sim->table_cond, NULL);
	sim->next_seq = sim->number_of_coders;
	sim->dongles = init_dongles(sim->number_of_coders);
	if (!sim->dongles)
	{
		destroy_sync(sim);
		return (1);
	}
	sim->coders = init_coders(sim->number_of_coders, sim->dongles, sim);
	if (!sim->coders)
	{
		destroy_dongles(sim->dongles, sim->number_of_coders);
		destroy_sync(sim);
		return (1);
	}
	return (0);
}

int	main(int ac, char *av[])
{
	t_sim	sim;
	int		failed;

	if (ac == 1)
		return (0);
	if (check_args(ac, av))
		return (1);
	fill_sim(&sim, av);
	if (init_sim(&sim))
		return (1);
	failed = launch_coders(&sim);
	cleanup(&sim);
	if (failed)
		fprintf(stderr, "Error: could not start the threads\n");
	return (failed);
}

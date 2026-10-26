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

	sim->start_time = get_timestamp_ms();
	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->coders[i].last_compile_and_start = sim->start_time;
		i++;
	}
}

void	launch_coders(t_sim *sim)
{
	pthread_t	monitor_thread;
	int			i;

	init_start(sim);
	pthread_create(&monitor_thread, NULL, monitor, sim);
	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_create(&sim->coders[i].thread, NULL, routine_coder,
			&sim->coders[i]);
		i++;
	}
	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	pthread_join(monitor_thread, NULL);
}

static int	init_sim(t_sim *sim)
{
	sim->running = 1;
	pthread_mutex_init(&sim->running_lock, NULL);
	pthread_mutex_init(&sim->print_lock, NULL);
	sim->dongles = init_dongles(sim->number_of_coders);
	if (!sim->dongles)
		return (1);
	sim->coders = init_coders(sim->number_of_coders, sim->dongles, sim);
	if (!sim->coders)
	{
		destroy_dongles(sim->dongles, sim->number_of_coders);
		return (1);
	}
	return (0);
}

int	main(int ac, char *av[])
{
	t_sim	sim;

	if (check_args(ac, av))
		return (1);
	fill_sim(&sim, av);
	if (init_sim(&sim))
		return (1);
	launch_coders(&sim);
	cleanup(&sim);
	return (0);
}

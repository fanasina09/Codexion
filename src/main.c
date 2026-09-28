/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:46:50 by faharila          #+#    #+#             */
/*   Updated: 2026/09/28 11:40:57 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

int	check_args(int ac, char *av[])
{
	int	i;

	if (ac != 9)
	{
		fprintf(stderr, "Error: wrong number of arguments\n");
		return (1);
	}
	i = 1;
	while (i < 8)
	{
		if (!is_valid_number(av[i]))
		{
			fprintf(stderr, "Error: invalid argument\n");
			return (1);
		}
		i++;
	}
	if (strcmp(av[8], "fifo") != 0 && strcmp(av[8], "edf") != 0)
	{
		fprintf(stderr, "Error: invalid scheduler\n");
		return (1);
	}
	return (0);
}

void	fill_sim(t_sim *sim, char *av[])
{
	sim->number_of_coders = atoi(av[1]);
	sim->time_to_burnout = atoi(av[2]);
	sim->time_to_compile = atoi(av[3]);
	sim->time_to_debug = atoi(av[4]);
	sim->time_to_refactor = atoi(av[5]);
	sim->number_of_compiles_required = atoi(av[6]);
	sim->dongle_cooldown = atoi(av[7]);
	if (strcmp(av[8], "fifo") == 0)
		sim->scheduler = 0;
	else
		sim->scheduler = 1;
}

void	launch_coders(t_sim *sim)
{
	pthread_t	monitor_thread;
	int			i;

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

int	main(int ac, char *av[])
{
	t_sim	sim;

	if (check_args(ac, av))
		return (1);
	fill_sim(&sim, av);
	sim.running = 1;
	pthread_mutex_init(&sim.running_lock, NULL);
	sim.dongles = init_dongles(sim.number_of_coders);
	sim.coders = init_coders(sim.number_of_coders, sim.dongles, &sim);
	printf("Coder 1 : id=%d, left=%p, right=%p\n", sim.coders[0].id,
		(void *)sim.coders[0].left_dongle, (void *)sim.coders[0].right_dongle);
	launch_coders(&sim);
	cleanup(&sim);
	return (0);
}

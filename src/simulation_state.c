/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_state.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 10:14:57 by faharila          #+#    #+#             */
/*   Updated: 2026/10/28 10:48:43 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_running(t_sim *sim)
{
	int	value;

	pthread_mutex_lock(&sim->running_lock);
	value = sim->running;
	pthread_mutex_unlock(&sim->running_lock);
	return (value);
}

void	stop_simulation(t_sim *sim)
{
	int	i;

	pthread_mutex_lock(&sim->running_lock);
	sim->running = 0;
	pthread_mutex_unlock(&sim->running_lock);
	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].is_lock);
		pthread_cond_broadcast(&sim->dongles[i].is_cond);
		pthread_mutex_unlock(&sim->dongles[i].is_lock);
		i++;
	}
}

void	smart_sleep(t_sim *sim, long duration_ms)
{
	long	start;

	start = get_timestamp_ms();
	while (is_running(sim) && get_timestamp_ms() - start < duration_ms)
		usleep(500);
}

t_scheduler	get_scheduler(char *str)
{
	if (strcmp(str, "fifo") == 0)
		return (SCHEDULER_FIFO);
	if (strcmp(str, "edf") == 0)
		return (SCHEDULER_EDF);
	return (SCHEDULER_UNKNOWN);
}

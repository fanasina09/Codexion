/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:46:54 by faharila          #+#    #+#             */
/*   Updated: 2026/09/28 09:46:55 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

int	all_compiled_enough(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		if (sim->coders[i].nb_compliles < sim->number_of_compiles_required)
			return (0);
		i++;
	}
	return (1);
}

void	*monitor(void *arg)
{
	t_sim	*sim;
	int		i;
	long	elapsed;

	sim = (t_sim *)arg;
	while (is_running(sim))
	{
		i = 0;
		while (i < sim->number_of_coders)
		{
			elapsed = get_timestamp_ms()
				- sim->coders[i].last_compile_and_start;
			if (elapsed > sim->time_to_burnout)
			{
				printf("%ld %d burned out\n", get_timestamp_ms(),
					sim->coders[i].id);
				stop_simulation(sim);
			}
			i++;
		}
		if (all_compiled_enough(sim))
			stop_simulation(sim);
		usleep(5000);
	}
	return (NULL);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:46:54 by faharila          #+#    #+#             */
/*   Updated: 2026/10/12 14:08:52 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	all_compiled_enough(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		if (get_compiles(&sim->coders[i]) < sim->number_of_compiles_required)
			return (0);
		i++;
	}
	return (1);
}

static int	check_burnout(t_sim *sim)
{
	int		i;
	long	elapsed;

	i = 0;
	while (i < sim->number_of_coders)
	{
		if (get_compiles(&sim->coders[i]) < sim->number_of_compiles_required)
		{
			elapsed = get_timestamp_ms() - get_last_compile(&sim->coders[i]);
			if (elapsed > sim->time_to_burnout)
			{
				print_burnout(sim, sim->coders[i].id);
				return (1);
			}
		}
		i++;
	}
	return (0);
}

void	*monitor(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (is_running(sim))
	{
		if (check_burnout(sim))
			break ;
		if (all_compiled_enough(sim))
		{
			stop_simulation(sim);
			break ;
		}
		usleep(1000);
	}
	return (NULL);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stagger.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 13:40:00 by faharila          #+#    #+#             */
/*   Updated: 2026/11/08 13:40:00 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long	scaled(long step, long pos)
{
	if (step > 0 && pos > LONG_MAX / step)
		return (LONG_MAX);
	return (step * pos);
}

long	start_rank(t_coder *coder)
{
	t_sim	*sim;
	long	half;

	sim = coder->sim;
	if (sim->number_of_coders % 2 == 0 || sim->number_of_coders == 1)
	{
		if (coder->id % 2 == 0)
			return (sim->number_of_coders / 2 + coder->id / 2 - 1);
		return ((coder->id - 1) / 2);
	}
	half = sim->number_of_coders / 2;
	return ((half * (coder->id - 1)) % sim->number_of_coders);
}

long	start_delay(t_coder *coder)
{
	t_sim	*sim;
	long	half;
	long	step;

	sim = coder->sim;
	if (sim->number_of_coders % 2 == 0 || sim->number_of_coders == 1)
	{
		if (coder->id % 2 == 0)
			return (sim->time_to_compile / 2);
		return (0);
	}
	half = sim->number_of_coders / 2;
	step = sim->time_to_compile / half + sim->dongle_cooldown / half;
	return (scaled(step, (half * (coder->id - 1)) % sim->number_of_coders));
}

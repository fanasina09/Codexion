/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   waiting_queue.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 10:14:28 by faharila          #+#    #+#             */
/*   Updated: 2026/09/28 10:14:29 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

void	add_to_waiting(t_dongle *dongle, t_coder *coder)
{
	dongle->waiting[dongle->nb_waiting] = coder;
	dongle->nb_waiting++;
}

void	remove_from_waiting(t_dongle *dongle, t_coder *coder)
{
	int	i;
	int	j;

	i = 0;
	while (i < dongle->nb_waiting && dongle->waiting[i] != coder)
		i++;
	j = i;
	while (j < dongle->nb_waiting - 1)
	{
		dongle->waiting[j] = dongle->waiting[j + 1];
		j++;
	}
	dongle->nb_waiting--;
}

long	get_deadline(t_coder *coder)
{
	return (coder->last_compile_and_start + coder->sim->time_to_burnout);
}

t_coder	*get_next_coder(t_dongle *dongle, int scheduler)
{
	int		i;
	t_coder	*best;

	if (dongle->nb_waiting == 0)
		return (NULL);
	best = dongle->waiting[0];
	if (scheduler == 0)
		return (best);
	i = 1;
	while (i < dongle->nb_waiting)
	{
		if (get_deadline(dongle->waiting[i]) < get_deadline(best))
			best = dongle->waiting[i];
		i++;
	}
	return (best);
}

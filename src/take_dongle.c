/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   take_dongle.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 10:14:34 by faharila          #+#    #+#             */
/*   Updated: 2026/10/28 06:02:42 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	wait_dongle(t_dongle *dongle, t_coder *coder)
{
	if (dongle->is_taken == 0 && get_next_coder(dongle) == coder)
	{
		pthread_mutex_unlock(&dongle->is_lock);
		usleep(500);
		pthread_mutex_lock(&dongle->is_lock);
		return ;
	}
	pthread_cond_wait(&dongle->is_cond, &dongle->is_lock);
}

static int	must_wait(t_dongle *dongle, t_coder *coder)
{
	if (dongle->is_taken == 1)
		return (1);
	if (get_next_coder(dongle) != coder)
		return (1);
	return (get_timestamp_ms() - dongle->released_at
		< coder->sim->dongle_cooldown);
}

void	take_dongle(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&dongle->is_lock);
	if (!add_to_waiting(dongle, coder))
	{
		pthread_mutex_unlock(&dongle->is_lock);
		stop_simulation(coder->sim);
		return ;
	}
	while (is_running(coder->sim) && must_wait(dongle, coder))
		wait_dongle(dongle, coder);
	remove_from_waiting(dongle, coder);
	if (!is_running(coder->sim))
	{
		pthread_mutex_unlock(&dongle->is_lock);
		return ;
	}
	dongle->is_taken = 1;
	pthread_mutex_unlock(&dongle->is_lock);
	print_status(coder, "has taken a dongle");
}

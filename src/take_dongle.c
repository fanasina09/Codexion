/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   take_dongle.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 10:14:34 by faharila          #+#    #+#             */
/*   Updated: 2026/09/28 10:14:53 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	take_dongle(t_dongle *dongle, t_coder *coder)
{
	struct timespec	ts;

	pthread_mutex_lock(&dongle->is_lock);
	add_to_waiting(dongle, coder);
	while (is_running(coder->sim) && (dongle->is_taken == 1
			|| get_next_coder(dongle, coder->sim->scheduler) != coder
			|| get_timestamp_ms()
			- dongle->released_at < coder->sim->dongle_cooldown))
	{
		ts.tv_sec = time(NULL) + 1;
		ts.tv_nsec = 0;
		pthread_cond_timedwait(&dongle->is_cond, &dongle->is_lock, &ts);
	}
	remove_from_waiting(dongle, coder);
	if (!is_running(coder->sim))
	{
		pthread_mutex_unlock(&dongle->is_lock);
		return ;
	}
	dongle->is_taken = 1;
	pthread_mutex_unlock(&dongle->is_lock);
	printf("%ld %d has taken a dongle\n", get_timestamp_ms(), coder->id);
}

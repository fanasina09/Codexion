/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   take_dongles.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 10:14:34 by faharila          #+#    #+#             */
/*   Updated: 2026/11/05 08:40:00 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_available(t_coder *coder)
{
	return (!coder->left_dongle->is_taken && !coder->right_dongle->is_taken
		&& heap_peek(&coder->left_dongle->waiting) == coder
		&& heap_peek(&coder->right_dongle->waiting) == coder);
}

static int	is_time(t_coder *coder)
{
	long	cooldown;

	cooldown = coder->sim->dongle_cooldown;
	return (ms_since(coder->left_dongle->released_at) >= cooldown
		&& ms_since(coder->right_dongle->released_at) >= cooldown
		&& get_elapsed_ms(coder->sim) >= coder->start_ms);
}

static void	wait_turn(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	if (is_available(coder))
	{
		pthread_mutex_unlock(&sim->table_lock);
		usleep(500);
		pthread_mutex_lock(&sim->table_lock);
		return ;
	}
	pthread_cond_wait(&sim->table_cond, &sim->table_lock);
}

int	take_dongles(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	if (coder->left_dongle == coder->right_dongle)
		return (lone_coder(coder));
	pthread_mutex_lock(&sim->table_lock);
	if (!enter_queues(coder))
	{
		pthread_mutex_unlock(&sim->table_lock);
		stop_simulation(sim);
		return (0);
	}
	while (is_running(sim) && !(is_available(coder) && is_time(coder)))
		wait_turn(coder);
	leave_queues(coder);
	if (!is_running(sim))
	{
		pthread_mutex_unlock(&sim->table_lock);
		return (0);
	}
	coder->left_dongle->is_taken = 1;
	coder->right_dongle->is_taken = 1;
	pthread_mutex_unlock(&sim->table_lock);
	return (1);
}

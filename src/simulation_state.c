/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_state.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 10:14:57 by faharila          #+#    #+#             */
/*   Updated: 2026/09/28 10:14:58 by faharila         ###   ########.fr       */
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
	pthread_mutex_lock(&sim->running_lock);
	sim->running = 0;
	pthread_mutex_unlock(&sim->running_lock);
}

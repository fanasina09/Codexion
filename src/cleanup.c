/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:45:37 by faharila          #+#    #+#             */
/*   Updated: 2026/10/12 09:40:44 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	cleanup(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_mutex_destroy(&sim->coders[i].data_lock);
		i++;
	}
	destroy_dongles(sim->dongles, sim->number_of_coders);
	free(sim->coders);
	pthread_mutex_destroy(&sim->running_lock);
	pthread_mutex_destroy(&sim->print_lock);
}

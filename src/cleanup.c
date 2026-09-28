/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:45:37 by faharila          #+#    #+#             */
/*   Updated: 2026/09/28 09:45:38 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

void	cleanup(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_mutex_destroy(&sim->dongles[i].is_lock);
		pthread_cond_destroy(&sim->dongles[i].is_cond);
		i++;
	}
	free(sim->dongles);
	free(sim->coders);
	pthread_mutex_destroy(&sim->running_lock);
}

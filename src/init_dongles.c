/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_dongles.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:46:44 by faharila          #+#    #+#             */
/*   Updated: 2026/09/28 09:46:45 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

t_dongle	*init_dongles(int number_of_coders)
{
	t_dongle	*dongles;
	int			i;

	dongles = malloc(sizeof(t_dongle) * number_of_coders);
	if (!dongles)
		return (NULL);
	i = 0;
	while (i < number_of_coders)
	{
		dongles[i].is_taken = 0;
		dongles[i].released_at = 0;
		dongles[i].nb_waiting = 0;
		pthread_mutex_init(&dongles[i].is_lock, NULL);
		pthread_cond_init(&dongles[i].is_cond, NULL);
		i++;
	}
	return (dongles);
}

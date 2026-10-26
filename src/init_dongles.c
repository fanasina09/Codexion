/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_dongles.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:46:44 by faharila          #+#    #+#             */
/*   Updated: 2026/10/12 09:52:35 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_one(t_dongle *dongle)
{
	dongle->is_taken = 0;
	dongle->released_at = 0;
	dongle->next_seq = 0;
	if (!heap_init(&dongle->waiting, 2))
		return (0);
	pthread_mutex_init(&dongle->is_lock, NULL);
	pthread_cond_init(&dongle->is_cond, NULL);
	return (1);
}

void	destroy_dongles(t_dongle *dongles, int count)
{
	while (count > 0)
	{
		count--;
		pthread_mutex_destroy(&dongles[count].is_lock);
		pthread_cond_destroy(&dongles[count].is_cond);
		heap_destroy(&dongles[count].waiting);
	}
	free(dongles);
}

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
		if (!init_one(&dongles[i]))
		{
			destroy_dongles(dongles, i);
			return (NULL);
		}
		i++;
	}
	return (dongles);
}

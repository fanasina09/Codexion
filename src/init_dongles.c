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
	return (heap_init(&dongle->waiting, 2));
}

void	destroy_dongles(t_dongle *dongles, int count)
{
	while (count > 0)
	{
		count--;
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

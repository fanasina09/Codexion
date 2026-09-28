/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_coders.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:46:30 by faharila          #+#    #+#             */
/*   Updated: 2026/09/28 09:46:32 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_coder	*init_coders(int number_of_coders, t_dongle *dongles, t_sim *sim)
{
	t_coder	*coders;
	int		i;

	coders = malloc(sizeof(t_coder) * number_of_coders);
	if (!coders)
		return (NULL);
	i = 0;
	while (i < number_of_coders)
	{
		coders[i].id = i + 1;
		coders[i].nb_compliles = 0;
		coders[i].left_dongle = &dongles[i];
		coders[i].right_dongle = &dongles[(i + 1) % number_of_coders];
		coders[i].last_compile_and_start = get_timestamp_ms();
		coders[i].sim = sim;
		i++;
	}
	return (coders);
}

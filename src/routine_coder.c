/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine_coder.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 10:15:03 by faharila          #+#    #+#             */
/*   Updated: 2026/09/28 10:15:22 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	*routine_coder(void *ag)
{
	t_coder	*coder;

	coder = (t_coder *)ag;
	while (is_running(coder->sim))
	{
		if (coder->id == coder->sim->number_of_coders)
		{
			take_dongle(coder->right_dongle, coder);
			take_dongle(coder->left_dongle, coder);
		}
		else
		{
			take_dongle(coder->left_dongle, coder);
			take_dongle(coder->right_dongle, coder);
		}
		if (!is_running(coder->sim))
			break ;
		coder->last_compile_and_start = get_timestamp_ms();
		printf("%ld %d is compiling\n", get_timestamp_ms(), coder->id);
		usleep(coder->sim->time_to_compile * 1000);
		release_dongle(coder->left_dongle);
		release_dongle(coder->right_dongle);
		coder->nb_compliles++;
		if (!is_running(coder->sim))
			break ;
		printf("%ld %d is debugging\n", get_timestamp_ms(), coder->id);
		usleep(coder->sim->time_to_debug * 1000);
		if (!is_running(coder->sim))
			break ;
		printf("%ld %d is refactoring\n", get_timestamp_ms(), coder->id);
		usleep(coder->sim->time_to_refactor * 1000);
	}
	return (NULL);
}

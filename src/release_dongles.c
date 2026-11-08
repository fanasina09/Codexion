/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   release_dongles.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:48:27 by faharila          #+#    #+#             */
/*   Updated: 2026/11/05 08:40:00 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	release_dongles(t_coder *coder)
{
	t_sim	*sim;
	long	now;

	sim = coder->sim;
	pthread_mutex_lock(&sim->table_lock);
	now = get_timestamp_us();
	coder->left_dongle->is_taken = 0;
	coder->left_dongle->released_at = now;
	coder->right_dongle->is_taken = 0;
	coder->right_dongle->released_at = now;
	pthread_cond_broadcast(&sim->table_cond);
	pthread_mutex_unlock(&sim->table_lock);
}

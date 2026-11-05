/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lone_coder.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 08:40:00 by faharila          #+#    #+#             */
/*   Updated: 2026/11/05 08:40:00 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	lone_coder(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	print_status(coder, "has taken a dongle");
	pthread_mutex_lock(&sim->table_lock);
	while (is_running(sim))
		pthread_cond_wait(&sim->table_cond, &sim->table_lock);
	pthread_mutex_unlock(&sim->table_lock);
	return (0);
}

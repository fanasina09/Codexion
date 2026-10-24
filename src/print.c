/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:45:37 by faharila          #+#    #+#             */
/*   Updated: 2026/10/12 09:40:44 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	print_status(t_coder *coder, char *msg)
{
	pthread_mutex_lock(&coder->sim->print_lock);
	if (is_running(coder->sim))
		printf("%ld %d %s\n", get_elapsed_ms(coder->sim), coder->id, msg);
	pthread_mutex_unlock(&coder->sim->print_lock);
}

void	print_burnout(t_sim *sim, int id)
{
	pthread_mutex_lock(&sim->print_lock);
	if (is_running(sim))
	{
		printf("%ld %d burned out\n", get_elapsed_ms(sim), id);
		stop_simulation(sim);
	}
	pthread_mutex_unlock(&sim->print_lock);
}

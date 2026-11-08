/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine_coder.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 10:15:03 by faharila          #+#    #+#             */
/*   Updated: 2026/11/04 10:23:49 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	compile_step(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	if (!take_dongles(coder))
		return (0);
	if (!set_last_compile(coder))
	{
		print_burnout(sim, coder->id);
		release_dongles(coder);
		return (0);
	}
	print_dongles(coder);
	print_status(coder, "is compiling");
	smart_sleep(sim, sim->time_to_compile);
	release_dongles(coder);
	add_compile(coder);
	return (is_running(sim));
}

void	*routine_coder(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;

	coder = (t_coder *)arg;
	sim = coder->sim;
	while (is_running(sim)
		&& get_compiles(coder) < sim->number_of_compiles_required)
	{
		if (!compile_step(coder))
			break ;
		print_status(coder, "is debugging");
		smart_sleep(sim, sim->time_to_debug);
		if (!is_running(sim))
			break ;
		print_status(coder, "is refactoring");
		smart_sleep(sim, sim->time_to_refactor);
	}
	return (NULL);
}

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

static void	take_both(t_coder *coder)
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
}

static int	compile_step(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	take_both(coder);
	if (!is_running(sim))
		return (0);
	set_last_compile(coder);
	print_status(coder, "is compiling");
	smart_sleep(sim, sim->time_to_compile);
	release_dongle(coder->left_dongle);
	release_dongle(coder->right_dongle);
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

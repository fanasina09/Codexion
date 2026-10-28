/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   waiting_queue.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 10:14:28 by faharila          #+#    #+#             */
/*   Updated: 2026/10/28 09:04:54 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	get_deadline(t_coder *coder)
{
	long	last;

	last = get_last_compile(coder);
	if (coder->sim->time_to_burnout > LONG_MAX - last)
		return (LONG_MAX);
	return (last + coder->sim->time_to_burnout);
}

static long	get_key(t_coder *coder, long seq)
{
	if (coder->sim->scheduler == SCHEDULER_EDF)
		return (get_deadline(coder));
	return (seq);
}

int	add_to_waiting(t_dongle *dongle, t_coder *coder)
{
	t_heap_node	node;

	node.coder = coder;
	node.seq = dongle->next_seq;
	dongle->next_seq++;
	node.key = get_key(coder, node.seq);
	return (heap_push(&dongle->waiting, node));
}

void	remove_from_waiting(t_dongle *dongle, t_coder *coder)
{
	heap_remove(&dongle->waiting, coder);
}

t_coder	*get_next_coder(t_dongle *dongle)
{
	return (heap_peek(&dongle->waiting));
}

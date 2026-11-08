/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   waiting_queue.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 10:14:28 by faharila          #+#    #+#             */
/*   Updated: 2026/11/05 08:40:00 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	get_deadline(t_coder *coder)
{
	long	last;

	last = get_last_compile(coder);
	if (coder->sim->time_to_burnout > (LONG_MAX - last) / 1000L)
		return (LONG_MAX);
	return (last + coder->sim->time_to_burnout * 1000L);
}

static long	get_key(t_coder *coder, long seq)
{
	if (coder->sim->scheduler == SCHEDULER_EDF)
		return (get_deadline(coder));
	return (seq);
}

static int	push_in(t_dongle *dongle, t_coder *coder, long seq)
{
	t_heap_node	node;

	node.coder = coder;
	node.seq = seq;
	node.key = get_key(coder, seq);
	return (heap_push(&dongle->waiting, node));
}

int	enter_queues(t_coder *coder)
{
	long	seq;

	seq = coder->first_seq;
	if (seq < 0)
	{
		seq = coder->sim->next_seq;
		coder->sim->next_seq++;
	}
	coder->first_seq = -1;
	if (!push_in(coder->left_dongle, coder, seq))
		return (0);
	if (!push_in(coder->right_dongle, coder, seq))
	{
		heap_remove(&coder->left_dongle->waiting, coder);
		return (0);
	}
	return (1);
}

void	leave_queues(t_coder *coder)
{
	heap_remove(&coder->left_dongle->waiting, coder);
	heap_remove(&coder->right_dongle->waiting, coder);
}

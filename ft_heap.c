/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_heap.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nfitahin <nfitahin@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 06:46:38 by nfitahin          #+#    #+#             */
/*   Updated: 2026/10/24 12:12:52 by nfitahin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	has_higher_priority(t_heap_entry a, t_heap_entry b)
{
	if (a.priority != b.priority)
		return (a.priority < b.priority);
	if (a.seq != b.seq)
		return (a.seq < b.seq);
	return (a.coder_id < b.coder_id);
}

int	ft_heap_push(t_heap *heap, t_coder *coder)
{
	int				i;
	int				p;
	t_heap_entry	tmp;

	if (!heap || !heap->data || heap->size >= heap->capacity)
		return (0);
	i = heap->size++;
	if (coder->env->args->scheduler == SCHEDULER_EDF)
		heap->data[i].priority = coder->time_burnout
			+ coder->env->args->burnout_time;
	else if (coder->env->args->scheduler == SCHEDULER_FIFO)
		heap->data[i].priority = -(long)heap->next_seq;
	else
		heap->data[i].priority = heap->next_seq;
	heap->data[i].seq = heap->next_seq++;
	heap->data[i].coder_id = coder->id;
	while (i > 0)
	{
		p = (i - 1) / 2;
		if (!has_higher_priority(heap->data[i], heap->data[p]))
			break ;
		tmp = heap->data[i];
		heap->data[i] = heap->data[p];
		heap->data[p] = tmp;
		i = p;
	}
	return (1);
}

static int	get_best_index(t_heap *heap, int i)
{
	int	best;
	int	left;
	int	right;

	best = i;
	left = 2 * i + 1;
	right = 2 * i + 2;
	if (left < heap->size && has_higher_priority(heap->data[left],
			heap->data[best]))
		best = left;
	if (right < heap->size && has_higher_priority(heap->data[right],
			heap->data[best]))
		best = right;
	return (best);
}

int	ft_heap_pop(t_heap *heap, t_heap_entry *out)
{
	int				i;
	int				best;
	t_heap_entry	tmp;

	if (!heap || !heap->data || !out || heap->size == 0)
		return (0);
	*out = heap->data[0];
	heap->data[0] = heap->data[--heap->size];
	i = 0;
	while (1)
	{
		best = get_best_index(heap, i);
		if (best == i)
			break ;
		tmp = heap->data[i];
		heap->data[i] = heap->data[best];
		heap->data[best] = tmp;
		i = best;
	}
	return (1);
}

int	ft_heap_peek(t_heap *heap, t_heap_entry *out)
{
	if (!heap || !heap->data || !out || heap->size == 0)
		return (0);
	*out = heap->data[0];
	return (1);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 02:45:00 by faharila          #+#    #+#             */
/*   Updated: 2026/10/26 02:45:00 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "heap.h"
#include <stdlib.h>

static int	heap_less(t_heap_node *a, t_heap_node *b)
{
	if (a->key != b->key)
		return (a->key < b->key);
	return (a->seq < b->seq);
}

static void	heap_swap(t_heap_node *a, t_heap_node *b)
{
	t_heap_node	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	heap_sift_up(t_heap *heap, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!heap_less(&heap->nodes[i], &heap->nodes[parent]))
			break ;
		heap_swap(&heap->nodes[i], &heap->nodes[parent]);
		i = parent;
	}
}

void	heap_sift_down(t_heap *heap, int i)
{
	int	best;
	int	child;

	while (1)
	{
		best = i;
		child = 2 * i + 1;
		if (child < heap->size
			&& heap_less(&heap->nodes[child], &heap->nodes[best]))
			best = child;
		child++;
		if (child < heap->size
			&& heap_less(&heap->nodes[child], &heap->nodes[best]))
			best = child;
		if (best == i)
			return ;
		heap_swap(&heap->nodes[i], &heap->nodes[best]);
		i = best;
	}
}

int	heap_init(t_heap *heap, int capacity)
{
	heap->size = 0;
	heap->capacity = capacity;
	heap->nodes = malloc(sizeof(t_heap_node) * capacity);
	return (heap->nodes != NULL);
}

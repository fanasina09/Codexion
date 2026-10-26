/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_ops.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 02:45:00 by faharila          #+#    #+#             */
/*   Updated: 2026/10/26 02:45:00 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "heap.h"
#include <stdlib.h>

void	heap_destroy(t_heap *heap)
{
	free(heap->nodes);
	heap->nodes = NULL;
	heap->size = 0;
	heap->capacity = 0;
}

int	heap_push(t_heap *heap, t_heap_node node)
{
	if (heap->size >= heap->capacity)
		return (0);
	heap->nodes[heap->size] = node;
	heap->size++;
	heap_sift_up(heap, heap->size - 1);
	return (1);
}

struct s_coder	*heap_peek(t_heap *heap)
{
	if (heap->size == 0)
		return (NULL);
	return (heap->nodes[0].coder);
}

int	heap_remove(t_heap *heap, struct s_coder *coder)
{
	int	i;

	i = 0;
	while (i < heap->size && heap->nodes[i].coder != coder)
		i++;
	if (i == heap->size)
		return (0);
	heap->size--;
	if (i == heap->size)
		return (1);
	heap->nodes[i] = heap->nodes[heap->size];
	heap_sift_up(heap, i);
	heap_sift_down(heap, i);
	return (1);
}

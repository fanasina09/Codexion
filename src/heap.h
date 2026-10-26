/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 02:45:00 by faharila          #+#    #+#             */
/*   Updated: 2026/10/26 02:45:00 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEAP_H
# define HEAP_H

struct	s_coder;

typedef struct s_heap_node
{
	long			key;
	long			seq;
	struct s_coder	*coder;
}					t_heap_node;

typedef struct s_heap
{
	t_heap_node		*nodes;
	int				size;
	int				capacity;
}					t_heap;

int					heap_init(t_heap *heap, int capacity);
void				heap_destroy(t_heap *heap);
int					heap_push(t_heap *heap, t_heap_node node);
struct s_coder		*heap_peek(t_heap *heap);
int					heap_remove(t_heap *heap, struct s_coder *coder);
void				heap_sift_up(t_heap *heap, int i);
void				heap_sift_down(t_heap *heap, int i);

#endif

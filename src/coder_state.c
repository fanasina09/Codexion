/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_state.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:45:37 by faharila          #+#    #+#             */
/*   Updated: 2026/10/26 07:16:21 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	get_last_compile(t_coder *coder)
{
	long	value;

	pthread_mutex_lock(&coder->data_lock);
	value = coder->last_compile_and_start;
	pthread_mutex_unlock(&coder->data_lock);
	return (value);
}

void	set_last_compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->data_lock);
	coder->last_compile_and_start = get_timestamp_ms();
	pthread_mutex_unlock(&coder->data_lock);
}

int	get_compiles(t_coder *coder)
{
	int	value;

	pthread_mutex_lock(&coder->data_lock);
	value = coder->nb_compiles;
	pthread_mutex_unlock(&coder->data_lock);
	return (value);
}

void	add_compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->data_lock);
	coder->nb_compiles++;
	pthread_mutex_unlock(&coder->data_lock);
}

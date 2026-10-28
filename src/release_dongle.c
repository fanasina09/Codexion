/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   release_dongle.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:48:27 by faharila          #+#    #+#             */
/*   Updated: 2026/10/28 00:23:57 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	release_dongle(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->is_lock);
	dongle->is_taken = 0;
	dongle->released_at = get_timestamp_ms();
	pthread_cond_broadcast(&dongle->is_cond);
	pthread_mutex_unlock(&dongle->is_lock);
}

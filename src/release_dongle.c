/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   release_dongle.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:48:27 by faharila          #+#    #+#             */
/*   Updated: 2026/09/28 09:48:28 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

void	release_dongle(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->is_lock);
	dongle->is_taken = 0;
	dongle->released_at = get_timestamp_ms();
	pthread_cond_signal(&dongle->is_cond);
	pthread_mutex_unlock(&dongle->is_lock);
}

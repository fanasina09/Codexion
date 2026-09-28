/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:48:10 by faharila          #+#    #+#             */
/*   Updated: 2026/09/28 09:48:11 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

int	is_valid_number(char *str)
{
	int	i;
	int	has_no_zero;

	i = 0;
	has_no_zero = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		if (str[i] != '0')
			has_no_zero = 1;
		i++;
	}
	return (has_no_zero);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:48:10 by faharila          #+#    #+#             */
/*   Updated: 2026/10/20 12:19:52 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_valid_number(char *str)
{
	int		i;
	long	nb;

	i = 0;
	nb = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		if (nb > (LONG_MAX - (str[i] - '0')) / 10)
			return (0);
		nb = nb * 10 + (str[i] - '0');
		i++;
	}
	return (nb > 0);
}
long	ft_atol(char *str)
{
	long long	nb;
	int		i;
	int		sign;

	nb = 0;
	i = 0;
	sign = 1;
	while(str[i] == 32 || (str[i] >= 9 && str[i] <= 13))
		i ++;
	if (str[i] == '+' || str[i] == '-')
	{
		if (str[i] == '-')
			sign *= -1;
		i ++;
	}
	while (str[i])
	{
		nb = nb * 10 + (str[i] - '0');
		i++;
	}
	nb = nb * sign;
	return (nb);
}
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: faharila <faharila@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/28 09:48:10 by faharila          #+#    #+#             */
/*   Updated: 2026/10/12 09:53:03 by faharila         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_valid_number(char *str, int allow_zero)
{
	int		i;
	long	nb;

	i = 0;
	nb = 0;
	if (!str[0])
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		if (nb > (LONG_MAX - (str[i] - '0')) / 10)
			return (0);
		nb = nb * 10 + (str[i] - '0');
		i++;
	}
	return (nb > 0 || allow_zero);
}

long	ft_atol(char *str)
{
	long	nb;
	int		i;

	nb = 0;
	i = 0;
	while (str[i])
	{
		nb = nb * 10 + (str[i] - '0');
		i++;
	}
	return (nb);
}

static int	check_numbers(char *av[])
{
	int	i;

	i = 1;
	while (i < 8)
	{
		if (!is_valid_number(av[i], i == 7))
		{
			fprintf(stderr, "Error: invalid argument\n");
			return (1);
		}
		i++;
	}
	if (ft_atol(av[1]) > INT_MAX)
	{
		fprintf(stderr, "Error: invalid number of coders\n");
		return (1);
	}
	return (0);
}

int	check_args(int ac, char *av[])
{
	if (ac != 9)
	{
		fprintf(stderr, "Error: wrong number of arguments\n");
		return (1);
	}
	if (check_numbers(av))
		return (1);
	if (strcmp(av[8], "fifo") != 0 && strcmp(av[8], "edf") != 0)
	{
		fprintf(stderr, "Error: invalid scheduler\n");
		return (1);
	}
	return (0);
}

void	fill_sim(t_sim *sim, char *av[])
{
	sim->number_of_coders = (int)ft_atol(av[1]);
	sim->time_to_burnout = ft_atol(av[2]);
	sim->time_to_compile = ft_atol(av[3]);
	sim->time_to_debug = ft_atol(av[4]);
	sim->time_to_refactor = ft_atol(av[5]);
	sim->number_of_compiles_required = ft_atol(av[6]);
	sim->dongle_cooldown = ft_atol(av[7]);
	if (strcmp(av[8], "fifo") == 0)
		sim->scheduler = 0;
	else
		sim->scheduler = 1;
}

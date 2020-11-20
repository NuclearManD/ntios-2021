/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr_base.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/23 17:40:28 by dbrophy           #+#    #+#             */
/*   Updated: 2019/10/23 17:40:28 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libneon.h"
#include "unistd.h"

static void		recursive_putnbr_base(int nb, int baselen, char *base)
{
	if (nb < baselen)
		write(1, &(base[nb]), 1);
	else
	{
		recursive_putnbr_base(nb / baselen, baselen, base);
		write(1, &(base[nb % baselen]), 1);
	}
}

void			putnbr_base(int nb, char *base)
{
	int len;
	int i;

	len = -1;
	while (base[++len])
	{
		i = len + 1;
		while (base[i])
			if (base[i++] == base[len] || base[len] == 43 || base[len] == 45)
				return ;
	}
	if (len <= 1)
		return ;
	if (nb < 0)
	{
		write(1, "-", 1);
		if (-nb >= len)
			recursive_putnbr_base(-(nb / len), len, base);
		write(1, &(base[(len - (nb % len)) % len]), 1);
	}
	else
		recursive_putnbr_base(nb, len, base);
}

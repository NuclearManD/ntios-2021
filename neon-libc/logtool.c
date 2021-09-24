/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logtool.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/03/03 16:40:54 by dbrophy           #+#    #+#             */
/*   Updated: 2020/03/03 16:40:54 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdint.h>
#include "printf/ft_printf.h"

int				nchar_abs(intmax_t val, int base, t_fmt_d *data)
{
	uintmax_t	log;
	int			len;

	len = 0;
	if (val == 0 && data->precision != 0)
		len++;
	log = (uintmax_t)val;
	if (val < 0 && data->cnvrt != 'u' && base == 10)
		log = (UINTMAX_MAX - log) + 1;
	while (log != 0)
	{
		log = log / base;
		len++;
	}
	return (len);
}

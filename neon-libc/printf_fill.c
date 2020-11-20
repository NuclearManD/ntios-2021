/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   printf_fill.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/27 13:43:21 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/27 13:43:21 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"
#include "unistd.h"
#include <stdint.h>

int			printf_put_many(int fd, int sz, char c)
{
	int res;

	res = 0;
	while (sz-- > 0)
		res += write(fd, &c, 1);
	return (res);
}

int			printf_num_fill(int *dlen, t_fmt_d *f, intmax_t num, int b)
{
	int size_out;
	int tmp;

	tmp = printf_fill(f->fd, *dlen, f);
	size_out = 0;
	if (b == 10 && f->cnvrt != 'u')
	{
		if (num < 0)
			size_out = write(f->fd, "-", 1);
		else if (f->flags & FLAG_PLUS)
			size_out = write(f->fd, "+", 1);
		else if (f->flags & FLAG_SPCE)
			size_out = write(f->fd, " ", 1);
	}
	if ((num != 0 && f->cnvrt == 'x' && (f->flags & FLAG_POUND))
		|| f->cnvrt == 'p')
		size_out += write(f->fd, "0x", 2);
	else if (num != 0 && f->cnvrt == 'X' && (f->flags & FLAG_POUND))
		size_out += write(f->fd, "0X", 2);
	else if (f->cnvrt == 'o' && (f->flags & FLAG_POUND))
		if (num != 0 || f->precision == 0)
			size_out += write(f->fd, "0", 1);
	*dlen -= size_out;
	return (size_out + tmp);
}

int			printf_fill(int fd, int dlen, t_fmt_d *f)
{
	int size;

	size = 0;
	if ((f->flags & (FLAG_ZERO | FLAG_MINUS)) == FLAG_ZERO && f->precision <= 0)
		;
	else if (f->min_width > 0)
		size += printf_put_many(fd, f->min_width - size - dlen, ' ');
	return (size);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   printf_nbrs.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/26 17:29:35 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/26 17:29:35 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"
#include <unistd.h>
#include <stdarg.h>
#include <stdint.h>

static int	rec_pn_base(int fd, uintmax_t nb, char base, char caps)
{
	int i;
	int d;

	i = 0;
	if (nb < (unsigned char)base)
		d = nb;
	else
	{
		i = rec_pn_base(fd, nb / base, base, caps);
		d = nb % base;
	}
	if (caps)
		return (i + write(fd, &("0123456789ABCDEF"[d]), 1));
	return (i + write(fd, &("0123456789abcdef"[d]), 1));
}

static int	putnbr_base(intmax_t n, char base, char caps, t_fmt_d *f)
{
	int			i;
	int			cnt;

	cnt = nchar_abs(n, base, f);
	i = printf_put_many(f->fd, f->dlen - cnt, '0');
	if (n < 0 && base == 10 && f->cnvrt != 'u')
	{
		if (n <= -base)
			i += rec_pn_base(f->fd, -(n / base), base, caps);
		else
			i += 0;
		i += rec_pn_base(f->fd, -(n % base), base, caps);
	}
	else if (cnt > 0)
		i += rec_pn_base(f->fd, n, base, caps);
	return (i);
}

/*
** Returns the expected length of the number, including all zeros,
** prefixes, and signs.
*/

static int	get_num_len(intmax_t num, char base, t_fmt_d *data)
{
	int			len;

	if (data->precision == 0 && num == 0 && data->cnvrt != 'p')
		if ((data->flags & FLAG_POUND) == 0)
			return (0);
	len = nchar_abs(num, base, data);
	if ((data->flags & FLAG_POUND) && data->cnvrt == 'o' && num != 0)
		len++;
	if (len < data->precision)
		len = data->precision;
	if (((num < 0 && base == 10) || (data->flags & 24)) && data->cnvrt != 'u')
		len++;
	if (data->flags & FLAG_POUND || data->cnvrt == 'p')
		if (((data->cnvrt | 32) == 'x' && num != 0) || data->cnvrt == 'p')
			len += 2;
	if (len < data->min_width && (data->flags & FLAG_ZERO))
		len = data->min_width;
	return (len);
}

intmax_t	pullnum(int type, va_list args, int is_unsigned)
{
	if (type == MOD_UNSPECIFIED && is_unsigned)
		return (va_arg(args, unsigned int));
	else if (type == MOD_UNSPECIFIED && !is_unsigned)
		return ((intmax_t)va_arg(args, int));
	else if (type == MOD_CHAR && is_unsigned)
		return (va_arg(args, unsigned int) & 255);
	else if (type == MOD_CHAR && !is_unsigned)
		return ((char)(va_arg(args, unsigned int) & 255));
	else if (type == MOD_SHORT && is_unsigned)
		return (va_arg(args, unsigned int) & 65535);
	else if (type == MOD_SHORT && !is_unsigned)
		return ((short)(va_arg(args, unsigned int) & 65535));
	else if (type == MOD_LONG && is_unsigned)
		return (va_arg(args, unsigned long));
	else if (type == MOD_LONG && !is_unsigned)
		return (va_arg(args, long));
	else if (type == MOD_LONG_LONG && is_unsigned)
		return (va_arg(args, unsigned long long));
	else if (type == MOD_LONG_LONG && !is_unsigned)
		return (va_arg(args, long long));
	else if (type == MOD_INTMAX_T)
		return (va_arg(args, intmax_t));
	else
		return (va_arg(args, size_t));
}

int			printf_handle_number(va_list args, t_fmt_d *data)
{
	int					len;
	int					i;
	intmax_t			num;
	char				base;

	if (data->cnvrt == 'd' || data->cnvrt == 'i' || data->cnvrt == 'u')
		base = 10;
	if (data->cnvrt == 'o')
		base = 8;
	if (data->cnvrt == 'x' || data->cnvrt == 'X' || data->cnvrt == 'p')
		base = 16;
	if (data->precision != -1 && (data->flags & FLAG_ZERO))
		data->flags ^= FLAG_ZERO;
	if (base != 10 || data->cnvrt == 'u')
		data->flags &= UNSIGNED_FLAG_MASK;
	num = pullnum(data->type, args, base != 10 || data->cnvrt == 'u');
	i = get_num_len(num, base, data);
	len = printf_num_fill(&i, data, num, base);
	data->dlen = i;
	if (data->precision != 0 || num != 0 || data->cnvrt == 'p')
		len += putnbr_base(num, base, data->cnvrt == 'X', data);
	return (len + printf_put_many(data->fd, -data->min_width - len, ' '));
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   printf_chrs.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/26 17:30:38 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/26 17:30:38 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"
#include <unistd.h>
#include <stdarg.h>
#include <wchar.h>

int			printf_handle_char(int fd, va_list args, t_fmt_d *data)
{
	char	c;
	wint_t	wc;
	int		len;

	if (data->cnvrt == 'c')
	{
		len = printf_fill(fd, 1, data);
		c = (char)va_arg(args, int);
		len += write(fd, &c, 1);
	}
	else if (data->cnvrt == 'C')
	{
		len = printf_fill(fd, sizeof(wint_t), data);
		wc = va_arg(args, wint_t);
		len += write(fd, &wc, sizeof(wint_t));
	}
	else
	{
		len = printf_fill(fd, 1, data);
		len += write(fd, &(data->cnvrt), 1);
	}
	return (len + printf_put_many(fd, -data->min_width - 1, ' '));
}

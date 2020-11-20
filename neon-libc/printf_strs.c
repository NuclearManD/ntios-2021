/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   printf_strs.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/26 15:06:47 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/26 15:06:47 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"
#include <unistd.h>
#include <stdarg.h>
#include <wchar.h>

static int	wstr_handle(int fd, va_list args, t_fmt_d *data)
{
	wchar_t		*wstr;
	int			len;
	int			printed;

	wstr = va_arg(args, wchar_t*);
	if (wstr == NULL)
		wstr = L"(null)";
	len = 0;
	while (wstr[len])
		len++;
	if (data->precision >= 0 && data->precision < len)
		len = data->precision;
	printed = printf_fill(fd, len, data);
	printed += write(fd, wstr, len * sizeof(wchar_t));
	printed += printf_put_many(fd, -data->min_width - len, ' ');
	return (printed);
}

int			printf_handle_string(int fd, va_list args, t_fmt_d *data)
{
	const char	*str;
	int			len;
	int			printed;

	if (data->cnvrt == 'S' || data->type == MOD_LONG)
		return (wstr_handle(fd, args, data));
	str = va_arg(args, const char*);
	if (str == NULL)
		str = "(null)";
	len = 0;
	while (str[len])
		len++;
	if (data->precision >= 0 && data->precision < len)
		len = data->precision;
	printed = printf_fill(fd, len, data);
	printed += write(fd, str, len);
	printed += printf_put_many(fd, -data->min_width - len, ' ');
	return (printed);
}

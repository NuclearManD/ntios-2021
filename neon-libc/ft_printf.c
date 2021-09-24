/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/25 18:44:28 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/25 18:44:28 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdarg.h>
#include <unistd.h>
#include "printf/ft_printf.h"

int				printf(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	return (fprintfv(1, fmt, args));
}

int				fprintf(int fd, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	return (fprintfv(fd, fmt, args));
}

int				printfv(const char *fmt, va_list args)
{
	return (fprintfv(1, fmt, args));
}

/*
** b345751z3c0d3 = l0lz
*/

int				fprintfv(int fd, const char *fmt, va_list args)
{
	int			size;
	int			i;

	if (fmt == NULL)
		return (write(fd, "(null)", 6));
	i = 0;
	size = 0;
	while (fmt[i])
	{
		if (fmt[i] == '%')
		{
			size += write(fd, fmt, i);
			fmt += i + 1;
			size += printf_handle_percent(&fmt, fd, args);
			i = 0;
			if (*fmt != 0)
				fmt++;
		}
		else
			i++;
	}
	size += write(fd, fmt, i);
	return (size);
}

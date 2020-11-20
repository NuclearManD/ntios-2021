/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strmapi.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/19 12:02:05 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/19 12:02:05 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libneon.h"
#include "stdlib.h"

char			*strmapi(const char *s, char (*f)(unsigned int, char))
{
	char	*out;
	int		i;

	if (s == NULL)
		return (NULL);
	i = -1;
	out = strnew(strlen(s));
	if (out == NULL)
		return (NULL);
	while (s[++i])
		out[i] = f(i, s[i]);
	return (out);
}

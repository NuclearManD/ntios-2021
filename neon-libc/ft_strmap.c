/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strmap.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/19 12:01:44 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/19 12:01:44 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "stdlib.h"
#include "libneon.h"

char			*strmap(const char *s, char (*f)(char))
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
		out[i] = f(s[i]);
	return (out);
}

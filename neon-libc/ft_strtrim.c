/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/19 12:53:44 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/19 12:53:44 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "stdlib.h"
#include "libneon.h"

char			*strtrim(const char *s)
{
	int		len;
	char	*nova;

	if (s == NULL)
		return (NULL);
	while (isspace(*s))
		s++;
	len = strlen(s);
	if (len > 0)
		while (isspace(s[len - 1]))
			len--;
	nova = (char*)malloc(len + 1);
	if (nova == NULL)
		return (NULL);
	nova[len] = 0;
	while (len--)
		nova[len] = s[len];
	return (nova);
}

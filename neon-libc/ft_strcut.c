/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strcut.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/19 14:05:49 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/19 14:05:49 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libneon.h"

/*
** ft_strcut returns a copy of the string s up to character c
**  If character c is not found then a copy of the whole string is returned
*/

char			*strcut(const char *s, char c)
{
	int i;

	i = strchri(s, c);
	if (i == -1)
		return (strdup(s));
	return (strsub(s, 0, i));
}

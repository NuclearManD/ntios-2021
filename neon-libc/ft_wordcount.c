/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_wordcount.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/23 11:31:18 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/23 11:31:18 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libneon.h"

int				wordcount(char *s)
{
	int cnt;

	cnt = 0;
	if (s == NULL)
		return (-1);
	while (*s)
	{
		cnt += (*s != 0 && !isspace(*s));
		while (*s != 0 && !isspace(*s))
			s++;
		while (isspace(*s))
			s++;
	}
	return (cnt);
}

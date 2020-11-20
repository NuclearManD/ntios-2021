/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strdup.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/21 11:55:26 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/21 11:55:26 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string.h>
#include <stdlib.h>
#include "libneon.h"

char			*strdup(const char *s)
{
	char *nova;

	nova = (char*)malloc(strlen(s) + 1);
	if (nova == NULL)
		return (NULL);
	return (strcpy(nova, s));
}

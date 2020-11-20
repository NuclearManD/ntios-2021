/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memdup.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/23 12:06:58 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/23 12:06:58 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "stdlib.h"
#include "libneon.h"

void			*memdup(void *src, size_t size)
{
	void *dst;

	dst = malloc(size);
	if (dst == NULL)
		return (NULL);
	return (memcpy(dst, src, size));
}

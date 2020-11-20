/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memalloc.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/19 11:59:05 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/19 11:59:05 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libneon.h"
#include "stdlib.h"

void			*memalloc(size_t size)
{
	void *r;

	r = malloc(size);
	if (r != NULL)
		bzero(r, size);
	return (r);
}

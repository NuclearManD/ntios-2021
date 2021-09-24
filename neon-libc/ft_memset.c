/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memset.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/21 11:55:26 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/21 11:55:26 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "string.h"

void			*memset(void *s, int c, size_t n)
{
	while(n--)
		((unsigned char*)s)[n] = (unsigned char)c;
	return (s);
}

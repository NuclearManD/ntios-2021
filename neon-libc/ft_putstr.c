/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putstr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/28 12:58:34 by dbrophy           #+#    #+#             */
/*   Updated: 2019/10/28 12:58:34 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libneon.h"
#include "unistd.h"

void			putstr(char *str)
{
	write(1, str, strlen(str));
}

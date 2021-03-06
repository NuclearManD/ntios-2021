/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_tolower.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/19 10:54:56 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/19 10:54:56 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

int				tolower(int c)
{
	if (c <= 'Z' && c >= 'A')
		return (c | ('a' - 1));
	return (c);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strupcase.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/28 18:40:06 by dbrophy           #+#    #+#             */
/*   Updated: 2019/10/28 18:40:06 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

char			*strupcase(char *str)
{
	int i;

	i = -1;
	while (str[++i])
		if (str[i] >= 'a' && str[i] <= 'z')
			str[i] -= 32;
	return (str);
}

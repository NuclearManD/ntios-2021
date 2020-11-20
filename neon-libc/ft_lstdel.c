/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstdel.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/21 13:47:38 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/21 13:47:38 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libneon.h"
#include "stdlib.h"

void			lstdel(t_list **alst, void (*del)(void *, size_t))
{
	if (alst == NULL)
		return ;
	if ((*alst)->next != NULL)
		lstdel(&((*alst)->next), del);
	if (del != NULL && (*alst)->content != NULL)
		del((*alst)->content, (*alst)->content_size);
	free(*alst);
	*alst = NULL;
}

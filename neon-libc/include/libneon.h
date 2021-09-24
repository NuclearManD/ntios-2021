/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   libft.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbrophy <dbrophy@student.42.us.org>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/02/21 14:23:55 by dbrophy           #+#    #+#             */
/*   Updated: 2020/02/21 14:23:55 by dbrophy          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LIBFT_H
# define LIBFT_H

# include "string.h"

typedef struct	s_list
{
	void			*content;
	size_t			content_size;
	struct s_list	*next;
}				t_list;

# define FLAG_ERR_INVALID -3
# define FLAG_ERR_NONE_SPECIFIED -2

# ifdef __cplusplus
extern "C" {
# endif

long			ft_getflags(int argc, char **argv, char *flags);
int				ft_hasoption(int argc, char **argv, char *option);
char			*ft_getoption(int argc, char **argv, char *option);

//int				atoi(char *str);
int				atoi_base(char *str, char *base);
void			bzero(void *s, size_t n);
int				imax(int a, int b);
int				imin(int a, int b);
int				isalnum(int c);
int				isalpha(int c);
int				isascii(int c);
int				isdigit(int c);
int				isprint(int c);
int				isspace(int c);
char			*ft_itoa(int n);
int				logi(int base, int val);
void			lstadd(t_list **alst, t_list *_new);
void			lstdel(t_list **alst, void (*del)(void *, size_t));
void			lstdelone(t_list **alst, void (*del)(void *, size_t));
void			lstiter(t_list *lst, void (*f)(t_list *elem));
t_list			*lstmap(t_list *lst, t_list *(*f)(t_list *elem));
t_list			*lstnew(const void *content, size_t content_size);
void			*memalloc(size_t size);
void			*memccpy(void *dest, const void *src, int c, size_t n);
int				memcmp(const void *s1, const void *s2, size_t n);
void			*memcpy(void *dest, const void *src, size_t n);
void			memdel(void **ap);
void			*memdup(void *src, size_t size);
void			*memmove(void *dest, const void *src, size_t n);
void			*memset(void *s, int c, size_t n);
void			putnbr(int nb);
void			putnbr_base(int nb, char *base);
void			putnbr_fd(int nb, int fd);
void			putstr(char *str);
void			putstr_fd(const char *str, int fd);
long			ft_signature_s(char *str);
long			ft_signature_i(int val);
long			ft_signature_l(long val);
long			ft_signature_d(double val);
int				str_is_alpha(char *str);
int				str_is_lowercase(char *str);
int				str_is_numeric(char *str);
int				str_is_printable(char *str);
int				str_is_uppercase(char *str);
char			*strcapitalize(char *str);
//char			*strcat(char *dest, char *src);
//char			*strchr(char *s, int c);
int				strchri(const char *s, char c);
void			strclr(char *s);
//int				strcmp(char *s1, char *s2);
int				strcnt(const char *s, char c);
int				strcnt_norep(const char *s, char c);
char			*strcpy(char *dest, const char *src);
char			*strcut(const char *s, char c);
void			strdel(char **as);
char			*strdup(const char *s);
int				strequ(const char *s1, const char *s2);
void			striter(char *s, void (*f)(char *));
void			striteri(char *s, void (*f)(unsigned int, char *));
char			*strjoin(const char *s1, const char *s2);
unsigned int	strlcat(char *dest, const char *src, unsigned int size);
unsigned int	strlcpy(char *dest, const char *src, unsigned int n);
size_t			strlen(const char *str);
char			*strlowcase(char *str);
char			*strmap(const char *s, char (*f)(char));
char			*strmapi(const char *s, char (*f)(unsigned int, char));
//char			*strncat(char *dest, const char *src, int nb);
//int				strncmp(const char *s1, const char *s2, unsigned int n);
//char			*strncpy(char *dest, const char *src, unsigned int n);
int				strnequ(const char *s1, const char *s2, size_t limit);
char			*strnew(size_t size);
char			*strnstr(const char *str, const char *to_find, size_t len);
//char			*strrchr(char *s, int c);
char			**strsplit(const char *s, char c);
//char			*strstr(char *str, const char *to_find);
char			*strsub(const char *s, unsigned int start, size_t len);
char			*strtrim(const char *s);
char			*strupcase(char *str);
int				tolower(int c);
int				toupper(int c);
int				wordsplit(char **arr, char *s, int maxcount);
int				wordcount(const char *s);

# ifdef __cplusplus
}
# endif

#endif

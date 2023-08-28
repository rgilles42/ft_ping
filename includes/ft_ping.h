/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgilles <rgilles@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/26 22:16:31 by rgilles           #+#    #+#             */
/*   Updated: 2023/08/28 15:39:28 by rgilles          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING_H
# define FT_PING_H

# include <stdbool.h>
# include <stdlib.h> // exit 
# include <stdio.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/time.h>
# include <libft.h>

# define PING_PKT_SIZE		84
# define PING_PKT_DATA_SIZE	(PING_PKT_SIZE - sizeof(struct icmp) - sizeof(time_t))
# define SWAP_ENDIANNESS_16(x) (x << 8 | x >> 8);
# define SWAP_ENDIANNESS_64(x) \
	( x >> 56 \
	| (x & (0xFFull << 48 )) >> 40 \
	| (x & (0xFFull << 40 )) >> 24 \
	| (x & (0xFFull << 32 )) >> 8 \
	| (x & (0xFFull << 24 )) << 8 \
	| (x & (0xFFull << 16 )) << 24 \
	| (x & (0xFFull << 8 )) << 40 \
	| (x & (0xFFull << 0 )) << 56 )

typedef struct __attribute__ ((__packed__))	s_reqframe {
	struct icmp	icmp_req;
	char		supplementary_data[36];
}											t_reqframe;

typedef struct __attribute__ ((__packed__))	s_respframe {
	char		ip_header[20];
	struct icmp	icmp_resp;
}											t_respframe;

typedef struct	s_curping {
	char*	hostname;
	char*	ip;
	bool	verb_flag;
}				t_curping;

void	parse_command(int argc, char** argv, t_curping* current_command);

#endif

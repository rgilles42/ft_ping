/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgilles <rgilles@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/26 22:16:31 by rgilles           #+#    #+#             */
/*   Updated: 2023/08/28 18:50:56 by rgilles          ###   ########.fr       */
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

# define PING_RECEIVED -42
# define SWAP_ENDIANNESS_16(x) (x << 8 | x >> 8);

typedef struct	s_curping {
	char*	hostname;
	char*	ip;
	bool	verb_flag;
	int		sock_fd;
	t_list*	timestamps_list;
}				t_curping;

typedef struct __attribute__ ((__packed__))	s_reqframe {
	struct icmp	icmp_req;
	char		supplementary_data[36];
}											t_reqframe;

typedef struct __attribute__ ((__packed__))	s_respframe {
	char		ip_header[20];
	struct icmp	icmp_resp;
}											t_respframe;

void	parse_command(int argc, char** argv, t_curping* current_command);
void	generate_request(t_curping* current_ping, t_reqframe* req_frame, struct sockaddr_in* ping_dstaddr);
void	handle_response(t_curping* current_ping, t_respframe resp_frame);

#endif

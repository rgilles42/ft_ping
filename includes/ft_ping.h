/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgilles <rgilles@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/26 22:16:31 by rgilles           #+#    #+#             */
/*   Updated: 2023/09/05 18:46:32 by rgilles          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING_H
# define FT_PING_H

# include <stdbool.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <libft.h>

# define PING_RECEIVED -42
# define SWAP_16(x) ((x & 0x00FF) << 8 | (x & 0xFF00) >> 8)

typedef struct	s_curping {
	char				hostname[254];
	bool				verb_flag;
	uint16_t			ttl;
	struct sockaddr_in	addr;
	char				ip[INET_ADDRSTRLEN];
	int					sock_fd;
	t_list*				timestamps_list;
}				t_curping;

typedef struct __attribute__ ((__packed__))	s_reqframe {
	struct icmp	icmp_req;
	char		supplementary_data[36];
}											t_reqframe;

typedef struct	s_respframe {
	struct iphdr	ip_header;
	struct icmp		icmp_resp;
	struct icmp		embedded_orig_icmp;
}											t_respframe;

void	print_error(const char *s);
void	parse_command(int argc, char** argv, t_curping* current_command);
void	generate_request(t_curping* current_ping, t_reqframe* req_frame);
void	handle_pong(t_curping* current_ping, t_respframe resp_frame);
void	handle_other_response(t_respframe resp_frame, char* message);

#endif

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgilles <rgilles@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/26 22:14:34 by rgilles           #+#    #+#             */
/*   Updated: 2023/09/04 23:57:21 by rgilles          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>
#include <ft_ping.h>
#include <string.h> //strerror
#include <netdb.h>	//getaddrinfo

t_curping			current_ping;

void	print_error(const char *s) {
	const char *colon;
	const char *errstring;
	if (s == NULL || *s == '\0')
		s = colon = "";
	else
		colon = ": ";
	errstring = strerror(errno);
	fprintf(stderr, "%s%s%s\n", s, colon, errstring);
}

void	sig_handler(int signo) {
	t_list*	tmp = current_ping.timestamps_list;
	int		tr_packets = 0;
	int		rcvd_packets = 0;

	(void)signo;
	printf("--- %s ping statistics ---\n", current_ping.hostname);
	while (tmp) {
		++tr_packets;
		if (tmp->rank == PING_RECEIVED)
			++rcvd_packets;
		tmp = tmp->next;
	}
	ft_lstclear(&current_ping.timestamps_list, free);
	printf("%d packets transmitted, %d packets received, %d%% packet loss\n", tr_packets, rcvd_packets, (tr_packets - rcvd_packets) * 100 / tr_packets);
	exit(0);
}

int		resolve_hostname() {
	struct addrinfo		hints;
	struct addrinfo*	getaddrinfo_results;
	int					gai_res;

	ft_memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_RAW; /* Datagram socket */
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_protocol = IPPROTO_ICMP;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	if ((gai_res = getaddrinfo(current_ping.hostname, NULL, &hints, &getaddrinfo_results))) {
		fprintf(stderr, "ft_ping: error resolving host \'%s\': %s\n", current_ping.hostname, gai_strerror(gai_res));
		return -1;
	}
	current_ping.addr.sin_family = AF_INET;
	current_ping.addr.sin_port = 0;
	current_ping.addr.sin_addr.s_addr = ((struct sockaddr_in*)getaddrinfo_results->ai_addr)->sin_addr.s_addr;
	freeaddrinfo(getaddrinfo_results);
	return 0;
}

int	main(int argc, char** argv) {
	struct timeval		timeout_sockopt;
	t_reqframe			req_frame;
	t_respframe			resp_frame;
	struct timeval		curr_timestamp;
	unsigned long		prev_req_timestamp;
	struct sockaddr_in	remote_addr;

	parse_command(argc, argv, &current_ping);

	if (resolve_hostname()) {
		exit(-1);
	}
	if (!inet_ntop(current_ping.addr.sin_family, &current_ping.addr.sin_addr, current_ping.ip, INET_ADDRSTRLEN)) {
		print_error("ft_ping: ip address field conversion");
		exit(-1);
	}


	if ((current_ping.sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
		print_error("ft_ping: icmp open socket");
		exit(-1);
	}

	timeout_sockopt.tv_sec = 5;
	timeout_sockopt.tv_usec = 0;

	if (setsockopt(current_ping.sock_fd, SOL_IP, IP_TTL, &current_ping.ttl, sizeof(current_ping.ttl)) ||
		setsockopt(current_ping.sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout_sockopt, sizeof(timeout_sockopt))
		)
		exit(1);


	ft_bzero(&req_frame, sizeof(req_frame));
	req_frame.icmp_req.icmp_type = ICMP_ECHO;
	req_frame.icmp_req.icmp_code = 0;
	req_frame.icmp_req.icmp_id = SWAP_ENDIANNESS_16((uint16_t)getpid());
	req_frame.icmp_req.icmp_seq = 0;

	prev_req_timestamp = 0;
	printf("PING %s (%s): %lu data bytes\n", current_ping.hostname, current_ping.ip, sizeof(req_frame.icmp_req.icmp_dun) + sizeof(req_frame.supplementary_data));

	signal(SIGINT, sig_handler);

	while (1) {
		gettimeofday(&curr_timestamp, NULL);
		if (curr_timestamp.tv_sec - prev_req_timestamp) {
			generate_request(&current_ping, &req_frame);
			prev_req_timestamp = curr_timestamp.tv_sec;
		}
		if (recvfrom(current_ping.sock_fd, &resp_frame, sizeof(resp_frame), MSG_DONTWAIT, NULL, NULL) > 0) {
			remote_addr.sin_family = AF_INET;
			remote_addr.sin_port = 0;
			remote_addr.sin_addr.s_addr = *(uint32_t*)&resp_frame.ip_header[12];
			switch (resp_frame.icmp_resp.icmp_type) {
				case ICMP_ECHOREPLY:
					if (resp_frame.icmp_resp.icmp_id == req_frame.icmp_req.icmp_id)
						handle_pong(&current_ping, resp_frame, remote_addr);
					break;
				case ICMP_UNREACH:
					// code 1: Dest Host Unr or code 3: Dest Net Unr
					printf("ICMP_UNREACH %u\n", resp_frame.icmp_resp.icmp_code);
					break;
				case ICMP_TIMXCEED:
					printf("ICMP_TIMXCEED\n");
					break;
			}
		}
	}
}

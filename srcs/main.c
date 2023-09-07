/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgilles <rgilles@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/26 22:14:34 by rgilles           #+#    #+#             */
/*   Updated: 2023/09/05 19:05:16 by rgilles          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

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
	if (tr_packets)
		printf("%d packets transmitted, %d packets received, %d%% packet loss\n", tr_packets, rcvd_packets, (tr_packets - rcvd_packets) * 100 / tr_packets);
	exit(0);
}

int		resolve_hostname() {
	struct addrinfo		hints;
	struct addrinfo*	getaddrinfo_results;
	int					gai_res;

	ft_memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_ICMP;
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

void	print_ip_header(struct iphdr	ip) {
	char	ip_buf[INET_ADDRSTRLEN];
	printf("IP Hdr Dump:\n");
	for (uint16_t i = 0; i < 20; ++i) {
		if (!(i % 2))
			printf(" ");
		printf("%02x", ((uint8_t*)&ip)[i]);
	}
	printf("\n");
	ip.tot_len = SWAP_16(ip.tot_len);
	ip.id = SWAP_16(ip.id);
	ip.frag_off = SWAP_16(ip.frag_off);
	ip.check = SWAP_16(ip.check);
	printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst     Data\n");
	printf (" %1x  %1x  %02x %04x %04x   %1x %04x  %02x  %02x %04x",
		ip.version, ip.ihl, ip.tos, ip.tot_len, ip.id, ip.frag_off >> 13, ip.frag_off & 0x1fff, ip.ttl, ip.protocol, ip.check);
	printf (" %s ", inet_ntop(AF_INET, &ip.saddr, ip_buf, INET_ADDRSTRLEN));
	printf (" %s ", inet_ntop(AF_INET, &ip.daddr, ip_buf, INET_ADDRSTRLEN));
	printf("\n");
}

int		main(int argc, char** argv) {
	struct timeval		timeout_sockopt;
	t_reqframe			req_frame;
	t_respframe			resp_frame;
	struct timeval		curr_timestamp;
	unsigned long		prev_req_timestamp;

	parse_command(argc, argv, &current_ping);
	if (resolve_hostname()) {
		exit(-1);
	}
	inet_ntop(current_ping.addr.sin_family, &current_ping.addr.sin_addr, current_ping.ip, INET_ADDRSTRLEN);
	
	if ((current_ping.sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
		print_error("ft_ping: icmp open socket");
		exit(-1);
	}
	
	timeout_sockopt.tv_sec = 5;
	timeout_sockopt.tv_usec = 0;
	if (setsockopt(current_ping.sock_fd, SOL_IP, IP_TTL, &current_ping.ttl, sizeof(current_ping.ttl)) ||
		setsockopt(current_ping.sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout_sockopt, sizeof(timeout_sockopt))
	) {
		print_error("ft_ping: setsockopt");
		exit(1);
	}
	
	ft_bzero(&req_frame, sizeof(req_frame));
	req_frame.icmp_req.icmp_type = ICMP_ECHO;
	req_frame.icmp_req.icmp_code = 0;
	req_frame.icmp_req.icmp_id = SWAP_16((uint16_t)getpid());
	req_frame.icmp_req.icmp_seq = 0;
	gettimeofday(&curr_timestamp, NULL);
	prev_req_timestamp = curr_timestamp.tv_sec;
	
	if (current_ping.verb_flag)
		printf("PING %s (%s): %lu data bytes, id 0x%04x = %u\n", current_ping.hostname, current_ping.ip,
			sizeof(req_frame.icmp_req.icmp_dun) + sizeof(req_frame.supplementary_data), (uint16_t)getpid(), (uint16_t)getpid());
	else
		printf("PING %s (%s): %lu data bytes\n", current_ping.hostname, current_ping.ip,
			sizeof(req_frame.icmp_req.icmp_dun) + sizeof(req_frame.supplementary_data));
	
	signal(SIGINT, sig_handler);
	
	while (1) {
		gettimeofday(&curr_timestamp, NULL);
		if (curr_timestamp.tv_sec - prev_req_timestamp) {
			generate_request(&current_ping, &req_frame);
			prev_req_timestamp = curr_timestamp.tv_sec;
		}
		if (recvfrom(current_ping.sock_fd, &resp_frame, sizeof(resp_frame), MSG_DONTWAIT, NULL, NULL) > 0) {
			if (resp_frame.icmp_resp.icmp_type == ICMP_ECHOREPLY && resp_frame.icmp_resp.icmp_id == req_frame.icmp_req.icmp_id) {
					handle_pong(&current_ping, resp_frame);
			} else if (resp_frame.embedded_orig_icmp.icmp_id == req_frame.icmp_req.icmp_id) {
				if (resp_frame.icmp_resp.icmp_type == ICMP_UNREACH)
					handle_other_response(resp_frame, resp_frame.icmp_resp.icmp_code == 3 ? "Destination Net Unreachable" : "Destination Host Unreachable");
				else if (resp_frame.icmp_resp.icmp_type == ICMP_TIMXCEED)
					handle_other_response(resp_frame, "Time to live exceeded");
				else
					handle_other_response(resp_frame, "Unexpected ICMP response");
				if (current_ping.verb_flag) {
					print_ip_header(*(struct iphdr*)&resp_frame.icmp_resp.icmp_dun.id_ip.idi_ip);
					printf("ICMP: type %u, code %u, size %u, id 0x%04x, seq 0x%04x\n",
						resp_frame.embedded_orig_icmp.icmp_type, resp_frame.embedded_orig_icmp.icmp_code,
						SWAP_16((*(struct iphdr*)&resp_frame.icmp_resp.icmp_dun.id_ip.idi_ip).tot_len) - 20,
						SWAP_16(resp_frame.embedded_orig_icmp.icmp_id), SWAP_16(resp_frame.embedded_orig_icmp.icmp_seq)
					);
				}
			}
			ft_bzero(&resp_frame, sizeof(resp_frame));
		}
	}
}

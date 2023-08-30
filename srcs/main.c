/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgilles <rgilles@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/26 22:14:34 by rgilles           #+#    #+#             */
/*   Updated: 2023/08/30 16:46:20 by rgilles          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>
#include <ft_ping.h>

t_curping			current_ping;

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

int	main(int argc, char** argv) {
	t_reqframe			req_frame;
	t_respframe			resp_frame;
	struct timeval		curr_timestamp;
	unsigned long		prev_req_timestamp;

	parse_command(argc, argv, &current_ping);

	current_ping.addr.sin_family = AF_INET;
	current_ping.addr.sin_port = 0;
	if (!inet_pton(AF_INET, current_ping.hostname, &current_ping.addr.sin_addr)) {
		printf("%s\n", "TODO: DNS resolution");
		exit(1);
	}

	if (!inet_ntop(current_ping.addr.sin_family, &current_ping.addr.sin_addr, current_ping.ip, INET_ADDRSTRLEN)) {
		perror("ntop");
		exit(-1);
	}


	if ((current_ping.sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
		perror("socket");
		exit(-1);
	}

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
			generate_request(&current_ping, &req_frame, &current_ping.addr);
			prev_req_timestamp = curr_timestamp.tv_sec;
		}

		if (recvfrom(current_ping.sock_fd, &resp_frame, sizeof(resp_frame), MSG_DONTWAIT, NULL, NULL) > 0
		&& *(uint32_t*)&resp_frame.ip_header[12] == current_ping.addr.sin_addr.s_addr) {
			handle_response(&current_ping, resp_frame);
		}
	}
}

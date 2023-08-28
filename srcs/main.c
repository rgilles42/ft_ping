/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgilles <rgilles@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/26 22:14:34 by rgilles           #+#    #+#             */
/*   Updated: 2023/08/28 17:29:14 by rgilles          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>

uint16_t	checksum(void *addr, int size) {
	uint16_t	*buff;
	uint32_t	sum;

	buff = (uint16_t *)addr;
	for (sum = 0; size > 1; size -= 2)
		sum += *buff++;
	if (size == 1)
		sum += *(uint8_t*)buff;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return (~sum);
}

void	add_timestamp(t_list** timestamp_list, uint16_t icmpseq, struct timeval* timestamp){
	struct timeval*	ts;
	t_list*			node;

	ts = malloc(sizeof(struct timeval));
	if (!ts) {
		perror("malloc");
		exit(-1);
	}
	node = ft_lstnew(ts);
	node->rank = icmpseq;
	ft_memcpy(node->content, timestamp, sizeof(struct timeval));
	ft_lstadd_back(timestamp_list, node);
}

struct timeval*	get_timestamp(t_list* timestamp_list, uint16_t icmpseq){
	while (timestamp_list->rank != icmpseq){
		timestamp_list = timestamp_list->next;
	}
	return timestamp_list->content;
}

int	main(int argc, char** argv) {
	t_curping			current_ping;
	int					sock_fd;
	t_reqframe			req_frame;
	struct sockaddr_in	ping_dstaddr;
	t_list				*timestamp_list;
	unsigned long		prev_req_timestamp;
	struct timeval		curr_timestamp;
	struct timeval		timestamp_resp;
	t_respframe			resp_frame;

	timestamp_list = NULL;
	ft_bzero(&current_ping, sizeof(current_ping));
	parse_command(argc, argv, &current_ping);

	if ((sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
		perror("socket");
		exit(-1);
	}


	ping_dstaddr.sin_family = AF_INET;
	ping_dstaddr.sin_port = 0;
	if (!inet_pton(AF_INET, current_ping.hostname, &ping_dstaddr.sin_addr)) {
		printf("%s\n", "TODO: DNS resolution");
		exit(1);
	} else {
		current_ping.ip = current_ping.hostname;
	}

	ft_bzero(&req_frame, sizeof(req_frame));
	req_frame.icmp_req.icmp_type = ICMP_ECHO;
	req_frame.icmp_req.icmp_code = 0;
	req_frame.icmp_req.icmp_id = SWAP_ENDIANNESS_16((uint16_t)getpid());
	req_frame.icmp_req.icmp_seq = 0;

	prev_req_timestamp = 0;
	printf("PING %s (%s): %lu data bytes\n", current_ping.hostname, current_ping.ip, sizeof(req_frame.icmp_req.icmp_dun) + sizeof(req_frame.supplementary_data));

	while (1) {
		gettimeofday(&curr_timestamp, NULL);
		if (curr_timestamp.tv_sec - prev_req_timestamp) {
			ft_memcpy(&req_frame.icmp_req.icmp_dun, &curr_timestamp.tv_sec, sizeof(curr_timestamp.tv_sec));
			ft_memcpy((char*)&req_frame.icmp_req.icmp_dun + sizeof(curr_timestamp.tv_sec), "googoogaga", 11);
			req_frame.icmp_req.icmp_cksum = 0;
			req_frame.icmp_req.icmp_cksum = checksum(&req_frame, sizeof(req_frame));

			if (sendto(sock_fd, &req_frame, sizeof(req_frame), 0, (struct sockaddr*)&ping_dstaddr, sizeof(ping_dstaddr)) < 0) {
				perror("sendto");
				exit(-1);
			}
			add_timestamp(&timestamp_list, req_frame.icmp_req.icmp_seq, &curr_timestamp);
			req_frame.icmp_req.icmp_seq += SWAP_ENDIANNESS_16((uint16_t)1);
			prev_req_timestamp = curr_timestamp.tv_sec;
		}

		if (recvfrom(sock_fd, &resp_frame, sizeof(resp_frame), MSG_DONTWAIT, NULL, NULL) > 0
		&& *(uint32_t*)&resp_frame.ip_header[12] == ping_dstaddr.sin_addr.s_addr
		&& resp_frame.icmp_resp.icmp_type == ICMP_ECHOREPLY) {
			gettimeofday(&timestamp_resp, NULL);
			uint16_t icmp_resp_size = (*(uint16_t*)&resp_frame.ip_header[2] << 8 | *(uint16_t*)&resp_frame.ip_header[2] >> 8) - 20 ;
			uint16_t icmp_resp_seq = SWAP_ENDIANNESS_16(resp_frame.icmp_resp.icmp_seq);
			struct timeval* req_ts = get_timestamp(timestamp_list, resp_frame.icmp_resp.icmp_seq);
			printf("%u bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n", icmp_resp_size, current_ping.ip, icmp_resp_seq, resp_frame.ip_header[8], (timestamp_resp.tv_sec - req_ts->tv_sec) * 1000.0 + (timestamp_resp.tv_usec - req_ts->tv_usec) / 1000.0);
		}
	}
}

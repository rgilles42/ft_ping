/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   io.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgilles <rgilles@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/28 17:56:02 by rgilles           #+#    #+#             */
/*   Updated: 2023/09/04 23:57:27 by rgilles          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>
#include <arpa/inet.h> // inet_ntop

struct timeval*	get_timestamp(t_list* timestamps_list, uint16_t icmpseq){
	while (timestamps_list->rank != icmpseq)
		timestamps_list = timestamps_list->next;
	timestamps_list->rank = PING_RECEIVED;
	return timestamps_list->content;
}

void	handle_pong(t_curping* current_ping, t_respframe resp_frame, struct sockaddr_in remote_addr) {
	struct timeval		timestamp_resp;
	struct timeval*		req_ts;
	char				remote_ip[INET_ADDRSTRLEN];
	uint16_t			icmp_resp_size;
	uint16_t			icmp_resp_seq;

	if (!inet_ntop(remote_addr.sin_family, &remote_addr.sin_addr, remote_ip, INET_ADDRSTRLEN)) {
		print_error("ft_ping: ip address field conversion");
		exit(-1);
	}
	icmp_resp_size = SWAP_ENDIANNESS_16(resp_frame.ip_header.tot_len) - 20 ;
	icmp_resp_seq = SWAP_ENDIANNESS_16(resp_frame.icmp_resp.icmp_seq);
	gettimeofday(&timestamp_resp, NULL);
	req_ts = get_timestamp(current_ping->timestamps_list, resp_frame.icmp_resp.icmp_seq);
	printf("%u bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n", icmp_resp_size, remote_ip, icmp_resp_seq, resp_frame.ip_header.ttl, (timestamp_resp.tv_sec - req_ts->tv_sec) * 1000.0 + (timestamp_resp.tv_usec - req_ts->tv_usec) / 1000.0);
}

void	add_timestamp(t_list** timestamps_list, uint16_t icmpseq, struct timeval* timestamp){
	struct timeval*	ts;
	t_list*			node;

	ts = malloc(sizeof(struct timeval));
	if (!ts) {
		print_error("ft_ping: malloc");
		exit(-1);
	}
	node = ft_lstnew(ts);
	node->rank = icmpseq;
	ft_memcpy(node->content, timestamp, sizeof(struct timeval));
	ft_lstadd_back(timestamps_list, node);
}

uint16_t	gen_checksum(void *addr, int size) {
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

void	generate_request(t_curping* current_ping, t_reqframe* req_frame) {
	struct timeval		curr_timestamp;

	gettimeofday(&curr_timestamp, NULL);
	ft_memcpy(&req_frame->icmp_req.icmp_dun, &curr_timestamp.tv_sec, sizeof(curr_timestamp.tv_sec));
	ft_memcpy((char*)&req_frame->icmp_req.icmp_dun + sizeof(curr_timestamp.tv_sec), "googoogaga", 11);
	req_frame->icmp_req.icmp_cksum = 0;
	req_frame->icmp_req.icmp_cksum = gen_checksum(req_frame, sizeof(*req_frame));
	if (sendto(current_ping->sock_fd, req_frame, sizeof(*req_frame), 0, (struct sockaddr*)&current_ping->addr, sizeof(current_ping->addr)) < 0) {
		print_error("ft_ping: request failed to be sent through socket");
		exit(-1);
	}
	add_timestamp(&current_ping->timestamps_list, req_frame->icmp_req.icmp_seq, &curr_timestamp);
	req_frame->icmp_req.icmp_seq += SWAP_ENDIANNESS_16((uint16_t)1);
}

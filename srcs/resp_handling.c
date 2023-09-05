/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   resp_handling.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgilles <rgilles@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/05 14:33:10 by rgilles           #+#    #+#             */
/*   Updated: 2023/09/05 15:21:22 by rgilles          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>
#include <arpa/inet.h> // inet_ntop
#include <sys/socket.h>
#include <netdb.h>	// getnameinfo

struct timeval*	get_timestamp(t_list* timestamps_list, uint16_t icmpseq){
	while (timestamps_list->rank != icmpseq)
		timestamps_list = timestamps_list->next;
	timestamps_list->rank = PING_RECEIVED;
	return timestamps_list->content;
}

void	handle_pong(t_curping* current_ping, t_respframe resp_frame, struct sockaddr_in* remote_addr) {
	struct timeval		timestamp_resp;
	struct timeval*		req_ts;
	char				remote_ip[INET_ADDRSTRLEN];
	uint16_t			icmp_resp_size;
	uint16_t			icmp_resp_seq;

	if (!inet_ntop(remote_addr->sin_family, &remote_addr->sin_addr, remote_ip, INET_ADDRSTRLEN)) {
		print_error("ft_ping: ip address field conversion");
		exit(-1);
	}
	icmp_resp_size = SWAP_ENDIANNESS_16(resp_frame.ip_header.tot_len) - 20 ;
	icmp_resp_seq = SWAP_ENDIANNESS_16(resp_frame.icmp_resp.icmp_seq);
	gettimeofday(&timestamp_resp, NULL);
	req_ts = get_timestamp(current_ping->timestamps_list, resp_frame.icmp_resp.icmp_seq);
	printf("%u bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n", icmp_resp_size, remote_ip, icmp_resp_seq, resp_frame.ip_header.ttl, (timestamp_resp.tv_sec - req_ts->tv_sec) * 1000.0 + (timestamp_resp.tv_usec - req_ts->tv_usec) / 1000.0);
}

void	handle_other_response(t_respframe resp_frame, struct sockaddr_in* remote_addr, char* message) {
	char				remote_ip[INET_ADDRSTRLEN];
	uint16_t			icmp_resp_size;
	char				remote_hostname[254];

	ft_bzero(remote_hostname, sizeof(remote_hostname));
	if (!inet_ntop(remote_addr->sin_family, &remote_addr->sin_addr, remote_ip, INET_ADDRSTRLEN)) {
		print_error("ft_ping: ip address field conversion");
		exit(-1);
	}
	icmp_resp_size = SWAP_ENDIANNESS_16(resp_frame.ip_header.tot_len) - 20;
	if (!getnameinfo((struct sockaddr*)remote_addr, sizeof(struct sockaddr_in), remote_hostname, sizeof(remote_hostname), NULL, 0, 0) && remote_hostname[0])
		printf("%u bytes from %s (%s): %s\n", icmp_resp_size, remote_hostname, remote_ip, message);
	else
		printf("%u bytes from %s: %s\n", icmp_resp_size, remote_ip, message);
}
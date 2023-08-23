/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgilles <rgilles@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/26 22:14:34 by rgilles           #+#    #+#             */
/*   Updated: 2023/08/23 19:20:01 by rgilles          ###   ########.fr       */
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

int	main(int argc, char** argv) {
	t_curping			current_ping;
	int					sock_fd;
	t_reqframe			req_frame;
	struct sockaddr_in	ping_dstaddr;
	struct timeval		timestamp;
	t_respframe			resp_frame;

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
	gettimeofday(&timestamp, NULL);

	ft_bzero(&req_frame, sizeof(req_frame));
	req_frame.icmp_req.icmp_type = ICMP_ECHO;
	req_frame.icmp_req.icmp_code = 0;
	req_frame.icmp_req.icmp_id = SWAP_ENDIANNESS_16((uint16_t)getpid());

	printf("PING %s (%s): %lu data bytes\n", current_ping.hostname, current_ping.ip, sizeof(req_frame.icmp_req.icmp_dun) + sizeof(req_frame.supplementary_data));

	req_frame.icmp_req.icmp_seq = 0;
	ft_memcpy(&req_frame.icmp_req.icmp_dun, &timestamp.tv_sec, sizeof(timestamp.tv_sec));
	ft_memcpy((char*)&req_frame.icmp_req.icmp_dun + sizeof(timestamp.tv_sec), "googoogaga", 11);
	req_frame.icmp_req.icmp_cksum = checksum(&req_frame.icmp_req, sizeof(req_frame.icmp_req));

	if (sendto(sock_fd, &req_frame, sizeof(req_frame), 0, (struct sockaddr*)&ping_dstaddr, sizeof(ping_dstaddr)) < 0) {
		perror("sendto");
		exit(-1);
	}

	if (recvfrom(sock_fd, &resp_frame, sizeof(resp_frame), 0, NULL, NULL) < 0) {
		perror("recvfrom");
		exit(-1);
	}
	uint16_t icmp_resp_size = (*(uint16_t*)&resp_frame.ip_header[2] << 8 | *(uint16_t*)&resp_frame.ip_header[2] >> 8) - 20 ;
	printf("%u bytes from %s: icmp_seq=%u ttl=%u time=uwu ms\n", icmp_resp_size, current_ping.ip, resp_frame.icmp_resp.icmp_type, resp_frame.ip_header[8]);

}

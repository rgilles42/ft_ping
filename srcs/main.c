/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgilles <rgilles@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/26 22:14:34 by rgilles           #+#    #+#             */
/*   Updated: 2023/08/22 18:13:23 by rgilles          ###   ########.fr       */
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
	struct icmp			icmp_req;
	struct sockaddr_in	ping_dstaddr;
	struct timeval		timestamp;
	t_respframe			resp_frame;

	ft_bzero(&current_ping, sizeof(current_ping));
	parse_command(argc, argv, &current_ping);

	if ((sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
		perror("socket");
		exit(-1);
	}

	ft_bzero(&icmp_req, sizeof(icmp_req));
	//eventual dns resolution
	current_ping.ip = current_ping.hostname;
	gettimeofday(&timestamp, NULL);

	inet_pton(AF_INET, current_ping.ip, &ping_dstaddr.sin_addr);
	ping_dstaddr.sin_family = AF_INET;
	ping_dstaddr.sin_port = 0;

	icmp_req.icmp_type = ICMP_ECHO;
	icmp_req.icmp_code = 0;
	icmp_req.icmp_id = SWAP_ENDIANNESS_16((uint16_t)getpid());

	printf("PING %s (%s): %lu data bytes\n", current_ping.hostname, current_ping.ip, sizeof(icmp_req.icmp_dun));

	icmp_req.icmp_seq = 0;
	ft_memcpy(&icmp_req.icmp_dun, &timestamp.tv_sec, sizeof(timestamp.tv_sec));
	ft_memcpy((char*)&icmp_req.icmp_dun + sizeof(timestamp.tv_sec), "googoogaga", 11);
	icmp_req.icmp_cksum = checksum(&icmp_req, sizeof(icmp_req));

	if (sendto(sock_fd, &icmp_req, sizeof(icmp_req), 0, (struct sockaddr*)&ping_dstaddr, sizeof(ping_dstaddr)) < 0) {
		perror("sendto");
		exit(-1);
	}

	if (recvfrom(sock_fd, &resp_frame, sizeof(resp_frame), 0, NULL, (socklen_t*)sizeof(ping_dstaddr)) < 0) {
		perror("recvfrom");
		exit(-1);
	}
	uint16_t icmp_resp_size = (*(uint16_t*)&resp_frame.ip_header[2] << 8 | *(uint16_t*)&resp_frame.ip_header[2] >> 8) - 20 ;
	printf("%u bytes from %s: icmp_seq=%u ttl=%u time=uwu ms\n", icmp_resp_size, current_ping.ip, resp_frame.icmp_resp.icmp_type, resp_frame.ip_header[8]);

}

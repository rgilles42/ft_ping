/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgilles <rgilles@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/26 22:14:34 by rgilles           #+#    #+#             */
/*   Updated: 2023/08/03 17:01:20 by rgilles          ###   ########.fr       */
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

int	main(void) {
	int					sock_fd;
	//t_curping			current_ping;
	struct icmp			icmp_frame;
	struct sockaddr_in	ping_sockaddr;
	struct timeval		timestamp;

	ft_bzero(&icmp_frame, sizeof(icmp_frame));

	//parse_command(argc, argv, &current_ping);
	gettimeofday(&timestamp, NULL);

	inet_pton(AF_INET, "8.8.8.8", &ping_sockaddr.sin_addr);
	ping_sockaddr.sin_family = AF_INET;
	ping_sockaddr.sin_port = 0;


	icmp_frame.icmp_type = ICMP_ECHO;
	icmp_frame.icmp_code = 0;
	icmp_frame.icmp_id = SWAP_ENDIANNESS_16((uint16_t)getpid());
	icmp_frame.icmp_seq = 0;
	ft_memcpy(&icmp_frame.icmp_dun, &timestamp.tv_sec, sizeof(timestamp.tv_sec));
	ft_memcpy(&icmp_frame.icmp_dun + sizeof(timestamp.tv_sec), "googoogaga", 11);

	icmp_frame.icmp_cksum = checksum(&icmp_frame, sizeof(icmp_frame));
	printf("%lu\n", sizeof(timestamp.tv_sec));


	if ((sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
		perror("socket");
	}
	if (sendto(sock_fd, &icmp_frame, sizeof(icmp_frame), 0, (struct sockaddr*)&ping_sockaddr, sizeof(ping_sockaddr)) < 0) {
		perror("sendto");
	}

}

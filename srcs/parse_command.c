/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_command.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgilles <rgilles@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/27 16:24:21 by rgilles           #+#    #+#             */
/*   Updated: 2023/08/22 18:13:30 by rgilles          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>

void	print_help() {
	printf("%s\n", "Usage: ft_ping [OPTION...] HOST ...");
	printf("%s\n", "Send ICMP ECHO_REQUEST packets to network hosts.");
}

void	parse_command(int argc, char** argv, t_curping* current_command) {
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-' && argv[i][1]) {
			switch (argv[i][1]) {
				case 'v':
					current_command->verb_flag = true;
					break;
				case 'h':
				case '?':
					print_help();
					exit(0);
				default:
					printf("ft_ping: invalid option -- '%c'\n", argv[i][1]);
					exit(64);
			}
		}
		else if (current_command->hostname == NULL) {
			current_command->hostname = argv[i];
		}
	}
	if (!current_command->hostname) {
		printf("%s\n", "ft_ping: missing host operand");
		exit(64);
	}
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_command.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgilles <rgilles@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/27 16:24:21 by rgilles           #+#    #+#             */
/*   Updated: 2023/09/04 18:40:07 by rgilles          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_ping.h>
#include <stdio.h>

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
				case '-':
					if (argv[i][2] == 't' && argv[i][3] == 't' && argv[i][4] == 'l')
						if (argc > i + 1 && argv[++i]) {
							current_command->ttl = ft_atoi(argv[i]);
							if (current_command->ttl > 0)
								break;
							--i;
						}
					// fall-through
				default:
					fprintf(stderr, "ft_ping: invalid option -- '%c'\n", argv[i][1]);
					exit(64);
			}
		}
		else if (current_command->hostname[0] == '\0') {
			ft_strlcpy(current_command->hostname, argv[i], 254);
		}
	}
	if (!current_command->hostname[0]) {
		fprintf(stderr, "%s\n", "ft_ping: missing host operand");
		exit(64);
	}
	if (!current_command->ttl)
		current_command->ttl = 64;
}

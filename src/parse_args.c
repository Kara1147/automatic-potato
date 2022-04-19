/*
 * MIT License
 * 
 * Copyright (c) 2022 Kara
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "parse_args.h"

void usage()
{
	puts("\
Usage: onslaught [options...] URL [URL...]\n\
   or: onslaught [options...] URL [URL...]\n\
                  -P PROCESS [process options...] [URL]\n\
                 [-P PROCESS [process options...] [URL] ...]\n\
   or: onslaught [options...]\n\
                  -P PROCESS [process options...] URL\n\
                 [-P PROCESS [process options...] URL ...]\n\
Send http(s) requests to URL.\n\
A URL may be specified in the process configuration; which overrides the\n\
global URL assignment.\n\
The protocol, port, host, and path are determined by the URL parameter:\n\
URL is in the format: [protocol://]host[:port][/path?args]
\n\
Options:\n\
 Global Options:\n\
  -h, --help                 show this message\n\
  -p, --processes PROCESSES  number of parallel processes to run (default 5)\n\
  -P, --configure PROCESS    specify process options for the process identified\n\
                             by PROCESS: a number from 1 to PROCESSES\n\
                             this option may be used multiple times\n\
 Process Options:\n\
  -d, --delay DELAY          minimum delay in milliseconds between requests\n\
                             if specified, the process will choose a random\n\
                             delay between DELAY and MAX_DELAY\n\
  -D, --max-delay MAX_DELAY  maximum delay in milliseconds between requests\n\
                             if not specified, defaults to DELAY
  -H, --header HEADER/@FILE  pass custom header(s) to server\n\
  -L, --location             follow redirects\n\
  -m, --max-time TIMEOUT     timeout in milliseconds for each request\n\
");
}

process_config get_config(daemon_config dconf, unsigned long int process)
{
	unsigned long int i;

	for (i = 0; i < dconf->configs; i++)
		if (dconf->pconf[i].process == process)
			return &(dconf->pconf[i]);

	return NULL;
}

process_config new_config(daemon_config dconf, unsigned long int process)
{
	void *tmp;
	process_config pconf = NULL;

	if (process > dconf->processes) {
		ERROR("cannot configure options for process %ld: max processes is %ld\n",
		      process, dconf->processes);
		usage();
		return NULL;
	}

	if ((pconf = get_config(dconf, process)) != NULL)
		return pconf;

	if ((tmp = reallocarray(dconf->pconf, dconf->configs + 1,
				sizeof(struct process_config))) == NULL) {
		ERROR("cannot allocate memory for process configuration\n");
		return NULL;
	}
	dconf->pconf = tmp;

	pconf = &(dconf->pconf[dconf->configs++]);

	if (dconf->configs == 1) {
		/* initialize */
		memset(pconf, 0, sizeof(struct process_config));
		pconf->max_time = 5000;
		pconf->processes = 5;
	} else {
		memcpy(pconf, dconf->pconf, sizeof(struct process_config));
	}

	pconf->process = process;

	return pconf;
}

int add_header(process_config pconf, const char *header)
{
	void *tmp;

	if ((tmp = reallocarray(pconf->headers, pconf->num_headers + 1,
				sizeof(char *))) == NULL) {
		errno = ENOMEM;
		ERROR("cannot allocate memory for header\n");
		return -1;
	}
	pconf->headers = tmp;

	pconf->headers[pconf->num_headers++] = header;

	return 0;
}

void free_args(daemon_config dconf)
{
	unsigned long int i;
	for (i = 0; i < dconf->configs; i++) {
		if (dconf->pconf[i].headers)
			free(dconf->pconf[i].headers);
		free(dconf->pconf[i]);
	}
	free(dconf->pconf);
}

/*
typedef struct process_config {
	unsigned long int process;
	const char *url;
	struct {
		int location : 1;
	};
	unsigned long int delay;
	unsigned long int max_delay;
	unsigned long int max_time;
	const char **headers;
	size_t num_headers;
} *process_config;

typedef struct daemon_config {
	unsigned long int processes;
	process_config pconf;
	unsigned long int configs;
} *daemon_config;
*/

daemon_config parse_args(int argc, char *argv[])
{
	// data?
	daemon_config dconf;
	process_config pconf;
	int opt;
	char *sp;
	void *tmp;

	// data
	unsigned long int process = 0;
	const char *optstring = "hp:P:d:D:H:Lm:";
	const struct option longopts[] = {
		{ "help", no_argument, NULL, "h" },
		{ "processes", required_argument, NULL, "p" },
		{ "configure", required_argument, NULL, "P" },
		{ "delay", required_argument, NULL, "d" },
		{ "max-delay", required_argument, NULL, "D" },
		{ "header", required_argument, NULL, "H" },
		{ "location", no_argument, NULL, "L" },
		{ "max-time", required_argument, NULL, "m" },
		{ 0, 0, NULL, 0 }
	};

	// text
	if (argv == NULL)
		abort();

	if ((dconf = malloc(sizeof(struct daemon_config))) == NULL) {
		errno = ENOMEM;
		ERROR("cannot allocate memory for daemon configuration\n");
		return NULL;
	}

	memset(dconf, 0, sizeof(struct daemon_config));

	/* first conf is the global conf */
	if ((pconf = new_config(dconf, process)) == NULL)
		goto die;

	do {
		while ((opt = getopt_long(argc, argv, optstring, longopts,
					  NULL)) != -1) {
			switch (opt) {
			/* Global Options: */
			case 'h':
				usage();
				exit(0);
				break;
			case 'p':
				dconf->processes = strtoul(optarg, NULL, 10);
				break;
			case 'P':
				process = strtoul(optarg, NULL, 10);
				if ((pconf = new_config(dconf, process)) ==
				    NULL)
					goto die;
				break;
			/* Process Options: */
			case 'd':
				pconf->delay = strtoul(optarg, NULL);
				break;
			case 'D':
				pconf->max_delay = strtoul(optarg, NULL);
				break;
			case 'H':
				if (add_header(pconf, optarg))
					goto die;
				break;
			case 'L':
				pconf->location = 1;
				break;
			case 'm':
				pconf->max_time = strtoul(optarg, NULL);
				break;
			case '?':
				__attribute__((fallthrough));
			default:
				if ((sp = strchr(optstring, optopt)) != NULL) {
					ERROR("option '%c' requires an argument\n",
					      optopt);
				} else {
					ERROR("unknown option '%c'\n", optopt);
				}
				usage();
				exit(2);
			}
		}

		/* positional arguments */
		while (optind < argc) {
			if (argv[optind][0] == '-')
				break;

			if (pconf->url == NULL) {
				pconf->url = argv[optind++];
			} else {
				ERROR("no candidate for positional argument '%s'",
				      optarg);
				usage();
				goto die;
			}
		}
	} while (optind < argc);

	/* validate */

	if (dconf->processes == 0) {
		ERROR("must have at least one process\n");
		usage();
		goto die;
	}

	/* make sure all processes get at least one URL.
	 * either the global config has a URL defined or every process has a URL defined.
	 */
	if (dconf->pconf->url == NULL) {
		for (process = 1; process < dconf->processes; process++) {
			pconf = get_config(dconf, process);
			if ((pconf == NULL) || (pconf->url == NULL)) {
				ERROR("no URL found for process %ld\n",
				      process);
				usage();
				goto die;
			}
		}
	}

	return dconf;
die:
	free_args(dconf);
	exit(2);
}

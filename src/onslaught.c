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

#include "includes.h"

void usage()
{
	puts("\
Usage: onslaught [options...] URL\n\
\n\
 Options:\n\
  -h, --help                 show this message\n\
  -H, --header HEADER/@FILE  pass custom header(s) to server\n\
  -m, --max-time FLOAT       maximum time in seconds allowed for transfer.\n\
                             INF or INFINITY disables timeout.\n\
  -p, --processes NUM        number of parallel processes to run\n\
");
}

#define MAX_PROCESSES 100

struct arg_data {
	char *url;
	struct {
		char ** data;
		size_t length;
	} headers;
	float maxtime;
	unsigned long int processes;
};

void parse_args(int argc, char *argv[], struct arg_data *argd)
{
	// data?
	int opt;
	char *sp;
	void *tmp;
	
	// data
	const char *optstring = "hH:m:";
	const struct option longopts[] = {
		{"help", no_argument, NULL, "h"},
		{"header", required_argument, NULL, "H"},
		{"max-time", required_argument, NULL, "m"},
		{"processes", required_argument, NULL, "p"},
		{0, 0, NULL, 0}
	};

	//text
	if ((argv == NULL) || (argd == NULL))
		abort();

	memset(argd, 0, sizeof(struct arg_data));

	/* initialize */
	argd->maxtime = 5.0f;
	argd->processes = 5;

	/* override */
	while ((opt = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) {
		switch (opt) {
		case 'h':
			usage();
			exit(0);
			break;
		case 'H':
			if ((tmp = reallocarray(argd->headers.data, argd->headers.length + 1, sizeof(char *))) == NULL) {
				if (argd->headers.data == NULL)
					free(argd->headers.data);
				ERROR("no memory to allocate for header array\n");
				exit(ENOMEM);
			}
			tmp[(argd->headers.length)++] = optarg;
			argd->headers.data = tmp;
			break;
		case 'm':
			argd->maxtime = strtof(optarg, NULL);
			break;
		case 'p':
			argd->processes = strtoul(optarg, NULL, 10);
			break;
		case '?':
			__attribute__((fallthrough));
		default:
			if ((sp = strchr(optstring, optopt)) != NULL) {
				ERROR("option '%c' requires an argument\n", optopt);
			} else {
				ERROR("unknown option '%c'\n", optopt);
			}
			usage();
			exit(2);
		}
	}

	/* positional arguments */
	while (optind < argc) {
		if (argd->url == NULL) {
			argd->url = argv[optind++];
		} else {
			ERROR("ignoring garbage at end of line\n");
			break;
		}
	}

	/* validate */
	if (argd->url == NULL) {
		ERROR("no url specified\n");
		usage();
		exit(2);
	}

	if (argd->maxtime < 0.0f) {
		ERROR("max time must be positive\n");
		usage();
		exit(2);
	}

	if (argd->processes == 0) {
		ERROR("must have at least one process\n");
		usage();
		exit(2);
	}
}

/* unique per child: */
struct _args {
	char *body; /* payload */
	krossock_t connection; /* connection */
	/* You Know What To Do. -K */
};

int probe(void *data)
{
	struct _args *args = (struct probe_args *)data;
	
	// TODO: send request body
	// TODO: get response
	// TODO: collect statistics about what just happened
}

void probe_term(int signo, siginfo_t *info, void *context)
{
	info->si_pid; // pid of the child that terminated
	// TODO: get _args for this pid
	// TODO: close connection
	// TODO: free body
	// TODO: get whatever stats I decided to throw in there
	// TODO: may need to send myself a signal here...
}

void main_term(int signo)
{
	// TODO: run through array of child pids and kill them off. free'ing and such should be handled by probe_term.
	// kill();
	// TODO: may need to wait for probe_term to finish...
}

int main(int argc, char *argv[])
{
	void *childstack;
	struct arg_data argd;

	struct sigaction main_act;
	struct sigaction probe_act;

	main_act.sa_handler=main_term;
	sigemptyset(main_act.sa_mask);
	main_act.sa_flags=0;

	probe_act.sa_sigaction=probe_term;
	sigemptyset(probe_act.sa_mask);
	probe_act.sa_flags=SA_SIGINFO;

	// TODO: handle sigterm or whatever and send death threats to the children so they stop
	if (sigaction(SIGTERM, &main_act, NULL) < 0 ) {
		ERROR("failed to register SIGTERM handler\n");
		abort();
	}
	// TODO: handle child death
	if (sigaction(SIGCHLD, &probe_act, NULL) < 0) {
		ERROR("failed to register SIGCHLD handler\n");
		abort();
	}
	
	parse_args(argc, argv, &args);

	// TODO: get path from url
	
	// TODO: build request body

	// TODO: do this for as many children you wanna start...
	// TODO: make copy of body
	// TODO: connect to host
	// TODO: set (or disable) connection timeout
	// TODO: build probe_args
	// TODO: spin up child
	// clone3();
	// TODO: stick probe_args and child pid in an associative array so we can clean up later and collect stats or whatever

	return 0;
}

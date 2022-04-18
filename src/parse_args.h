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

#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

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
} * process_config;

typedef struct daemon_config {
	unsigned long int processes;
	process_config pconf;
	unsigned long int configs;
} * daemon_config;

#define MAX_CONFIGURATIONS 100

daemon_config parse_args(int argc, char *argv[]);
void free_args(daemon_config dconf);
void usage();

#endif

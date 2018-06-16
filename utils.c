/**
 * @file    utils.c
 * @brief   mbed Edge Utilities Implementation
 * @author  Doug Anson
 * @version 1.0
 * @see     
 *
 * Copyright (c) 2018
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

// shutdown handler in main.cpp
extern void shutdown_handler(int signo);

/**
 * \brief Set up the signal handler for catching signals from OS.
 * This example signal handler setup catches SIGTERM and SIGINT for shutting down
 * the protocol translator client gracefully.
 */
bool setup_signals(void)
{
    struct sigaction sa = { .sa_handler = shutdown_handler, };
    struct sigaction sa_pipe = { .sa_handler = SIG_IGN, };
    int ret_val;

    if (sigemptyset(&sa.sa_mask) != 0) {
        return false;
    }
    if (sigaction(SIGTERM, &sa, NULL) != 0) {
        return false;
    }
    if (sigaction(SIGINT, &sa, NULL) != 0) {
        return false;
    }
    ret_val = sigaction(SIGPIPE, &sa_pipe, NULL);
    if (ret_val != 0) {
        printf("setup_signals: sigaction with SIGPIPE returned error=(%d) errno=(%d) strerror=(%s)\n",
                ret_val,
                errno,
                strerror(errno));
    }
    return true;
}

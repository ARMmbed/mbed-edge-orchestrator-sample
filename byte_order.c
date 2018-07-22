/*
 * ----------------------------------------------------------------------------
 * Copyright 2018 ARM Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
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
 * ----------------------------------------------------------------------------
 */

#include <arpa/inet.h>
#include <inttypes.h>
#include <string.h>
#include <endian.h>
#include <stdio.h>

void display_conversion(char *prefix, const uint8_t *buffer, int buffer_len, long value) {
    printf("%s(%d): ",prefix,buffer_len);
    for(int i=0;i<buffer_len;++i) {
      printf("0x%x ",*(buffer+i));
    }
    printf("VALUE: %ld\n",value);
}

void convert_value_to_host_order_long(const uint8_t *buffer, long *host_value)
{
    #if __x86_64__
      long net_value = -1;
      memcpy(&net_value, buffer, sizeof(net_value));
      *host_value = be64toh(net_value);
    #else
      uint64_t net_value = -1;
      memcpy(&net_value, buffer, sizeof(net_value)); 	
      *host_value = (long)be64toh(net_value);
    #endif

    // DEBUG
    display_conversion((char *)"ntohl(W)",buffer,sizeof(net_value),*host_value);
}

void convert_long_value_to_network_byte_order(long host_value, uint8_t *buffer) 
{
    #if __x86_64__
      long net_value = htobe64(host_value);
    #else
      long long net_value = (long long)htobe64((uint64_t)host_value);
    #endif
    memcpy(buffer, &net_value, sizeof(net_value));

    // DEBUG
    display_conversion((char *)"htonl(R)",buffer,sizeof(net_value),host_value);
}

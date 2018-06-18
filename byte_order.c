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

void convert_value_to_host_order_long(const uint8_t *buffer, long *host_value)
{
    long net_value = -1;
    memcpy(&net_value, buffer, sizeof(long));
    *host_value = be64toh(net_value);
}

void convert_long_value_to_network_byte_order(long host_value, uint8_t *buffer) 
{
    long net_value = htobe64(host_value);
    memcpy(buffer, &net_value, sizeof(long));
}

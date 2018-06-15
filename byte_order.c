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

void convert_value_to_host_order_integer(uint8_t *value, int *int_value)
{
    uint32_t temp_value;
    memcpy(&temp_value, value, sizeof(int));
    temp_value = ntohl(temp_value);
    memcpy(int_value, &temp_value, sizeof(int));
}

void convert_int_value_to_network_byte_order(int value, uint8_t *buffer) 
{
    uint32_t temp_value;
    memcpy(&temp_value, &value, sizeof(int));
    temp_value = htonl(temp_value);
    memcpy(buffer, &temp_value, sizeof(int));
}

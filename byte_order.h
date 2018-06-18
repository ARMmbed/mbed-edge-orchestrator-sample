/**
 * @file    byte_order.h
 * @brief   mbed Edge byte order utils
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

#ifndef __BYTE_ORDER_UTILS_H__
#define __BYTE_ORDER_UTILS_H__

extern "C" void convert_value_to_host_order_long(const uint8_t *buffer, long *host_value);
extern "C" void convert_long_value_to_network_byte_order(long host_value, uint8_t *buffer);

#endif // __BYTE_ORDER_UTILS_H__

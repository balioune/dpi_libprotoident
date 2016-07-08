/* 
 * This file is part of libprotoident
 *
 * Copyright (c) 2011-2015 The University of Waikato, Hamilton, New Zealand.
 * Author: Shane Alcock
 *
 * With contributions from:
 *      Aaron Murrihy
 *      Donald Neal
 *
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND 
 * research group. For further information please see http://www.wand.net.nz/
 *
 * libprotoident is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * libprotoident is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libprotoident; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

#include <string.h>

#include "libprotoident.h"
#include "proto_manager.h"
#include "proto_common.h"

/* This appears to be associated with BitTorrent somehow - there are occasional
 * DHT-style bencoding dictionaries in these flows, but cannot find anything
 * to confirm this :/
 */

static inline bool payload_check(uint32_t a, uint32_t b) {

	/* In most cases, the last two bytes must match but I've also seen
	 * instances where one of the bytes is one greater than the other
	 *
	 * No idea what is actually going on though :/ */

	if (a == b)
		return true;
	if (a - 1 == b)
		return true;
	if (a + 1 == b)
		return true;
	return false;

}

static inline bool match_2102_response(uint32_t payload, uint32_t other,
		uint32_t len) {

	if (len == 0)
		return true;

	if (!payload_check(htonl(payload) & 0x0000ffff, 
				htonl(other) & 0x0000ffff))
		return false;

	if (MATCH(payload, 0x21, 0x02, ANY, ANY) && len == 30)
		return true;
	if (MATCH(payload, 0x21, 0x00, ANY, ANY) && len == 20)
		return true;
	if (MATCH(payload, 0x21, 0x01, ANY, ANY) && len == 26)
		return true;
	

	return false;


}

static inline bool match_3102_response(uint32_t payload, uint32_t other,
		uint32_t len) {

	if (len == 0)
		return true;
	if (len != 30 && len != 20)
		return false;

	if (!payload_check(htonl(payload) & 0x0000ffff, 
				htonl(other) & 0x0000ffff))
		return false;
	
	if (MATCH(payload, 0x31, 0x02, ANY, ANY) && len == 30)
		return true;
	if (MATCH(payload, 0x31, 0x00, ANY, ANY) && len == 20)
		return true;
	

	return false;


}

static inline bool match_4102_response(uint32_t payload, uint32_t other,
		uint32_t len) {

	if (len == 0)
		return true;
	if (len != 33)
		return false;

	if (!payload_check(htonl(payload) & 0x0000ffff, 
				htonl(other) & 0x0000ffff))
		return false;

        if (!MATCH(payload, 0x41, 0x02, ANY, ANY))
                return false;

	return true;	

}

static inline bool match_dict(uint32_t payload, uint32_t len) {

	/* Check for bencoded dictionary */
	if (MATCH(payload, 'd', '1', ':', 'r'))
		return true;
	if (MATCH(payload, 'd', '1', ':', 'a'))
		return true;
	if (MATCH(payload, 'd', '1', ':', 'e'))
		return true;
	if (MATCH(payload, 'd', '1', ANY, ':'))
		return true;

	return false;

}

static inline bool match_0100_request(uint32_t payload, uint32_t len) {

	if (MATCH(payload, 0x01, 0x00, ANY, ANY) && len > 500)
		return true;
	return false;

}

static inline bool match_2102_request(uint32_t payload, uint32_t len) {

	if (MATCH(payload, 0x21, 0x02, ANY, ANY) && len == 30)
		return true;
	if (MATCH(payload, 0x21, 0x00, ANY, ANY) && len == 20)
		return true;
	if (MATCH(payload, 0x21, 0x01, ANY, ANY) && len == 26)
		return true;
	return false;

}
static inline bool match_3102_request(uint32_t payload, uint32_t len) {

	if (MATCH(payload, 0x31, 0x02, ANY, ANY) && len == 30)
		return true;
	if (MATCH(payload, 0x31, 0x00, ANY, ANY) && len == 20)
		return true;
	return false;

}
static inline bool match_4102_request(uint32_t payload, uint32_t len) {

	if (MATCH(payload, 0x41, 0x02, ANY, ANY) && len == 30)
		return true;
	if (MATCH(payload, 0x41, 0x00, ANY, ANY) && len == 20)
		return true;
	return false;

}

static inline bool match_mystery_bt_udp(lpi_data_t *data, lpi_module_t *mod UNUSED) {
	
	
	if (match_4102_request(data->payload[0], data->payload_len[0])) {
		if (match_2102_response(data->payload[1], data->payload[0], 
				data->payload_len[1]))
			return true;
		if (match_4102_response(data->payload[1], data->payload[0], 
				data->payload_len[1]))
			return true;
		if (match_dict(data->payload[1], data->payload_len[1]))
			return true;
	}

	if (match_4102_request(data->payload[1], data->payload_len[1])) {
		if (match_2102_response(data->payload[0], data->payload[1], 
				data->payload_len[0]))
			return true;
		if (match_4102_response(data->payload[0], data->payload[1], 
				data->payload_len[0]))
			return true;
		if (match_dict(data->payload[0], data->payload_len[0]))
			return true;
	}

	if (match_2102_request(data->payload[0], data->payload_len[0])) {
		if (match_3102_response(data->payload[1], data->payload[0], 
				data->payload_len[1]))
			return true;
	}

	if (match_2102_request(data->payload[1], data->payload_len[1])) {
		if (match_3102_response(data->payload[0], data->payload[1], 
				data->payload_len[0]))
			return true;
	}
	
	if (match_0100_request(data->payload[0], data->payload_len[0])) {
		if (match_3102_response(data->payload[1], data->payload[0], 
				data->payload_len[1]))
			return true;
		if (match_2102_response(data->payload[1], data->payload[0], 
				data->payload_len[1]))
			return true;
	}

	if (match_0100_request(data->payload[1], data->payload_len[1])) {
		if (match_3102_response(data->payload[0], data->payload[1], 
				data->payload_len[0]))
			return true;
		if (match_2102_response(data->payload[0], data->payload[1], 
				data->payload_len[0]))
			return true;
	}

	if (match_3102_request(data->payload[0], data->payload_len[0])) {
		if (data->payload_len[1] == 0)
			return true;
	}

	if (match_3102_request(data->payload[1], data->payload_len[1])) {
		if (data->payload_len[0] == 0)
			return true;
	}
	

	
	if (match_dict(data->payload[0], data->payload_len[0])) {
		if (match_2102_response(data->payload[1], data->payload[0], 
				data->payload_len[1]))
			return true;
	}	

	if (match_dict(data->payload[1], data->payload_len[1])) {
		if (match_2102_response(data->payload[0], data->payload[1], 
				data->payload_len[0]))
			return true;
	}	

	return false;
}

static lpi_module_t lpi_mystery_bt_udp= {
	LPI_PROTO_UDP_MYSTERY_BT,
	LPI_CATEGORY_P2P,
	"Mystery_BitTorrent_UDP",
	9,	/* Need to be higher than Skype or Gnutella */
	match_mystery_bt_udp
};

void register_mystery_bt_udp(LPIModuleMap *mod_map) {
	register_protocol(&lpi_mystery_bt_udp, mod_map);
}


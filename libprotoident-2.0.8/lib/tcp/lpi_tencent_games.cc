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

/* This traffic has been observed with BNS China, an MMO operated by
 * Tencent. I can't be sure whether this is exclusive to BNS or is used
 * by all Tencent games yet, so I'll use the generic name "TencentGames"
 * rather than assuming it is only BNS.
 */

static inline bool match_tcg_req(uint32_t payload, uint32_t len) {

        uint32_t hdrlen = (ntohl(payload) & 0xffff);

        if (hdrlen != len)
                return false;

        if (MATCH(payload, 0xd8, 0x5c, ANY, ANY))
                return true;
        return false;
}

static inline bool match_tcg_reply(uint32_t payload, uint32_t len) {

        uint32_t hdrlen = (ntohl(payload) & 0xffff);

        if (hdrlen != len)
                return false;

        if (MATCH(payload, 0xd8, 0x5d, ANY, ANY))
                return true;
        return false;
}

static inline bool match_tencent_games(lpi_data_t *data, lpi_module_t *mod UNUSED) {

        if (match_tcg_req(data->payload[0], data->payload_len[0])) {
                if (match_tcg_reply(data->payload[1], data->payload_len[1]))
                        return true;
        }

        if (match_tcg_req(data->payload[1], data->payload_len[1])) {
                if (match_tcg_reply(data->payload[0], data->payload_len[0]))
                        return true;
        }


	return false;
}

static lpi_module_t lpi_tencent_games = {
	LPI_PROTO_TENCENT_GAMES,
	LPI_CATEGORY_GAMING,
	"TencentGames",
	5,
	match_tencent_games
};

void register_tencent_games(LPIModuleMap *mod_map) {
	register_protocol(&lpi_tencent_games, mod_map);
}



/*
 * Copyright (C) 2007-2011 Siemens AG
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*******************************************************************
 *
 * @author Sebastian.Kaebisch.EXT@siemens.com, jharper@anl.gov		 *
 * @contact jharper@anl.gov
 *
 *
 ********************************************************************/

/*Add ANL Copyright*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef V2GTP_H_
#define V2GTP_H_

#include <stdint.h>

/* generic V2GTP header length */
#define V2GTP_HEADER_LENGTH 8

/* define V2GTP Version */
#define V2GTP_VERSION 0x01
#define V2GTP_VERSION_INV 0xFE

/* define V2GTP payload types*/
#define V2GTP_EXI_TYPE 0x8001  //-2 or DIN
#define V2GTP_COMMON_MESSAGES_20 0x8002
#define V2GTP_AC_MESSAGES_20 0x8003
#define V2GTP_DC_MESSAGES_20 0x8004
#define V2GTP_ACDP_MESSAGES_20 0x8005
#define V2GTP_WPT_MESSAGES_20 0x8006
#define V2GTP_SCHEDULE_RENOGOTIATION 0x8101

#define V2GTP_SDP_REQUEST_TYPE 0x9000
#define V2GTP_SDP_RESPONSE_TYPE 0x9001



int write_v2gtpHeader(uint8_t* outStream, uint32_t* outStreamLength, uint16_t payloadType);

int read_v2gtpHeader(uint8_t* inStream, uint32_t* payloadLength);

#endif /* V2GTP_H_ */

#ifdef __cplusplus
}
#endif


/*
 * Copyright (C) 2007-2012 Siemens AG
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

/*
 * This file implements the v2gtp header writer and reader.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "v2gtp.h"
#include "evse_server.h"


int write_v2gtpHeader(uint8_t* outStream, uint32_t* outStreamLength, uint16_t payloadType)
{

	/* write v2gtp version number 1=byte */
	outStream[0]=V2GTP_VERSION;

	/* write inverse v2gtp version */
	outStream[1]=V2GTP_VERSION_INV;


	/* write payload type */
	outStream[3] = (uint8_t)(payloadType & 0xFF);
	outStream[2] = (uint8_t)(payloadType >> 8 & 0xFF);

	/* write payload length */
	outStream[7] = (uint8_t)(*outStreamLength & 0xFF);
	outStream[6] = (uint8_t)(*outStreamLength>>8 & 0xFF);
	outStream[5] = (uint8_t)(*outStreamLength>>16 & 0xFF);
	outStream[4] = (uint8_t)(*outStreamLength>>24 & 0xFF);

	/* here, the outStream length have to be resized by the v2gtp offset*/
	*outStreamLength += V2GTP_HEADER_LENGTH;

	return 0;
}

/*returns: payloadType on success and -1 on error
 *
 */
int read_v2gtpHeader(uint8_t* inStream, uint32_t* inStreamLength)
{
	uint16_t payloadType=0;
	uint32_t payloadLength;

	/* check, if we support this v2gtp version */
	if(inStream[0]!=V2GTP_VERSION || inStream[1]!=V2GTP_VERSION_INV) {
		logtime();
		printf("Error: Invalid V2GTP Version\n");
		printf("V2GTP Version: %d\n", inStream[0]);
		printf("V2GTP Version Inv: %d\n", inStream[1]);
		return -1;
	}



	/* check, if we support this payload type*/
	payloadType = inStream[2];
	payloadType = (payloadType << 8 | inStream[3]);

	//debug
	/*for(i=0; i<*payloadLength; i++) {
			printf("%X ",inStream[i]);
		}
		printf("\n");*/
	//printf("V2GTP Type: %X\n", payloadType);
	switch(payloadType) {

	case V2GTP_EXI_TYPE:
	//ISO1, ISO2 and DIN utilize this payloadType
		break;
/*
	case V2GTP_COMMON_MESSAGES_20:
		break;

	case V2GTP_AC_MESSAGES_20:
		break;

	case V2GTP_DC_MESSAGES_20:
		break;

	case V2GTP_ACDP_MESSAGES_20:
		break;

	case V2GTP_WPT_MESSAGES_20:
		break;

	case V2GTP_SCHEDULE_RENOGOTIATION:
		break;
*/
	default : /* Optional */
		logtime();
		printf("Error: Invalid V2GTP Type: %d\n", payloadType);
		return -1;
	}

	/* determine payload length*/
	payloadLength = inStream[4];
	payloadLength = (payloadLength << 8 | inStream[5]);
	payloadLength = (payloadLength << 16 | inStream[6]);
	payloadLength = (payloadLength << 24 | inStream[7]);

	if((payloadLength+V2GTP_HEADER_LENGTH)!=*inStreamLength) {
		logtime();
		printf("Error: V2GTP Header Length does not match Payload Length\n");
		printf("payloadLength+V2GTP_HEADER_LENGTH = %d\n", (payloadLength+V2GTP_HEADER_LENGTH));
		printf("Payload Length = %d\n", (*inStreamLength));

		return -1;
	}


	return payloadType;
}


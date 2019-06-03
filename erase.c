/* Copyright (c) 2018 Atmark Techno, Inc.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
#include "scp.h"
#include "smCom.h"
#include "sm_printf.h"
#include "sm_apdu.h"
#include "apduComm.h"
#include "configCmd.h"
#include "axCliUtil.h"

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "ax_api.h"
#include "ax_util.h"
#include "sm_errors.h"
#include "smComSCI2C.h"
#include "i2c_a7.h"

#if !defined(TGT_A70CI) && !defined(TGT_A70CM) && !defined(NO_SECURE_CHANNEL_SUPPORT)
#define SECURE_CHANNEL_SUPPORTED
#endif

#ifdef SECURE_CHANNEL_SUPPORTED
#include "ax_util.h"
#include "HostCryptoAPI.h"
#endif // SECURE_CHANNEL_SUPPORTED

#include <assert.h>

void usage(){
	printf("USAGE : \n./erase [cnt|pair|pub|sym] -x <index>\n");
	return;
}

static U16 SST_EraseGeneric(U8 ins, SST_Index_t index)
{
	U16 rv;
	apdu_t apdu;
	apdu_t * pApdu = (apdu_t *) &apdu;

	pApdu->cla   = AX_CLA;
	pApdu->ins   = ins;
	pApdu->p1    = index;
	pApdu->p2    = 0x00;

	AllocateAPDUBuffer(pApdu);
	SetApduHeader(pApdu, USE_STANDARD_APDU_LEN);

	rv = smCom_Transceive(pApdu);
	if (rv == SMCOM_OK)
	{
		// No response data expected
		rv = CheckNoResponseData(pApdu);
	}

	FreeAPDUBuffer(pApdu);
	return rv;
}

int main(int argc, char **argv)
{
	int index = 0;
	int argCurrent = 1;
	U16 sw = SW_OK;

	if(4 != argc){
		usage();
		printf("argment error\n");
		return -1;
	}

	index = atoi(argv[3]);

	int ret = 0;
	U8 atr[64];
	U16 atrLen = sizeof(atr);
	ret = axI2CInit();
	if(I2C_OK != ret){
		return -1; 
	}
	sw = smComSCI2C_Open(ESTABLISH_SCI2C, 0x00, atr, &atrLen);
	if(SW_OK != sw){
		return sw; 
	}

	if (strcmp(argv[argCurrent], "pair") == 0) {
		sw = SST_EraseGeneric(A71CH_INS_ERASE_ECC_KEYPAIR, index);
	}
	else if (strcmp(argv[argCurrent], "pub") == 0) {
		sw = SST_EraseGeneric(A71CH_INS_ERASE_ECC_PUBLIC_KEY, index);
	}
	else if (strcmp(argv[argCurrent], "sym") == 0) {
		sw = SST_EraseGeneric(A71CH_INS_ERASE_SYM_KEY, index);
	}
	else if (strcmp(argv[argCurrent], "cnt") == 0) {
		sw = SST_EraseGeneric(A71CH_INS_ERASE_COUNTERS, index);
	}
	else {
		sw = A7X_CONFIG_STATUS_API_ERROR;
	}

	if(sw != SW_OK){
		printf("error : sw=0x%x\n",sw);
		usage();
	}
	return sw;
}

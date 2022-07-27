//
// Copyright (c) 2019-2022 yanggaofeng
//
#include <yangavutil/video/YangCNalu.h>
#include <yangutil/yangtype.h>
#include <yangutil/yangavtype_h265.h>
#include <memory.h>
#include <yangutil/sys/YangLog.h>
#include <yangutil/sys/YangEndian.h>
#include <yangutil/sys/YangAmf.h>
#include <yangavutil/video/YangMeta.h>

int32_t yang_getH264KeyframeNalu( YangFrame *videoFrame) {
	uint8_t *tmp = NULL; //videoFrame->payload;
	int len = videoFrame->nb;
	uint32_t naluLen = 0;
	int pos = 0;
	int32_t err = 1;
	while (pos < len) {
		tmp = videoFrame->payload + pos;
		if ((*(tmp + 4) & kNalTypeMask) == YangAvcNaluTypeIDR) {
			videoFrame->payload = tmp;
			videoFrame->nb = len - pos;
			err = Yang_Ok;
			break;
		}
		naluLen = yang_get_be32(tmp);
		if (naluLen > len) {
			break;
		}
		pos += naluLen + 4;
	}
	return err;
}

bool yang_hasH264Pframe(uint8_t *p) {
	if ((*(p + 4) & kNalTypeMask) == YangAvcNaluTypeNonIDR)
		return true;
	return false;
}

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

static int32_t getNextNaluLength(const char* nalus, uint32_t nalusLength, uint32_t* pStart, uint32_t* pNaluLength)
{
    int32_t retStatus = 0;
    uint32_t zeroCount = 0, offset;
    _Bool naluFound = FALSE;
    char* pCurrent = NULL;

    if (nalus == NULL || pStart == NULL || pNaluLength == NULL) {
        printf("Warning: Failed to getNextNaluLength\n");
		return -1;
	}

    // Annex-B Nalu will have 0x00000001 or 0x000001 start code, at most 4 bytes
    for (offset = 0; offset < 4 && offset < nalusLength && nalus[offset] == 0; offset++)
        ;

    if (offset >= nalusLength - 1 || offset >= 4 || offset < 2 || nalus[offset] != 1) {
		printf("Warning: getNextNaluLength nalu error\n");
		return -2;
	}

	// Don
	retStatus = nalus[offset+1];

    *pStart = ++offset;
    pCurrent = nalus + offset;

    /* Not doing validation on number of consecutive zeros being less than 4 because some device can produce
     * data with trailing zeros. */
    while (offset < nalusLength) {
        if (*pCurrent == 0) {
            /* Maybe next byte is 1 */
            offset++;
            pCurrent++;

        } else if (*pCurrent == 1) {
            if (*(pCurrent - 1) == 0 && *(pCurrent - 2) == 0) {
                zeroCount = *(pCurrent - 3) == 0 ? 3 : 2;
                naluFound = TRUE;
                break;
            }

            /* The jump is always 3 because of the 1 previously matched.
             * All the 0's must be after this '1' matched at offset */
            offset += 3;
            pCurrent += 3;
        } else {
            /* Can jump 3 bytes forward */
            offset += 3;
            pCurrent += 3;
        }
    }
    *pNaluLength = MIN(offset, nalusLength) - *pStart - (naluFound ? zeroCount : 0);

    // As we might hit error often in a "bad" frame scenario, we can't use CHK_LOG_ERR as it will be too frequent

    return retStatus;
}

int32_t player_parseH264Nalu( YangFrame *videoFrame,  YangH264NaluData *pnalu) {
	uint8_t *tmp = NULL; //videoFrame->payload;
	uint32_t len = videoFrame->nb;
	uint32_t naluLen = 0;
	int32_t pos = 0;
	int32_t err = 1;
	pnalu->spsppsPos = -1;
	pnalu->keyframePos = -1;
	while (pos < len) {
		tmp = videoFrame->payload + pos;
		if ((*(tmp + 4) & kNalTypeMask) == YangAvcNaluTypeIDR) {
			pnalu->keyframePos = pos;
			break;
		}
		if ((*(tmp + 4) & kNalTypeMask) == YangAvcNaluTypeSPS) {
			pnalu->spsppsPos = pos;
		}
		naluLen = yang_get_be32(tmp);
		if (naluLen > len) {
			break;
		}
		pos += naluLen + 4;
	}
	return err;
}

int32_t yang_parseH264Nalu( YangFrame *videoFrame,  YangH264NaluData *pnalu) {
	uint32_t remainNalusLength = videoFrame->nb;
	char* curPtrInNalus = videoFrame->payload;
	char* originPosition = videoFrame->payload;
	uint32_t startIndex = 0;
	uint32_t nextNaluLength = 0;

    do {
        int32_t ret = getNextNaluLength(curPtrInNalus, remainNalusLength, &startIndex, &nextNaluLength);
		if (ret < 0) {
			break;
		}

        curPtrInNalus += startIndex;
        remainNalusLength -= startIndex;

        if (remainNalusLength == 0) {
			return -1;
		}
		// SPS
		if (ret == 0x67) {
			pnalu->spsppsPos = curPtrInNalus - originPosition;
		}
		// IDR
		else if (ret == 0x65) {
			pnalu->keyframePos = curPtrInNalus - originPosition;
			return 0;
		}

        remainNalusLength -= nextNaluLength;
        curPtrInNalus += nextNaluLength;
    } while (remainNalusLength != 0);
	return -2;
}


int32_t yang_getNalupos(uint8_t* data,int plen){
	uint8_t* p=NULL;
	int len=plen-4;
	for(int i=0;i<len;i++){
		p=data+i;
		if((*p)==0x00&&(*(p+1))==0x00&&(*(p+2))==0x00&&(*(p+3))==0x01){
			return i+4;
		}
	}
	return -1;
}
int32_t yang_parseH264Nalu2( YangFrame *videoFrame,  YangH264NaluData2 *pnalu) {
	uint8_t *tmp = NULL;
	uint32_t len = videoFrame->nb;
	//uint32_t naluLen = 0;
	int32_t pos = 0;
	int32_t err = 1;
	int32_t tmpPos=0;
	int32_t tmpLen=len;
	pnalu->spsPos = -1;
	pnalu->ppsPos = -1;
	pnalu->keyframePos = -1;
	pnalu->spsLen=0;
	pnalu->ppsLen=0;
	int32_t preType=-1;
	while (pos < len) {

		tmp = videoFrame->payload + pos;
		tmpPos=yang_getNalupos(tmp,tmpLen);
		if(tmpPos==-1) return 1;

		if(preType==0){
			pnalu->spsLen=tmpPos-4;
			preType=-1;
		}
		if(preType==1){
				pnalu->ppsLen=tmpPos-4;
				preType=-1;
			}
		tmp+=tmpPos;

		if ((*tmp & kNalTypeMask) == YangAvcNaluTypeIDR) {
			pnalu->keyframePos =tmp-videoFrame->payload;
			preType=2;
			return Yang_Ok;
		}
		if ((*tmp & kNalTypeMask) == YangAvcNaluTypeSPS) {
			pnalu->spsPos = tmp-videoFrame->payload;
			preType=0;
		}
		if ((*tmp & kNalTypeMask) == YangAvcNaluTypePPS) {
					pnalu->ppsPos = tmp-videoFrame->payload;
					preType=1;
		}
		tmpLen-=tmpPos;
		pos+=tmpPos;



	}
	return err;
}
int32_t yang_getH264SpsppseNalu( YangFrame *videoFrame, uint8_t *pnaludata) {
	if (!videoFrame || !pnaludata || !videoFrame->payload)
		return yang_error_wrap(1, "getSpsppseNalu is null");
	int32_t spsLen = yang_get_be32(pnaludata);
	int32_t ppsLen = yang_get_be32(pnaludata + 4 + spsLen);
	if (spsLen > videoFrame->nb || ppsLen > videoFrame->nb)
		return 1;
	uint8_t *sps = pnaludata + 4;
	uint8_t *pps = pnaludata + 4 + spsLen + 4;

	 YangSample sps_sample,pps_sample;
    sps_sample.bytes=(char*)sps;
	sps_sample.nb=spsLen;

    pps_sample.bytes=(char*)pps;
	pps_sample.nb=ppsLen;

	yang_getConfig_Meta_H264(&sps_sample,&pps_sample,videoFrame->payload,&videoFrame->nb);

	return Yang_Ok;
}




int32_t yang_parseH265Nalu( YangFrame *videoFrame,  YangH264NaluData *pnalu) {
	uint8_t *tmp = NULL; //videoFrame->payload;
	uint32_t len = videoFrame->nb;
	uint32_t naluLen = 0;
	int32_t pos = 0;
	int32_t err = 1;
	pnalu->spsppsPos = -1;
	pnalu->keyframePos = -1;
	int32_t v=0;
	while (pos < len) {
		tmp = videoFrame->payload + pos;
		v=YANG_HEVC_NALU_TYPE(*(tmp + 4));

		if (v >= YANG_NAL_UNIT_CODED_SLICE_BLA&&v<= YANG_NAL_UNIT_CODED_SLICE_CRA) {
			pnalu->keyframePos = pos;
			break;
		}
		if (v== YANG_NAL_UNIT_VPS) {
			pnalu->spsppsPos = pos;
		}
		naluLen = yang_get_be32(tmp);
		if (naluLen > len) {
			break;
		}
		pos += naluLen + 4;
	}
	return err;
}
int32_t yang_getH265SpsppseNalu( YangFrame *videoFrame, uint8_t *pnaludata) {
	if (!videoFrame || !pnaludata || !videoFrame->payload)
		return yang_error_wrap(1, "getSpsppseNalu is null");
	 YangSample vps_sample,sps_sample,pps_sample;
	int32_t vpsLen = yang_get_be32(pnaludata);
	int32_t spsLen = yang_get_be32(pnaludata+ 4 + vpsLen);
	int32_t ppsLen = yang_get_be32(pnaludata + 4 + vpsLen+4+spsLen);
	if (spsLen > videoFrame->nb || ppsLen > videoFrame->nb)
		return 1;
	uint8_t *vps = pnaludata + 4;
	uint8_t *sps = pnaludata + 4 + vpsLen + 4;
	uint8_t *pps=pnaludata + 4 + vpsLen + 4+spsLen+4;

    vps_sample.bytes=(char*)vps;
	vps_sample.nb=vpsLen;

    sps_sample.bytes=(char*)sps;
	sps_sample.nb=spsLen;

    pps_sample.bytes=(char*)pps;
	pps_sample.nb=ppsLen;

	yang_getConfig_Meta_H265(&vps_sample,&sps_sample,&pps_sample,videoFrame->payload,&videoFrame->nb);

	return Yang_Ok;
}

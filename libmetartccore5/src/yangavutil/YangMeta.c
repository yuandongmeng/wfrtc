//
// Copyright (c) 2019-2022 yanggaofeng
//
#include <yangavutil/video/YangCMeta.h>
#include <yangutil/sys/YangLog.h>
#include <yangutil/sys/YangEndian.h>
#include <yangutil/sys/YangAmf.h>
#include <yangutil/yangavtype.h>
#include <memory.h>
#include <stdbool.h>

void yang_find_start_code(YangVideoCodec pve,uint8_t *buf,int32_t bufLen,int32_t *vpsPos,int32_t *vpsLen,int32_t *spsPos,int32_t *spsLen,int32_t *ppsPos,int32_t *ppsLen)
{
   int32_t i = 0;
   *spsPos=0;*ppsPos=0;
   if(pve==Yang_VED_265)	  {
	   *vpsPos=0;
	   while(i<bufLen-3){
		      if (buf[i] == 0 && buf[i + 1] == 0 &&buf[i + 2] == 0&& buf[i + 3] == 1){
		       	*vpsPos=i+4;
		       	i+=4;
		       	 break;
		       }
		      i++;
	   }
   }
   while (i <bufLen-3) {
       if (buf[i] == 0 && buf[i + 1] == 0 &&buf[i + 2] == 0&& buf[i + 3] == 1){
    	   if(pve==Yang_VED_265) *vpsLen=i-4;
       	*spsPos=i+4;
       	i+=4;
       	 break;
       }

       i++;
   }
   while (i <bufLen-3) {
       if (buf[i] == 0 && buf[i + 1] == 0 &&buf[i + 2] == 0&& buf[i + 3] == 1){
       	*spsLen=i-*spsPos;
       	*ppsPos=i+4;
       	*ppsLen=bufLen-*ppsPos;
       	 break;
       }

       i++;
   }

}
int32_t yang_find_pre_start_code(uint8_t *buf,int32_t bufLen){
	int32_t i = 0;
	while(i<bufLen-3){
		if (buf[i] == 0 && buf[i + 1] == 0 &&buf[i + 2] == 0&& buf[i + 3] == 1){
			return i;
			break;
		}
		i++;
	}
	return -1;
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

void yang_createH264Meta( YangVideoMeta* pvmd, YangFrame *videoFrame){
	uint32_t remainNalusLength = videoFrame->nb;
	char* curPtrInNalus = videoFrame->payload;
	uint32_t startIndex = 0;
	uint32_t nextNaluLength = 0;
	memset(pvmd,0,sizeof( YangVideoMeta));

	uint32_t spslen = 0;
	uint32_t ppslen = 0;

    do {
        int32_t ret = getNextNaluLength(curPtrInNalus, remainNalusLength, &startIndex, &nextNaluLength);
		if (ret < 0) {
			continue;
		}

        curPtrInNalus += startIndex;
        remainNalusLength -= startIndex;

        if (remainNalusLength == 0) {
			return;
		}
		// SPS
		if (ret == 0x67) {
			pvmd->mp4Meta.spsLen = nextNaluLength;
			memcpy(pvmd->mp4Meta.sps, curPtrInNalus, nextNaluLength);
		}
		// PPS
		else if (ret == 0x68) {
			pvmd->mp4Meta.ppsLen = nextNaluLength;
			memcpy(pvmd->mp4Meta.pps, curPtrInNalus, nextNaluLength);
			break;
		}

        remainNalusLength -= nextNaluLength;
        curPtrInNalus += nextNaluLength;
    } while (remainNalusLength != 0);
}

void yang_createH265Meta( YangVideoMeta* pvmd, YangFrame *videoFrame){
	memset(pvmd,0,sizeof( YangVideoMeta));
	uint32_t vpslen=yang_get_be32(videoFrame->payload);
	uint32_t spslen=yang_get_be32(videoFrame->payload+vpslen+4);
	uint32_t ppslen=yang_get_be32(videoFrame->payload+vpslen+4+spslen+4);

	pvmd->mp4Meta.vpsLen=vpslen;
	pvmd->mp4Meta.spsLen=spslen;
	pvmd->mp4Meta.ppsLen=ppslen;
	memcpy(pvmd->mp4Meta.vps,videoFrame->payload+4,vpslen);
	memcpy(pvmd->mp4Meta.sps,videoFrame->payload+4+vpslen+4,spslen);
	memcpy(pvmd->mp4Meta.pps,videoFrame->payload+4+vpslen+4+spslen+4,ppslen);

}


void yang_getConfig_Flv_H264( YangH2645Conf *p_264, uint8_t *configBuf,
		int32_t *p_configLen) {

	configBuf[0] = 0x17;
	configBuf[1] = 0x00;
	configBuf[2] = 0x00;
	configBuf[3] = 0x00;
	configBuf[4] = 0x00;
	configBuf[5] = 0x01;
	configBuf[6] = p_264->sps[1];//0x42;
	configBuf[7] = p_264->sps[2];//0xC0;
	configBuf[8] = p_264->sps[3];//0x29;		//0x29;  //AVCLevelIndication1f
	configBuf[9] = 0xff;		//03;//ff;//0x03; AVCLevelIndication
	configBuf[10] = 0xe1;		//01;//e1;//01;numOfSequenceParameterSets
	uint8_t * szTmp =  configBuf + 11;


    *szTmp++=0x00;
    *szTmp++=p_264->spsLen;

	memcpy(szTmp, p_264->sps, p_264->spsLen);
	szTmp += p_264->spsLen;
	// *szTmp = 0x01;
	// szTmp += 1;
    *szTmp++=0x00;
    *szTmp++=p_264->ppsLen;
	memcpy(szTmp, p_264->pps, p_264->ppsLen);

	szTmp += p_264->ppsLen;
	*p_configLen = szTmp - configBuf;
	szTmp = NULL;
}

void yang_getConfig_Meta_H264( YangSample* psps,  YangSample* ppps,uint8_t *configBuf,int32_t *p_configLen){
	//type_codec1 + avc_type + composition time + fix header + count of sps + len of sps + sps + count of pps + len of pps + pps
				// int32_t nb_payload = 1 + 1 + 3 + 5 + 1 + 2 + sps->size + 1 + 2 + pps->size;
	int32_t spsLen=psps->nb;
	int32_t ppsLen=ppps->nb;
	uint8_t* sps=(uint8_t*)psps->bytes;
	uint8_t* pps=(uint8_t*)ppps->bytes;
	configBuf[0] = 0x17;
	configBuf[1] = 0x00;
	configBuf[2] = 0x00;
	configBuf[3] = 0x00;
	configBuf[4] = 0x00;
	configBuf[5] = 0x01;
	configBuf[6] = sps[1];//0x42;
	configBuf[7] = sps[2];//0xC0;
	configBuf[8] = sps[3];//0x29;		//0x29;  //AVCLevelIndication1f
	configBuf[9] = 0xff;		//03;//ff;//0x03; AVCLevelIndication
	configBuf[10] = 0xe1;		//01;//e1;//01;numOfSequenceParameterSets
	uint8_t * szTmp = configBuf + 11;

	yang_put_be16((char*) szTmp, (uint16_t) spsLen);
	szTmp+=2;
    //*szTmp++=0x00;
   // *szTmp++=spsLen;

	memcpy(szTmp, sps, spsLen);
	szTmp += spsLen;
	*szTmp = 0x01;
	szTmp += 1;

	yang_put_be16((char*) szTmp, (uint16_t) ppsLen);
	szTmp+=2;
   // *szTmp++=0x00;
   // *szTmp++=ppsLen;
	memcpy(szTmp, pps, ppsLen);

	szTmp += ppsLen;
	*p_configLen = szTmp -  configBuf;
	szTmp = NULL;
}
void yang_getConfig_Flv_H265( YangH2645Conf *p_264,	uint8_t *configBuf, int32_t *p_configLen) {
	int32_t i = 0;
	configBuf[i++] = 0x1C;
	configBuf[i++] = 0x00;

	configBuf[i++] = 0x00;
	configBuf[i++] = 0x00;
	configBuf[i++] = 0x00;
//configurationVersion
	configBuf[i++] = 0x00;


	configBuf[i++] = p_264->sps[1];
//general_profile_compatibility_flags 32 bit
	configBuf[i++] = p_264->sps[2];
	configBuf[i++] = p_264->sps[3];
	configBuf[i++] = p_264->sps[4];
	configBuf[i++] = p_264->sps[5];

// 48 bit NUll nothing deal in rtmp
	configBuf[i++] = p_264->sps[6];
	configBuf[i++] = p_264->sps[7];
	configBuf[i++] = p_264->sps[8];
	configBuf[i++] = p_264->sps[9];
	configBuf[i++] = p_264->sps[10];
	configBuf[i++] = p_264->sps[11];

//general_level_idc
	configBuf[i++] = p_264->sps[12];


// 48 bit NUll nothing deal in rtmp
	configBuf[i++] = 0x00;
	configBuf[i++] = 0x00;
	configBuf[i++] = 0x00;
	configBuf[i++] = 0x00;
	configBuf[i++] = 0x00;
	configBuf[i++] = 0x00;

//bit(16) avgFrameRate;
	configBuf[i++] = 0x00;
	configBuf[i++] = 0x00;

//bit(2) constantFrameRate;
// bit(3) numTemporalLayers;
///bit(1) temporalIdNested;
	configBuf[i++] = 0x00;
	configBuf[i++] = 0x03;

//yang_trace("HEVCDecoderConfigurationRecord data = %s\n", configBuf);
	configBuf[i++] = 0x20;  //vps 32
	configBuf[i++] = 0x0;
	configBuf[i++] = 0x01;
	configBuf[i++] = (p_264->vpsLen >> 8) & 0xff;
	configBuf[i++] = (p_264->vpsLen) & 0xff;
	memcpy(&configBuf[i], p_264->vps, p_264->vpsLen);
	i += p_264->vpsLen;

	configBuf[i++] = 0x21; //sps 33
	configBuf[i++] = (1 >> 8) & 0xff;
	configBuf[i++] = 1 & 0xff;
	configBuf[i++] = (p_264->spsLen >> 8) & 0xff;
	configBuf[i++] = (p_264->spsLen) & 0xff;
	memcpy(&configBuf[i], p_264->sps, p_264->spsLen);
	i += p_264->spsLen;

	configBuf[i++] = 0x22; //sps 33
	configBuf[i++] = (1 >> 8) & 0xff;
	configBuf[i++] = 1 & 0xff;
	configBuf[i++] = (p_264->ppsLen >> 8) & 0xff;
	configBuf[i++] = (p_264->ppsLen) & 0xff;
	memcpy(&configBuf[i], p_264->pps, p_264->ppsLen);
	i += p_264->ppsLen;
	*p_configLen = i;

}
void yang_getConfig_Meta_H265( YangSample* pvps, YangSample* psps,  YangSample* ppps, uint8_t *configBuf,int32_t *p_configLen){
	int32_t i = 0;
	int32_t vpsLen=pvps->nb;
	int32_t spsLen=psps->nb;
	int32_t ppsLen=ppps->nb;
	uint8_t* vps=(uint8_t*)pvps->bytes;
	uint8_t* sps=(uint8_t*)psps->bytes;
	uint8_t* pps=(uint8_t*)ppps->bytes;
		configBuf[i++] = 0x1C;
		configBuf[i++] = 0x00;

		configBuf[i++] = 0x00;
		configBuf[i++] = 0x00;
		configBuf[i++] = 0x00;
	//configurationVersion
		configBuf[i++] = 0x00;


		configBuf[i++] = sps[1];
	//general_profile_compatibility_flags 32 bit
		configBuf[i++] = sps[2];
		configBuf[i++] = sps[3];
		configBuf[i++] = sps[4];
		configBuf[i++] = sps[5];

	// 48 bit NUll nothing deal in rtmp
		configBuf[i++] = sps[6];
		configBuf[i++] = sps[7];
		configBuf[i++] = sps[8];
		configBuf[i++] = sps[9];
		configBuf[i++] = sps[10];
		configBuf[i++] = sps[11];

	//general_level_idc
		configBuf[i++] = sps[12];


	// 48 bit NUll nothing deal in rtmp
		configBuf[i++] = 0x00;
		configBuf[i++] = 0x00;
		configBuf[i++] = 0x00;
		configBuf[i++] = 0x00;
		configBuf[i++] = 0x00;
		configBuf[i++] = 0x00;

	//bit(16) avgFrameRate;
		configBuf[i++] = 0x00;
		configBuf[i++] = 0x00;

	//bit(2) constantFrameRate;
	// bit(3) numTemporalLayers;
	///bit(1) temporalIdNested;
		configBuf[i++] = 0x00;
		configBuf[i++] = 0x03;


		configBuf[i++] = 0x20;  //vps 32
		configBuf[i++] = 0x0;
		configBuf[i++] = 0x01;
		configBuf[i++] = (vpsLen >> 8) & 0xff;
		configBuf[i++] = (vpsLen) & 0xff;

		memcpy(&configBuf[i], vps, vpsLen);
		i += vpsLen;

		configBuf[i++] = 0x21; //sps 33
		configBuf[i++] = (1 >> 8) & 0xff;
		configBuf[i++] = 1 & 0xff;
		configBuf[i++] = (spsLen >> 8) & 0xff;
		configBuf[i++] = (spsLen) & 0xff;

		memcpy(&configBuf[i], sps, spsLen);
		i += spsLen;

		configBuf[i++] = 0x22; //sps 33
		configBuf[i++] = (1 >> 8) & 0xff;
		configBuf[i++] = 1 & 0xff;
		configBuf[i++] = (ppsLen >> 8) & 0xff;
		configBuf[i++] = (ppsLen) & 0xff;

		memcpy(&configBuf[i], pps, ppsLen);
		i += ppsLen;
		*p_configLen = i;
}


void yang_getH265RtmpHeader(uint8_t *meta, uint8_t *src, int32_t *hLen) {
				uint8_t s[4];
				s[0]=0x00;
				s[1]=0x00;
				s[2]=0x00;
				s[3]=0x01;

				int32_t pos=0;
				int32_t vpsLen=*(meta+32);
				int32_t vpsPos=33;

				int32_t spsLen=*(meta+32+vpsLen+5);
				int32_t spsPos=32+vpsLen+5+1;

				int32_t ppsLen=*(meta+32+vpsLen+5+spsLen+5);
				int32_t ppsPos=32+vpsLen+5+spsLen+5+1;


				memcpy(src,s,4);
				memcpy(src+4,meta+vpsPos,vpsLen);
				pos+=4+vpsLen;
				memcpy(src+pos,s,4);
				pos+=4;
				memcpy(src+pos,meta+spsPos,spsLen);
				pos+=spsLen;
				memcpy(src+pos,s,4);
				pos+=4;
				memcpy(src+pos,meta+ppsPos,ppsLen);
				pos+=ppsLen;
				*hLen=pos;

}

void yang_getH264RtmpHeader(uint8_t *buf, uint8_t *src, int32_t *hLen) {
	int32_t spsLen = *(buf + 12) + 1;
	uint8_t *spsStart = buf + 13;
	int32_t ppsLen = *(spsStart + spsLen + 1) + 1;
	uint8_t *ppsStart = buf + 13 + spsLen + 2;
	*(src + 3) = 0x01;
	memcpy(src + 4, spsStart, spsLen);
	*(src + 4 + spsLen + 3) = 0x01;
	memcpy(src + 4 + spsLen + 4, ppsStart, ppsLen);
	*hLen = 8 + spsLen + ppsLen;
}

void yang_decodeMetaH264(uint8_t *buf,int32_t p_configLen, YangSample* sps, YangSample* pps){
	sps->nb= *(buf + 12);
	sps->bytes = (char*)buf + 13;
	pps->nb = *(sps->bytes + sps->nb + 1);
	pps->bytes= (char*)buf + 13 + sps->nb + 2;
}
void yang_decodeMetaH265(uint8_t *meta,int32_t p_configLen, YangSample* vps, YangSample* sps, YangSample* pps){
	vps->nb=*(meta+32);
	vps->bytes=(char*)meta+33;
	int32_t vpsLen=vps->nb;
	sps->nb=*(meta+32+vpsLen+5);
	int32_t spsLen=sps->nb;
	sps->bytes=(char*)meta+32+vpsLen+5+1;

	pps->nb=*(meta+32+vpsLen+5+spsLen+5);
	pps->bytes=(char*)meta+32+vpsLen+5+spsLen+5+1;
}

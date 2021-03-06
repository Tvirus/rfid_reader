#ifndef  _ISO7816_H_
#define  _ISO7816_H_

#include "iso14443.h"


#ifndef u8
#define u8  unsigned char
#endif
#ifndef s8
#define s8  signed char
#endif

#ifndef u16
#define u16  unsigned short
#endif
#ifndef s16
#define s16  signed short
#endif

#ifndef u32
#define u32  unsigned int
#endif
#ifndef s32
#define s32  unsigned int
#endif




#define MFID  0x3F00


extern int iso7816_debug;


extern int iso7816_select_id(card_info_t *info, u16 id, u8 *rsp, int size);
extern int iso7816_get_challenge(card_info_t *info, u8 challenge_len, u8 *challenge);
extern int iso7816_exter_authn(card_info_t *info, u8 key_id, const u8 *data);
#define ALGORITHM_ENCRYPT  0x00
#define ALGORITHM_DECRYPT  0x01
#define ALGORITHM_MAC      0x02
extern int iso7816_inter_authn(card_info_t *info, u8 key_id, u8 algorithm, const u8 *data, u8 *result);
extern int iso7816_read_binary(card_info_t *info, u16 offset, u8 len, u8 *buf);
extern int iso7816_write_binary(card_info_t *info, u16 offset, u8 len, const u8 *buf);


extern int fmcos_erase(card_info_t *info);
extern int fmcos_create_keyfile(card_info_t *info, u16 id, u16 filesize, u8 shortid, u8 wstatus_upper, u8 wstatus_lower);
extern int fmcos_create_binfile(card_info_t *info, u16 id, u16 filesize, u8 rstatus_upper, u8 rstatus_lower, u8 wstatus_upper, u8 wstatus_lower);




#endif

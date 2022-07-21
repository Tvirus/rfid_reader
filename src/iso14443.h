#ifndef  _ISO14443_H_
#define  _ISO14443_H_




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
#define s32  signed int
#endif




#define UID_SIZE_4  0x00
#define UID_SIZE_7  0x01
#define UID_SIZE_10 0x02
#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
typedef struct {
    u8 rfu0:4;
    u8 rsv:4;

    u8 uid_size:2;
    u8 rfu1:1;
    u8 anticollision:5;
}ATQA_t;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
typedef struct {
    u8 rsv:4;
    u8 rfu0:4;

    u8 anticollision:5;
    u8 rfu1:1;
    u8 uid_size:2;
}ATQA_t;
#else
#error  need define __BYTE_ORDER__
#endif


#define SELECT_LEVEL_1  0x93
#define SELECT_LEVEL_2  0x95
#define SELECT_LEVEL_3  0x97
#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
typedef struct {
    u8 level;

    u8 bytes:4;
    u8 bits:4;

    u8 uid[4];
    u8 bcc;
}SELECT_t;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
typedef struct {
    u8 level;

    u8 bits:4;
    u8 bytes:4;

    u8 uid[4];
    u8 crc_a;
}SELECT_t;
#else
#error  need define __BYTE_ORDER__
#endif

#define SAK_UID_NOT_COMPLETE  0x04
#define SAK_SPT_ISO14443_4    0x20



#define RATS  0xE0
#define FSI_16   0x00
#define FSI_24   0x01
#define FSI_32   0x02
#define FSI_40   0x03
#define FSI_48   0x04
#define FSI_64   0x05
#define FSI_96   0x06
#define FSI_128  0x07
#define FSI_256  0x08
#define FSI_512  0x09
#define FSI_1024 0x0A
#define FSI_2048 0x0B
#define FSI_4096 0x0C
#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
typedef struct {
    u8 cmd;

    u8 fsdi:4;
    u8 cid:4; //如果PICC的CID是0，PICC会回应不包含CID的消息
}RATS_t;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
typedef struct {
    u8 cmd;

    u8 cid:4;
    u8 fsdi:4;
}RATS_t;
#else
#error  need define __BYTE_ORDER__
#endif

#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
typedef struct {
    u8 tl; //长度，包括TL本身，不包括2字节CRC

    u8 rfu:1;
    u8 tc:1;
    u8 tb:1;
    u8 ta:1;
    u8 fsci:4;

    u8 data[0];
}ATS_t;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
typedef struct {
    u8 tl;

    u8 fsci:4;
    u8 ta:1;
    u8 tb:1;
    u8 tc:1;
    u8 rfu:1;

    u8 data[0];
}ATS_t;
#else
#error  need define __BYTE_ORDER__
#endif

#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
typedef struct {
    u8 diff_div:1; //different divisors for each direction
    u8 ds8:1;
    u8 ds4:1;
    u8 ds2:1;
    u8 rfu:1;
    u8 dr8:1;
    u8 dr4:1;
    u8 dr2:1;
}ATS_TA_t;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
typedef struct {
    u8 dr2:1;
    u8 dr4:1;
    u8 dr8:1;
    u8 rfu:1;
    u8 ds2:1;
    u8 ds4:1;
    u8 ds8:1;
    u8 diff_div:1;
}ATS_TA_t;
#else
#error  need define __BYTE_ORDER__
#endif

#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
typedef struct {
    u8 fwi:4;
    u8 sfgi:4;
}ATS_TB_t;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
typedef struct {
    u8 sfgi:4;
    u8 fwi:4;
}ATS_TB_t;
#else
#error  need define __BYTE_ORDER__
#endif

#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
typedef struct {
    u8 rfu:6;
    u8 spt_cid:1;
    u8 spt_nad:1;
}ATS_TC_t;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
typedef struct {
    u8 spt_nad:1;
    u8 spt_cid:1;
    u8 rfu:6;
}ATS_TC_t;
#else
#error  need define __BYTE_ORDER__
#endif


#define BLOCK_TYPE_I  0x00
#define BLOCK_TYPE_R  0x02
#define BLOCK_TYPE_S  0x03
#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
typedef struct {
    u8 block_type:2;
    u8 rsv0_set0:1;
    u8 chaining:1;
    u8 cid:1;
    u8 nad:1;
    u8 rsv1_set1:1;
    u8 block_num:1; //必须从0开始
}I_BLOCK_t;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
typedef struct {
    u8 block_num:1;
    u8 rsv1_set1:1;
    u8 nad:1;
    u8 cid:1;
    u8 chaining:1;
    u8 rsv0_set0:1;
    u8 block_type:2;
}I_BLOCK_t;
#else
#error  need define __BYTE_ORDER__
#endif

#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
typedef struct {
    u8 block_type:2;
    u8 rsv0_set1:1;
    u8 nak:1; //1:nak, 0:ack
    u8 cid:1;
    u8 rsv1_set0:1;
    u8 rsv2_set1:1;
    u8 block_num:1;
}R_BLOCK_t;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
typedef struct {
    u8 block_num:1;
    u8 rsv2_set1:1;
    u8 rsv1_set0:1;
    u8 cid:1;
    u8 nak:1; //1:nak, 0:ack
    u8 rsv0_set1:1;
    u8 block_type:2;
}R_BLOCK_t;
#else
#error  need define __BYTE_ORDER__
#endif

#define S_BLOCK_DESELECT  0x00
#define S_BLOCK_WTX       0x03
#define S_BLOCK_PARAM     0x03
#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
typedef struct {
    u8 block_type:2;
    u8 desel_wtx:2; //not_param=0时应该设置成11，not_param=1时、00是deselect 11是wtx
    u8 cid:1;
    u8 rsv0:1;
    u8 not_param:1; //0:parameters  1:deselect or wtx
    u8 rsv1:1;
}S_BLOCK_t;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
typedef struct {
    u8 rsv1:1;
    u8 not_param:1; //0:parameters  1:deselect or wtx
    u8 rsv0:1;
    u8 cid:1;
    u8 desel_wtx:2; //not_param=0时应该设置成11，not_param=1时、00是deselect 11是wtx
    u8 block_type:2;
}S_BLOCK_t;
#else
#error  need define __BYTE_ORDER__
#endif

#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
typedef struct {
    u8 power_level:2;
    u8 rfu:2;
    u8 cid:4; //如果PICC的CID是0，PICC会回应不包含CID的消息
}BLOCK_CID_t;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
typedef struct {
    u8 cid:4;
    u8 rfu:2;
    u8 power_level:2;
}BLOCK_CID_t;
#else
#error  need define __BYTE_ORDER__
#endif

#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
typedef struct {
    u8 rsv0:1;
    u8 dad:3;
    u8 rsv1:1;
    u8 sad:3;
}BLOCK_NAD_t;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
typedef struct {
    u8 sad:3;
    u8 rsv1:1;
    u8 dad:3;
    u8 rsv0:1;
}BLOCK_NAD_t;
#else
#error  need define __BYTE_ORDER__
#endif






#define TYPEB_APP_ALL         0
#define TYPEB_APP_TRANSPORT   1
#define TYPEB_APP_FINANCIAL   2
#define TYPEB_APP_ID          3
#define TYPEB_APP_TELECOM     4
#define TYPEB_APP_MEDICAL     5
#define TYPEB_APP_MULTIMEDIA  6
#define TYPEB_APP_GAMING      7
#define TYPEB_APP_STORAGE     8

#define TYPEB_SUBAPP_ALL      0

#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
typedef struct {
    u8 apf;

    //AFI
    u8 afi_app:4;
    u8 afi_subapp:4;

    //PARAM
    u8 rfu:3;
    u8 ext_atqb:1; //extended ATQB
    u8 wupb:1;
    u8 n:3;
}REQB_t;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
typedef struct {
    u8 apf;

    //AFI
    u8 afi_subapp:4;
    u8 afi_app:4;

    //PARAM
    u8 n:3;
    u8 wupb:1;
    u8 ext_atqb:1; //extended ATQB
    u8 rfu:3;
}REQB_t;
#else
#error  need define __BYTE_ORDER__
#endif

#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
typedef struct {
    u8 cmd;
    u8 pupi[4];

    //Application data
    u8 afi;
    u8 crc_b[2];
    u8 app_num:4;
    u8 app_num_all:4;

    //Protocol Info
    u8 bitrate_same:1;
    u8 bitrate_c2d_848k:1;
    u8 bitrate_c2d_424k:1;
    u8 bitrate_c2d_212k:1;
    u8 bitrate_rfu:1;
    u8 bitrate_d2c_848k:1;
    u8 bitrate_d2c_424k:1;
    u8 bitrate_d2c_212k:1;

    u8 fsci:4; //Maximum frame size
    u8 rfu0:1;
    u8 min_tr2:2; //delay between PICC EOF start and PCD SOF start
    u8 spt_14443_4:1;

    u8 fwi:4;
    u8 rfu1:1;
    u8 adc:1; //Application Data Coding
    u8 spt_nad:1;
    u8 spt_cid:1;

    //Extended ATQB
    u8 sfgi:4;
    u8 rfu2:4;
}ATQB_t;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
typedef struct {
    u8 cmd;
    u8 pupi[4];

    u8 afi;
    u8 crc_b[2];
    u8 app_num_all:4;
    u8 app_num:4;

    //Protocol Info
    u8 bitrate_d2c_212k:1;
    u8 bitrate_d2c_424k:1;
    u8 bitrate_d2c_848k:1;
    u8 bitrate_rfu:1;
    u8 bitrate_c2d_212k:1;
    u8 bitrate_c2d_424k:1;
    u8 bitrate_c2d_848k:1;
    u8 bitrate_same:1;

    u8 spt_14443_4:1;
    u8 min_tr2:2; //delay between PICC EOF start and PCD SOF start
    u8 rfu0:1;
    u8 fsci:4; //Maximum frame size

    u8 spt_cid:1;
    u8 spt_nad:1;
    u8 adc:1; //Application Data Coding
    u8 rfu1:1;
    u8 fwi:4;

    //Extended ATQB
    u8 rfu2:4;
    u8 sfgi:4;
}ATQB_t;
#else
#error  need define __BYTE_ORDER__
#endif

#define ATTRIB_BITRATE_106K  0
#define ATTRIB_BITRATE_212K  1
#define ATTRIB_BITRATE_424K  2
#define ATTRIB_BITRATE_848K  3
#if __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
typedef struct {
    u8 cmd;
    u8 id[4];

    //Param1
    u8 min_tr0:2; //the PICC minimum delay before responding after the end of a command sent by a PCD
    u8 min_tr1:2; //the PICC minimum delay between subcarrier modulation start and beginning of data transmission
    u8 eof:1;
    u8 sof:1;
    u8 rfu0:2;

    //Param2
    u8 bitrate_c2d:2;
    u8 bitrate_d2c:2;
    u8 fsdi:4;

    //Param3
    u8 rfu1:5;
    u8 min_tr2:2;
    u8 spt_14443_4:1;

    //Param4
    u8 rfu2:4;
    u8 cid:4; //如果PICC不支持CID，PICC不会回应非0CID的消息
}ATTRIB_t;
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
typedef struct {
    u8 cmd;
    u8 id[4];

    //Param1
    u8 rfu0:2;
    u8 sof:1;
    u8 eof:1;
    u8 min_tr1:2; //the PICC minimum delay between subcarrier modulation start and beginning of data transmission
    u8 min_tr0:2; //the PICC minimum delay before responding after the end of a command sent by a PCD

    //Param2
    u8 fsdi:4;
    u8 bitrate_d2c:2;
    u8 bitrate_c2d:2;

    //Param3
    u8 spt_14443_4:1;
    u8 min_tr2:2;
    u8 rfu1:5;

    //Param4
    u8 cid:4; //如果PICC不支持CID，PICC不会回应非0CID的消息
    u8 rfu2:4;
}ATTRIB_t;
#else
#error  need define __BYTE_ORDER__
#endif




#define MAX_UID_SIZE 10
/*
typedef struct {
    u8 len;
    u8 uid[MAX_UID_SIZE];
}UID_t;
*/
typedef struct {
    u8 cid;
    u8 nads; //sad
    u8 nadd; //dad
    //以上由用户设置

    u8 uid_len;
    u8 uid[MAX_UID_SIZE];
    u8 spt_14443_4;
    u8 spt_cid;
    u8 spt_nad;
    u8 power_level;
    u8 block_num;

    u32 fsc; //卡最大接收帧长度 fsc Frame Size for proximity Card
    u32 sfg; //ms 定义了在发送了ATS之后，准备接收下一个帧之前PICC所需的特定保护时间
    u32 fwt; //ms 帧等待时间，Frame waiting time，pcd的帧发完后多久内picc需要回复

    u8 spt_diff_div:1; //different divisors for each direction
    u8 spt_ds8:1;
    u8 spt_ds4:1;
    u8 spt_ds2:1;
    u8 spt_dr8:1;
    u8 spt_dr4:1;
    u8 spt_dr2:1;

    //Type_B
    u8 pupi[4];
    u8 afi;
    u8 crc_b[2];
    u8 app_num;
    u8 app_num_all;
    u8 min_tr2;
    u8 adc;

    u8 bitrate_same:1;
    u8 bitrate_c2d_848k:1;
    u8 bitrate_c2d_424k:1;
    u8 bitrate_c2d_212k:1;
    u8 bitrate_d2c_848k:1;
    u8 bitrate_d2c_424k:1;
    u8 bitrate_d2c_212k:1;
}card_info_t;


extern int typeab_debug;


extern int init_card_info(card_info_t *info, u8 cid);

extern int typea_activate(card_info_t *info);
extern int typea_halt(void);
extern int typea_rats(card_info_t *info);

extern int typeb_request(card_info_t *info, u8 wakeup, u8 app, u8 sub_app, ATQB_t *atqb);
extern int typeb_halt(card_info_t *info);
extern int typeb_attrib(card_info_t *info);

extern int typeab_send(card_info_t *info, const u8 *send, u8 send_len, u8 *recv, u8 recv_len);
extern int typeab_deselect(card_info_t *info);

extern int typeb_read_idcard_uid(card_info_t *info, u8 *uid);


#endif

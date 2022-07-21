#include "iso14443.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "fm17550.h"




#define GET_CRC(buf) ((buf)[0] ^ (buf)[1] ^ (buf)[2] ^ (buf)[3])
#define FWI2FWT(fwi) ((1 << (fwi)) * 256 * 16 / 13560)
#define SFGI2SFGT(sfgi) ((1 << (sfgi)) * 256 * 16 / 13560)

#define DEBUG(fmt, arg...)  do{if(iso14443_debug)printf("--TypeAB-- " fmt "\n", ##arg);}while(0)
#define ERROR(fmt, arg...)  printf("--TypeAB-- " fmt "\n", ##arg)




int iso14443_debug = 1;


static u32 fsi2fs[] = {
    [0x00] = 16,
    [0x01] = 24,
    [0x02] = 32,
    [0x03] = 40,
    [0x04] = 48,
    [0x05] = 64,
    [0x06] = 96,
    [0x07] = 128,
    [0x08] = 256,
    [0x09] = 512,
    [0x0A] = 1024,
    [0x0B] = 2048,
    [0x0C] = 4096
};
#define MAX_FSI ((sizeof(fsi2fs) / sizeof(fsi2fs[0])) - 1)
#define MAX_FS  (fsi2fs[MAX_FSI])
u32 get_fs(u32 fsi)
{
    if (MAX_FSI <= fsi)
        return MAX_FS;
    return fsi2fs[fsi];
}
u8 get_fsi(u32 fs)
{
    int i;

    for (i = MAX_FSI; i >= 0; i--)
    {
        if (fsi2fs[i] <= fs)
            return (u8)i;
    }

    ERROR("%s: invalid fs(%u) !", __func__, fs);
    return 0;
}


int init_card_info(card_info_t *info, u8 cid)
{
    if (NULL == info)
        return -1;

    memset((u8 *)info, 0, sizeof(card_info_t));
    info->cid = cid;

    if (15 <= cid)
        return -1;

    return 0;
}

//wakeup:是否使用wakeup代替req，和req相比、wakeup还可以多唤醒处于halt状态的卡
int typea_request(u8 wakeup, ATQA_t *atqa)
{
    u8 cmd;
    u8 buf[2];
    u8 coll_pos;
    int len;


    if (NULL == atqa)
        return -1;

    pcd_enable_crc(0);
    pcd_set_timer(1);
    //待处理。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。
    fm17550_set_bits(Status2Reg, Crypto1On, 0);

    if (wakeup)
    {
        DEBUG("  ==> WAKEUP A");
        cmd = 0x52;
    }
    else
    {
        DEBUG("  ==> REQA");
        cmd = 0x26;
    }

    len = pcd_send(&cmd, 7, buf, sizeof(buf), 0, &coll_pos);
    if ((2 * 8) != len)
    {
        if (wakeup)
            DEBUG("WAKEUP A failed(%d)", len);
        else
            DEBUG("REQA failed(%d)", len);
        return -1;
    }

    ((u8 *)atqa)[0] = buf[0];
    ((u8 *)atqa)[1] = buf[1];
    DEBUG("<==   ATQA: %02x %02x, rsv:%u uid_size:%u", buf[0], buf[1], atqa->rsv, atqa->uid_size);
    if (coll_pos)
    {
        DEBUG("      ATQA collison %u", coll_pos);
        atqa->anticollision = 0x1F;
    }

    return 0;
}


int typea_halt(void)
{
    int len;

    pcd_enable_crc(1);
    pcd_set_timer(1);
    len = pcd_send((u8 *)"\x50\x00", 2 * 8, NULL, 0, 0, NULL);
    if (0 != len)
    {
        ERROR("halt a failed(%d) !", len);
        return -1;
    }
    return 0;
}


//level: 串联级别 cascade level, [1,3]
static int typea_anticoll(u8 level, u8 *uid, u8 *valid_bits)
{
    SELECT_t sel;
    u8 buf[5] = {0};
    u8 coll_pos;
    int len;
    u8 start, mask;


    if (32 == *valid_bits)
        return 0;
    else if (32 < *valid_bits)
        return -1;

    pcd_enable_crc(0);
    pcd_set_timer(1);

    if (1 == level)
        sel.level = SELECT_LEVEL_1;
    else if (2 == level)
        sel.level = SELECT_LEVEL_2;
    else if (3 == level)
        sel.level = SELECT_LEVEL_3;
    else
        return -1;
    sel.bytes = (*valid_bits) / 8 + 2;
    sel.bits  = (*valid_bits) & 0x07;
    memcpy(sel.uid, uid, 4);
    DEBUG("  ==> Anticoll: %02x %02x  %02x %02x %02x %02x, valid_bits:%u",
           ((u8 *)&sel)[0],
           ((u8 *)&sel)[1],
           ((u8 *)&sel)[2],
           ((u8 *)&sel)[3],
           ((u8 *)&sel)[4],
           ((u8 *)&sel)[5],
           *valid_bits);

    len = pcd_send((u8 *)&sel, (*valid_bits) + 2*8, buf, sizeof(buf), (*valid_bits) & 0x07, &coll_pos);
    if ((5 * 8 - ((*valid_bits) & 0xF8)) != len)
    {
        ERROR("anticoll failed(%d) !", len);
        return -1;
    }
    len /= 8;
    if (32 < (*valid_bits + coll_pos))
    {
        ERROR("wrong coll_pos(%u) !", coll_pos);
        return -1;
    }

    //拼接
    start = (*valid_bits) / 8; //从uid的第几个字节开始复制
    mask = (u8)(0xFF << ((*valid_bits) % 8)); //第一个要复制字节的位
    uid[start] = (buf[0] & mask) | (uid[start] & ((u8)~mask));
    memcpy(&uid[start + 1], &buf[1], 3 - start);

    DEBUG("<==   Anticoll: %02x %02x %02x %02x %02x, len:%u, mask:0x%02x, coll_pos:%u",
           buf[0], buf[1], buf[2], buf[3], buf[4], len, mask, coll_pos);

    if (0 == coll_pos)
    {
        if (GET_CRC(uid) != buf[len - 1])
        {
            ERROR("anticoll crc error !");
            return -1;
        }
        *valid_bits = 4 * 8;
    }
    else
    {
        *valid_bits += coll_pos - 1;
    }

    return 0;
}


static int typea_select(u8 level, const u8 *uid, u8 *sak)
{
    SELECT_t sel;
    int len;


    pcd_enable_crc(1);
    pcd_set_timer(1);

    if (1 == level)
        sel.level = SELECT_LEVEL_1;
    else if (2 == level)
        sel.level = SELECT_LEVEL_2;
    else if (3 == level)
        sel.level = SELECT_LEVEL_3;
    else
        return -1;
    sel.bytes = 7;
    sel.bits  = 0;
    memcpy(sel.uid, uid, 4);
    sel.crc_a  = GET_CRC(sel.uid);
    DEBUG("  ==> SELECT: %02x %02x  %02x %02x %02x %02x  %02x",
           ((u8 *)&sel)[0],
           ((u8 *)&sel)[1],
           ((u8 *)&sel)[2],
           ((u8 *)&sel)[3],
           ((u8 *)&sel)[4],
           ((u8 *)&sel)[5],
           ((u8 *)&sel)[6]);

    len = pcd_send((u8 *)&sel, sizeof(sel) * 8, sak, 1, 0, NULL);
    if (8 != len)
    {
        ERROR("select failed(%d) !", len);
        return -1;
    }
    DEBUG("<==   SAK: %02x, not_complete:%u ISO14443-4:%u",
           *sak, !!((*sak)&SAK_UID_NOT_COMPLETE), !!((*sak)&SAK_SPT_ISO14443_4));

    return 0;
}


int typea_activate(card_info_t *info)
{
    ATQA_t atrq;
    u8 tmp_uid[4];
    u8 valid_bits = 0;
    u8 sak;
    int i, j;


    if (NULL == info)
        return -1;

    if (0 != typea_request(1, &atrq))
        return -1;

    info->uid_len = 0;
    for (i = 0; i < 3; i++)
    {
        valid_bits = 0;
        for (j = 0; j < 32; j++)
        {
            if (0 != typea_anticoll(i + 1, tmp_uid, &valid_bits))
                return -1;
            if (32 <= valid_bits)
                break;

            tmp_uid[valid_bits / 8] |= 1 << (valid_bits % 8);
            valid_bits++;
            if (32 <= valid_bits)
                break;
        }
        if (32 <= j)
        {
            ERROR("%s: typea_anticoll infinite loop !", __func__);
            return -1;
        }
        if (0 != typea_select(i + 1, tmp_uid, &sak))
            return -1;
        if (0 == (sak & SAK_UID_NOT_COMPLETE))
            break;
        memcpy(&info->uid[i * 3], &tmp_uid[1], 3);
        info->uid_len += 3;
    }
    if (3 <= i)
    {
        ERROR("%s: cascade level err !", __func__);
        return -1;
    }

    memcpy(&info->uid[i * 3], tmp_uid, 4);
    info->uid_len += 4;
    info->spt_14443_4 = !!(sak & SAK_SPT_ISO14443_4);

    DEBUG("[UID](%u): %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", info->uid_len,
           info->uid[0], info->uid[1], info->uid[2], info->uid[3],
           info->uid[4], info->uid[5], info->uid[6], info->uid[7], info->uid[8], info->uid[9]);

    return 0;
}




int typea_rats(card_info_t *info)
{
    RATS_t rats;
    u8 buf[PCD_FIFO_SIZE];
    ATS_t *ats = (ATS_t *)buf;
    int len;
    int i;

    if (NULL == info)
        return -1;

    pcd_enable_crc(1);
    pcd_set_timer(7);

    //设置默认值
    info->fsc = get_fs(FSI_16);
    info->sfg = 0;
    info->fwt = 100;
    info->spt_diff_div = 0;
    info->spt_ds8 = 0;
    info->spt_ds4 = 0;
    info->spt_ds2 = 0;
    info->spt_dr8 = 0;
    info->spt_dr4 = 0;
    info->spt_dr2 = 0;
    info->spt_cid = 0;
    info->spt_nad = 0;

    rats.cmd  = RATS;
    rats.fsdi = get_fsi(PCD_FIFO_SIZE);
    rats.cid  = info->cid;
    DEBUG("  ==> RATS: %02x %02x", ((u8 *)&rats)[0], ((u8 *)&rats)[1]);
    len = pcd_send((u8 *)&rats, sizeof(rats) * 8, buf, sizeof(buf), 0, NULL);
    if ((0 >= len) || (len % 8))
    {
        ERROR("rats failed(%d) !", len);
        return -1;
    }
    len /= 8;
    if (ats->tl != len)
    {
        ERROR("ats error, TL:%u, recv:%u !", ats->tl, len);
        return -1;
    }
    if (1 == len)
    {
        DEBUG("<==   ATS: %02x", buf[0]);
        return 1;
    }
    if (iso14443_debug)
    {
        printf("--TypeAB-- <==   ATS: ");
        for (i = 0; i < len; i++)
        {
            printf("%02x ", buf[i]);
        }
        printf("\n");
    }
    if ((ats->ta + ats->tb + ats->tc + sizeof(ATS_t)) > len)
    {
        ERROR("ats error, TL:%u, recv:%u, FSCI:%u, TA:%u, TB:%u, TC:%u !", \
               ats->tl, len, ats->fsci, ats->ta, ats->tb, ats->tc);
        return -1;
    }
    info->fsc = get_fs(ats->fsci);

    i = 0;
    if (ats->ta)
    {
        info->spt_diff_div = !!(((ATS_TA_t *)(&ats->data[i]))->diff_div);
        info->spt_ds8 = !!(((ATS_TA_t *)(&ats->data[i]))->ds8);
        info->spt_ds4 = !!(((ATS_TA_t *)(&ats->data[i]))->ds4);
        info->spt_ds2 = !!(((ATS_TA_t *)(&ats->data[i]))->ds2);
        info->spt_dr8 = !!(((ATS_TA_t *)(&ats->data[i]))->dr8);
        info->spt_dr4 = !!(((ATS_TA_t *)(&ats->data[i]))->dr4);
        info->spt_dr2 = !!(((ATS_TA_t *)(&ats->data[i]))->dr2);
        i++;
    }
    if (ats->tb)
    {
        info->fwt = FWI2FWT(((ATS_TB_t *)(&ats->data[i]))->fwi);
        info->sfg = SFGI2SFGT(((ATS_TB_t *)(&ats->data[i]))->sfgi);
        if (0 == info->fwt)
            info->fwt = 1;
        i++;
    }
    if (ats->tc)
    {
        info->spt_cid = !!(((ATS_TC_t *)(&ats->data[i]))->spt_cid);
        info->spt_nad = !!(((ATS_TC_t *)(&ats->data[i]))->spt_nad);
        i++;
    }

    DEBUG("          FSD:%u FSC:%u SFG:%ums FWT:%ums  diff_div:%u DS8:%u DS4:%u DS2:%u DR8:%u DR4:%u DR2:%u  cid:%u nad:%u",
           PCD_FIFO_SIZE,
           info->fsc,
           info->sfg,
           info->fwt,
           info->spt_diff_div,
           info->spt_ds8,
           info->spt_ds4,
           info->spt_ds2,
           info->spt_dr8,
           info->spt_dr4,
           info->spt_dr2,
           info->spt_cid,
           info->spt_nad);

    usleep((info->sfg) * 1000);
    return 0;
}




//slot: Slot-MARKER槽位号，如果是0或者1则为普通REQB命令，如果[2,16]则为Slot-MARKER命令
int typeb_request(card_info_t *info, u8 wakeup, u8 app, u8 sub_app, ATQB_t *atqb)
{
    REQB_t reqb;
    int len;
    int i;


    if ((NULL == info) || (NULL == atqb))
        return -1;

    pcd_set_timer(5);

    reqb.apf = 0x05;
    reqb.afi_app = app;
    reqb.afi_subapp = sub_app;
    reqb.rfu = 0;
    reqb.ext_atqb = 1;
    reqb.wupb = !!wakeup;
    reqb.n = 0; //暂时没做防冲突
    if (wakeup)
        DEBUG("  ==> WAKEUP B: %02x %02x %02x, app:%u, subapp:%u, ext_atqb:%u, n:%u", \
               ((u8*)&reqb)[0], ((u8*)&reqb)[1], ((u8*)&reqb)[2], app, sub_app, reqb.ext_atqb, reqb.n);
    else
        DEBUG("  ==> REQB: %02x %02x %02x, app:%u, subapp:%u, ext_atqb:%u, n:%u", \
               ((u8*)&reqb)[0], ((u8*)&reqb)[1], ((u8*)&reqb)[2], app, sub_app, reqb.ext_atqb, reqb.n);

    len = pcd_send((u8 *)&reqb, sizeof(reqb) * 8, (u8 *)atqb, sizeof(*atqb), 0, NULL);
    if (((sizeof(*atqb) * 8) != len) && (((sizeof(*atqb) - 1) * 8) != len))
    {
        if (wakeup)
            DEBUG("WAKEUP B failed(%d)", len);
        else
            DEBUG("REQB failed(%d) !", len);
        return -1;
    }
    len /= 8;
    if (0x50 != atqb->cmd)
    {
        ERROR("ATQB is not start with 0x50 !");
        return -1;
    }
    memcpy(info->pupi, atqb->pupi, 4);
    info->afi         = atqb->afi;
    info->crc_b[0]    = atqb->crc_b[0];
    info->crc_b[1]    = atqb->crc_b[1];
    info->app_num     = atqb->app_num;
    info->app_num_all = atqb->app_num_all;
    info->fsc         = get_fs(atqb->fsci);
    info->min_tr2     = atqb->min_tr2;
    info->spt_14443_4 = atqb->spt_14443_4;
    info->fwt         = FWI2FWT(atqb->fwi);
    if (0 == info->fwt)
        info->fwt = 1;
    info->adc         = atqb->adc;
    info->spt_nad     = atqb->spt_nad;
    info->spt_cid     = atqb->spt_cid;
    info->bitrate_same     = atqb->bitrate_same;
    info->bitrate_c2d_848k = atqb->bitrate_c2d_848k;
    info->bitrate_c2d_424k = atqb->bitrate_c2d_424k;
    info->bitrate_c2d_212k = atqb->bitrate_c2d_212k;
    info->bitrate_d2c_848k = atqb->bitrate_d2c_848k;
    info->bitrate_d2c_424k = atqb->bitrate_d2c_424k;
    info->bitrate_d2c_212k = atqb->bitrate_d2c_212k;
    if (sizeof(ATQB_t) == len)
        info->sfg = SFGI2SFGT(atqb->sfgi);
    else
        info->sfg = 0;
    if (iso14443_debug)
    {
        printf("--TypeAB-- <==   ATQB(%u):", len);
        for (i = 0; i < len; i++)
            printf(" %02x", ((u8*)atqb)[i]);
        printf("\n");
        printf("--TypeAB--           PUPI: %02x %02x %02x %02x, AFI:%02x, CRC_B: %02x %02x, app_num:%u, app_num_all:%u\n",
                info->pupi[0], info->pupi[1], info->pupi[2], info->pupi[3], info->afi, info->crc_b[0], info->crc_b[1], info->app_num, info->app_num_all);
        printf("--TypeAB--           FSC:%u, TR2:%u, 14443-4:%u, FWT:%u, ADC:%u, nad:%u, cid:%u, SFG:%u\n",
                info->fsc, info->min_tr2, info->spt_14443_4, info->fwt, info->adc, info->spt_nad, info->spt_cid, info->sfg);
        printf("--TypeAB--           bitrate: same(%u) c2d_848k(%u) c2d_424k(%u) c2d_212k(%u)  d2c_848k(%u) d2c_424k(%u) d2c_212k(%u)\n",
                info->bitrate_same, info->bitrate_c2d_848k, info->bitrate_c2d_424k, info->bitrate_c2d_212k,
                info->bitrate_d2c_848k, info->bitrate_d2c_424k, info->bitrate_d2c_212k);
    }

    return 0;
}

int typeb_halt(card_info_t *info)
{
    u8 send[5];
    u8 recv[PCD_FIFO_SIZE];
    int len;


    if (NULL == info)
        return -1;

    pcd_set_timer(info->fwt);

    send[0] = 0x50;
    memcpy(&send[1], info->pupi, 4);
    DEBUG("  ==> HLTB: %02x %02x %02x %02x %02x", send[0], send[1], send[2], send[3], send[4]);

    len = pcd_send(send, 5 * 8, recv, sizeof(recv), 0, NULL);
    if ((8 != len) || (0 != recv[0]))
    {
        ERROR("halt b failed !");
        return -1;
    }
    DEBUG("<==   HLTB OK");

    return 0;
}

int typeb_attrib(card_info_t *info)
{
    ATTRIB_t attrib;
    u8 buf[PCD_FIFO_SIZE];
    int len;
    int i;


    if (NULL == info)
        return -1;

    pcd_set_timer(info->fwt);

    attrib.cmd = 0x1D;
    memcpy(attrib.id, info->pupi, 4);
    attrib.min_tr0 = 0;
    attrib.min_tr1 = 0;
    attrib.eof = 0;
    attrib.sof = 0;
    attrib.rfu0 = 0;
    attrib.bitrate_c2d = ATTRIB_BITRATE_106K;
    attrib.bitrate_d2c = ATTRIB_BITRATE_106K;
    attrib.fsdi = get_fsi(PCD_FIFO_SIZE); //身份证需要设置为8
    attrib.rfu1 = 0;
    attrib.min_tr2 = info->min_tr2; //???
    attrib.spt_14443_4 = 1;
    attrib.rfu2 = 0;
    if (info->spt_cid)
        attrib.cid = info->cid;
    else
        attrib.cid = 0;
    DEBUG("  ==> ATTRIB: %02x  %02x %02x %02x %02x  %02x %02x %02x %02x", \
          ((u8*)&attrib)[0], ((u8*)&attrib)[1], ((u8*)&attrib)[2], ((u8*)&attrib)[3], \
          ((u8*)&attrib)[4], ((u8*)&attrib)[5], ((u8*)&attrib)[6], ((u8*)&attrib)[7], ((u8*)&attrib)[8]);
    DEBUG("          TR0:%u, TR1:%u, EOF:%u, SOF:%u, bitrate c2d:%u, d2c:%u, FSDI:%u, TR2:%u, 14443-4:%u, CID:%u", \
           attrib.min_tr0, attrib.min_tr1, attrib.eof, attrib.sof, attrib.bitrate_c2d, attrib.bitrate_d2c, \
           attrib.fsdi, attrib.min_tr2, attrib.spt_14443_4, attrib.cid);

    len = pcd_send((u8 *)&attrib, sizeof(attrib) * 8, buf, sizeof(buf), 0, NULL);
    if ((8 > len) || (len % 8))
    {
        ERROR("attrib failed(%d)", len);
        return -1;
    }
    len /= 8;
    if (iso14443_debug)
    {
        printf("--TypeAB-- <==   ATTRIB(%d):", len);
        for (i = 0; i < len; i++)
            printf(" %02x", buf[i]);
        printf(", MBL:%u, CID:%u", buf[0] >> 4, buf[0] & 0x0F);
        printf("\n");
    }
    if (info->spt_cid && (info->cid != (buf[0] & 0x0F)))
    {
        ERROR("attrib cid err !");
        return -1;
    }

    usleep(info->sfg * 1000);
    return 0;
}

//读取身份证uid专用命令，attrib时需要设置fsdi=8(fsd=256)
int typeb_read_idcard_uid(card_info_t *info, u8 *uid)
{
    u8 recv[PCD_FIFO_SIZE];
    int len;


    if ((NULL == info) || (NULL == uid))
        return -1;

    pcd_set_timer(info->fwt);

    DEBUG("  ==> read idcard uid: 00 36 00 00 08");
    len = pcd_send((u8*)"\x02\x36\x00\x00\x08", 5 * 8, recv, sizeof(recv), 0, NULL);
    if (80 != len)
    {
        ERROR("read idcard uid failed(%d) !", len);
        return -1;
    }
    DEBUG("<==   idcard uid: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", \
           recv[0], recv[1], recv[2], recv[3], recv[4], recv[5], recv[6], recv[7], recv[8], recv[9]);

    return 0;
}






int typeab_send(card_info_t *info, const u8 *send, u8 send_len, u8 *recv, u8 recv_len)
{
    u8 s[PCD_FIFO_SIZE] = {0};
    u8 r[PCD_FIFO_SIZE] = {0};
    I_BLOCK_t *send_iblock = (I_BLOCK_t *)s;
    I_BLOCK_t *recv_iblock = (I_BLOCK_t *)r;
    R_BLOCK_t *recv_rblock = (R_BLOCK_t *)r;
    unsigned int prologue_len = 0;
    int len;
    int i;


    if ((NULL == info) || (NULL == send) || (0 == send_len) || (NULL == recv) || (0 == recv_len))
    {
        ERROR("%s: invalid param, info:0x%p send:0x%p send_len:%u recv:0x%p recv_len:%u", \
               __func__, info, send, send_len, recv, recv_len);
        return -1;
    }

    pcd_enable_crc(1);
    pcd_set_timer(info->fwt);

    //if (info->fs < PCD_FIFO_SIZE) ..............................
    send_iblock->block_type = BLOCK_TYPE_I;
    send_iblock->rsv0_set0  = 0;
    send_iblock->cid        = !!(info->spt_cid);
    send_iblock->rsv1_set1  = 1;

    for (i = 0;; i++)
    {
        send_iblock->block_num = !!(info->block_num);
        prologue_len = 1;
        if (info->spt_cid && (0 != info->cid))
        {
            ((BLOCK_CID_t *)(&s[prologue_len]))->power_level = 0;
            ((BLOCK_CID_t *)(&s[prologue_len]))->rfu = 0;
            ((BLOCK_CID_t *)(&s[prologue_len]))->cid = info->cid;
            prologue_len++;
        }
        //NAD只在第一个chaining block中传输
        if ((info->spt_nad) && (0 == i))
        {
            ((BLOCK_NAD_t *)(&s[prologue_len]))->dad = info->nadd;
            ((BLOCK_NAD_t *)(&s[prologue_len]))->sad = info->nads;
            prologue_len++;
        }

        if (PCD_FIFO_SIZE >= (prologue_len + send_len))
        {
            send_iblock->chaining = 0;
            memcpy(s + prologue_len, send, send_len);
            break;
        }

        send_iblock->chaining = 1;
        memcpy(s + prologue_len, send, PCD_FIFO_SIZE - prologue_len);
        DEBUG("  ==> I-BLOCK len(%u), prologue(%u): %02x %02x %02x", \
               PCD_FIFO_SIZE, prologue_len, s[0], s[1], s[2]);
        len = pcd_send(s, PCD_FIFO_SIZE * 8, r, sizeof(r), 0, NULL);
        send += PCD_FIFO_SIZE - prologue_len;
        send_len -= PCD_FIFO_SIZE - prologue_len;
        if ((0 >= len) || (len % 8))
        {
            ERROR("%s: pcd_send failed(%d) !", __func__, len);
            return -1;
        }
        len /= 8;
        if (BLOCK_TYPE_R != recv_rblock->block_type)
        {
            ERROR("%s: not recv R-Block(%d): %02x %02x", __func__, len, r[0], r[1]);
            return -1;
        }
        if (   (0 != recv_rblock->nak)
            || (1 != recv_rblock->rsv0_set1)
            || (0 != recv_rblock->rsv1_set0)
            || (1 != recv_rblock->rsv2_set1)
            || (send_iblock->block_num != recv_rblock->block_num))
        {
            ERROR("%s: R-Block error: %02x", __func__, r[0]);
            return -1;
        }
        info->block_num = !(info->block_num);
        if ((0 == recv_rblock->cid) && (1 != len))
        {
            ERROR("%s: R-Block(cid=0) len(%d) is not 1 !", __func__, len);
            return -1;
        }
        if (1 == recv_rblock->cid)
        {
            if (2 != len)
            {
                ERROR("%s: R-Block(cid=1) len(%d) is not 2 !", __func__, len);
                return -1;
            }
            if (info->cid != ((BLOCK_CID_t *)(&r[1]))->cid)
            {
                ERROR("%s: R-Block cid:%02x, but expect %02x !", __func__, \
                      ((BLOCK_CID_t *)(&r[1]))->cid, \
                      info->cid);
                return -1;
            }
            info->power_level = ((BLOCK_CID_t *)(&r[1]))->power_level;
            DEBUG("<==   R-BLOCK(2): %02x %02x, power_level:%u", r[0], r[1], info->power_level);
        }
        else
            DEBUG("<==   R-BLOCK(1): %02x", r[0]);
    }

    DEBUG("  ==> I-BLOCK len(%d), prologue(%u): %02x %02x %02x", \
           prologue_len + send_len, prologue_len, s[0], s[1], s[2]);
    len = pcd_send(s, (prologue_len + send_len) * 8, r, sizeof(r), 0, NULL);
    if ((0 >= len) || (len % 8))
    {
        ERROR("%s: pcd_send failed(%d) !", __func__, len);
        return -1;
    }
    len /= 8;
    if (BLOCK_TYPE_I != recv_iblock->block_type)
    {
        ERROR("%s: not recv I-Block(%d): %02x %02x !", __func__, len, r[0], r[1]);
        return -1;
    }
    prologue_len = 1;
    //暂时没做chaining接收
    if (   (0 != recv_iblock->chaining)
        || (0 != recv_iblock->rsv0_set0)
        || (1 != recv_iblock->rsv1_set1)
        || (send_iblock->block_num != recv_iblock->block_num))
    {
        ERROR("%s: I-Block error: %02x", __func__, r[0]);
        return -1;
    }
    info->block_num = !(info->block_num);
    if (1 == recv_iblock->cid)
    {
        if (2 > len)
        {
            ERROR("%s: I-Block(cid=1) len(%d) is not >= 2 !", __func__, len);
            return -1;
        }
        if (info->cid != ((BLOCK_CID_t *)(&r[1]))->cid)
        {
            ERROR("%s: I-Block cid:%02x, but expect %02x !", __func__, \
                  ((BLOCK_CID_t *)(&r[1]))->cid, info->cid);
            return -1;
        }
        info->power_level = ((BLOCK_CID_t *)(&r[1]))->power_level;
        prologue_len++;
    }
    if (1 == ((I_BLOCK_t *)(&r[0]))->nad)
    {
        if ((prologue_len + 1) > len)
        {
            ERROR("%s: I-Block(nad=1) len(%d) error !", __func__, len);
            return -1;
        }
        prologue_len++;
    }
    DEBUG("<==   I-BLOCK len(%d), prologue(%u): %02x %02x %02x, (power_level:%u)", \
           len, prologue_len, r[0], r[1], r[2], ((BLOCK_CID_t *)(&r[1]))->power_level);

    if (recv_len > (len - prologue_len))
        recv_len = len - prologue_len;
    memcpy(recv, r + prologue_len, recv_len);

    return (len - prologue_len);
}


int typeab_deselect(card_info_t *info)
{
    u8 s[2] = {0};
    u8 r[2] = {0};
    S_BLOCK_t *send_sblock = (S_BLOCK_t *)s;
    int len;


    pcd_enable_crc(1);
    pcd_set_timer(5);

    send_sblock->block_type = BLOCK_TYPE_S;
    send_sblock->desel_wtx = S_BLOCK_DESELECT;
    send_sblock->cid = !!(info->spt_cid);
    send_sblock->rsv0 = 0;
    send_sblock->not_param = 1;
    send_sblock->rsv1 = 0;
    if (info->spt_cid && (0 != info->cid))
    {
        s[1] = info->cid;
        DEBUG("  ==> DESELECT: %02x %02x", s[0], s[1]);
        len = pcd_send(s, 2 * 8, r, sizeof(r), 0, NULL);
        if ((16 != len) || (s[0] != r[0]) || (s[1] != r[1]))
        {
            ERROR("DESELECT failed(%d): %02x %02x", len, r[0], r[1]);
            return -1;
        }
    }
    else
    {
        DEBUG("  ==> DESELECT: %02x", s[0]);
        len = pcd_send(s, 1 * 8, r, sizeof(r), 0, NULL);
        if ((8 != len) || (s[0] != r[0]))
        {
            ERROR("DESELECT failed(%d): %02x", len, r[0]);
            return -1;
        }
    }

    DEBUG("<==   DESELECT OK");
    return 0;
}

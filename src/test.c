#include <stdio.h>
#include "fm17550.h"
#include "iso7816.h"
#include "iso14443.h"




int main(void)
{
    card_info_t info;
    ATQB_t atqb;


    pcd_init();
    init_card_info(&info, 1);

    do {
        printf("\n########## Type A ##########\n");

        if (pcd_set_mode(PCD_MODE_TYPEA))
            break;

        if (typea_activate(&info))
            break;
        printf("UID(%u): %02x %02x %02x %02x\n", info.uid_len, info.uid[0], info.uid[1], info.uid[2], info.uid[3]);

        if (typea_rats(&info))
            break;
    } while(0);

    do {
        printf("\n########## Type B ##########\n");

        if (pcd_set_mode(PCD_MODE_TYPEB))
            break;

        if (typeb_request(&info, 1, 0, 0, &atqb))
            break;
        printf("PUPI: %02x %02x %02x %02x\n", info.pupi[0], info.pupi[1], info.pupi[2], info.pupi[3]);

        if (typeb_attrib(&info))
            break;
    } while(0);

    pcd_set_mode(PCD_MODE_HALT);
    pcd_deinit();
    return 0;
}

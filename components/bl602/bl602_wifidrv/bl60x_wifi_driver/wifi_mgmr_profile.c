/**
 ****************************************************************************************
 *
 * @file wifi_mgmr_profile.c
 * Copyright (C) Bouffalo Lab 2016-2018
 *
 ****************************************************************************************
 */

#include <string.h>

#include "wifi_mgmr.h"
#include "wifi_mgmr_profile.h"

int wifi_mgmr_profile_add(wifi_mgmr_t *mgmr, wifi_mgmr_profile_msg_t *profile_msg, int index)
{
    int i;
    wifi_mgmr_profile_t *profile;

    profile = NULL;

    if (-1 == index) {
        /*use default index for profile*/
        profile = &(mgmr->profiles[0]);
    } else {
        /*scan free index for profile*/
        for (i = 0; i < sizeof(mgmr->profiles)/sizeof(mgmr->profiles[0]); i++) {
            if (0 == mgmr->profiles[i].isUsed) {
                profile = &(mgmr->profiles[i]);
                
                mgmr->profile_active_index = i;
                os_printf("[WF][PF] Using free profile, idx is @%d\r\n", i);
                break;
            }
        }
    }
    if (NULL == profile) {
        return -1;
    }
    memset(profile, 0, sizeof(*profile));
    profile->isUsed = 1;//Set isUsed firstly
    profile->isActive = 0;//By default, isActive is 0
    profile->ssid_len = profile_msg->ssid_len;
    profile->psk_len = profile_msg->psk_len;
    profile->pmk_len = profile_msg->pmk_len;
    profile->priority = 255;
    memcpy(profile->ssid, profile_msg->ssid, sizeof(profile->ssid));
    memcpy(profile->psk, profile_msg->psk, sizeof(profile->psk));
    memcpy(profile->pmk, profile_msg->pmk, sizeof(profile->pmk));
    memcpy(profile->mac, profile_msg->mac, sizeof(profile->mac));
    profile->dhcp_use = profile_msg->dhcp_use;
    profile->net_conf.ip   = profile_msg->net_conf.ip;
    profile->net_conf.mask = profile_msg->net_conf.mask;
    profile->net_conf.gw   = profile_msg->net_conf.gw;
    profile->net_conf.dns1 = profile_msg->net_conf.dns1;
    profile->net_conf.dns2 = profile_msg->net_conf.dns2;
    profile->ip_lease_time = profile_msg->ip_lease_time;
    memcpy(profile->net_conf.gw_mac, profile_msg->net_conf.gw_mac, sizeof(profile->mac));

    return 0;
}

int wifi_mgmr_profile_del(wifi_mgmr_t *mgmr, char *ssid, int len)
{
    int i;
    wifi_mgmr_profile_t *profile;

    profile = NULL;
    for (i = 0; i < sizeof(mgmr->profiles)/sizeof(mgmr->profiles[0]); i++) {
        if (1 == mgmr->profiles[i].isUsed &&
                len == mgmr->profiles[i].ssid_len &&
                0 == memcmp(mgmr->profiles[i].ssid, ssid, len)) {
            profile = &(mgmr->profiles[i]);
            if (i == mgmr->profile_active_index) {
                mgmr->profile_active_index = -1;
            }
            os_printf("[WF][PF] Free profile, idx is @%d\r\n", i);
            break;
        }
    }
    if (NULL == profile) {
        return -1;
    }
    memset(profile, 0, sizeof(*profile));

    return 0;
}

int wifi_mgmr_profile_get(wifi_mgmr_t *mgmr, wifi_mgmr_profile_msg_t *profile_msg)
{
    int i;
    wifi_mgmr_profile_t *profile;

    profile = NULL;
    for (i = 0; i < sizeof(mgmr->profiles)/sizeof(mgmr->profiles[0]); i++) {
        if (1 == mgmr->profiles[i].isUsed) {
            profile = &(mgmr->profiles[i]);
            os_printf("[WF][PF] Using profile, idx is @%d\r\n", i);
            break;
        }
    }
    if (NULL == profile) {
        return -1;
    }

    memset(profile_msg, 0, sizeof(*profile_msg));
    profile_msg->ssid_len = profile->ssid_len;
    profile_msg->psk_len = profile->psk_len;
    profile_msg->pmk_len = profile->pmk_len;
    profile_msg->dhcp_use = profile->dhcp_use;
    memcpy(profile_msg->ssid, profile->ssid, sizeof(profile->ssid));
    memcpy(profile_msg->psk, profile->psk, sizeof(profile->psk));
    memcpy(profile_msg->pmk, profile->pmk, sizeof(profile->pmk));
    memcpy(profile_msg->mac, profile->mac, sizeof(profile->mac));
    profile_msg->net_conf.ip = profile->net_conf.ip;
    profile_msg->net_conf.mask = profile->net_conf.mask;
    profile_msg->net_conf.gw = profile->net_conf.gw;
    profile_msg->net_conf.dns1 = profile->net_conf.dns1;
    profile_msg->net_conf.dns2 = profile->net_conf.dns2;
    profile_msg->timestamp = profile->timestamp;
    profile_msg->ip_lease_time = profile->ip_lease_time;

    return 0;
}

int wifi_mgmr_valid_profile_get(wifi_mgmr_t *mgmr)
{
    int i, index = -1;
    for (i = 0; i < sizeof(mgmr->profiles)/sizeof(mgmr->profiles[0]); i++) {
        if (1 == mgmr->profiles[i].isUsed) {
            index = i;
            os_printf("valid profile idx is @%d\r\n", i);
            break;
        }
    }

    return index;
}

int wifi_mgmr_profile_update(wifi_mgmr_t *mgmr, uint8_t dhcp_use)
{
    int i;
    wifi_mgmr_profile_t *profile;

    profile = NULL;
    for (i = 0; i < sizeof(mgmr->profiles)/sizeof(mgmr->profiles[0]); i++) {
        if (1 == mgmr->profiles[i].isUsed) {
            profile = &(mgmr->profiles[i]);
            os_printf("[WF][PF] Using profile, idx is @%d\r\n", i);
            break;
        }
    }
    if (NULL == profile) {
        return -1;
    }

    profile->dhcp_use = dhcp_use;
    if (dhcp_use) {
        memset(&profile->net_conf, 0, sizeof(struct wifi_mgmr_ipgot_msg));
    }

    return 0;
}

wifi_mgmr_profile_t* __lookup_profile(wifi_mgmr_t *mgmr, int index)
{
    wifi_mgmr_profile_t *profile = NULL;

    if (-1 == index) {
        /*default profile*/
        profile = &(mgmr->profiles[0]);
    } else {
        if (index >= 0 && index < sizeof(mgmr->profiles)/sizeof(mgmr->profiles[0])) {
            profile = &(mgmr->profiles[index]);
        }
    }
    return profile;
}

int wifi_mgmr_profile_autoreconnect_is_enabled(wifi_mgmr_t *mgmr, int index)
{
#if 0
    wifi_mgmr_profile_t *profile;
    
    profile = __lookup_profile(mgmr, index);
    if (NULL == profile) {
        return -1;
    }

    return profile->no_autoconnect ? 0 : 1;
#else
    return mgmr->disable_autoreconnect ? 0 : 1;
#endif
}

int wifi_mgmr_profile_autoreconnect_disable(wifi_mgmr_t *mgmr, int index)
{
#if 0
    wifi_mgmr_profile_t *profile;

    profile = __lookup_profile(mgmr, index);
    if (NULL == profile) {
        return -1;
    }
    profile->no_autoconnect = 1;
    return 0;
#else
    mgmr->disable_autoreconnect = 1;

    return 0;
#endif
}

int wifi_mgmr_profile_autoreconnect_enable(wifi_mgmr_t *mgmr, int index)
{
#if 0
    wifi_mgmr_profile_t *profile;

    profile = __lookup_profile(mgmr, index);
    if (NULL == profile) {
        return -1;
    }
    profile->no_autoconnect = 0;

    return 0;
#else
    mgmr->disable_autoreconnect = 0;

    return 0;
#endif
}

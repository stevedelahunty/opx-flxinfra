#include <stdio.h>
#include "onlp.h"
#include <onlp/sys.h>
#include <onlp/fan.h>
#include <onlp/sfp.h>
#include <dlfcn.h>

static onlp_oid_t fan_oid_table[ONLP_OID_TABLE_SIZE];
static onlp_fan_info_t fan_info_table[ONLP_OID_TABLE_SIZE];
static int flag[ONLP_OID_TABLE_SIZE] = {0};
onlp_sys_info_t si;
void *handle;

int (*onlpInit)(void);
int (*onlpSysInfoGet)(onlp_sys_info_t*);
int (*onlpFanInfoGet)(onlp_oid_t, onlp_fan_info_t*);

/* SFP API */
void (*onlpSfpBitmapInit)(onlp_sfp_bitmap_t*);
int (*onlpSfpBitmapGet)(onlp_sfp_bitmap_t*);
int (*onlpSfpIsPresent)(int);
int (*onlpSfpPresenceBitmapGet)(onlp_sfp_bitmap_t*);
int (*onlpSfpRxLosBitmapGet)(onlp_sfp_bitmap_t*);
int (*onlpSfpControlGet)(int, onlp_sfp_control_t, int*);
int (*onlpSfpEepromRead)(int, uint8_t**);

int
loadOnlpSym()
{
    char *error;

    handle = dlopen ("/lib/x86_64-linux-gnu/libonlp.so", RTLD_LAZY);
    if (!handle) {
        printf(dlerror(), stderr);
        return -1;
    }

	onlpInit = dlsym(handle, "onlp_init");
    if ((error = dlerror()) != NULL)  {
        printf(error, stderr);
        return -1;
    }

	onlpFanInfoGet = dlsym(handle, "onlp_fan_info_get");
	if ((error = dlerror()) != NULL)  {
		printf(error, stderr);
		return -1;
	}

    onlpSysInfoGet = dlsym(handle, "onlp_sys_info_get");
    if ((error = dlerror()) != NULL)  {
        printf(error, stderr);
		return -1;
    }

#if 0
    onlpSfpBitmapInit = dlsym(handle, "onlp_sfp_bitmap_init");
    if ((error = dlerror()) != NULL)  {
        printf(error, stderr);
		return -1;
    }
#endif

    onlpSfpBitmapGet = dlsym(handle, "onlp_sfp_bitmap_get");
    if ((error = dlerror()) != NULL)  {
        printf(error, stderr);
		return -1;
    }

    onlpSfpIsPresent = dlsym(handle, "onlp_sfp_is_present");
    if ((error = dlerror()) != NULL)  {
        printf(error, stderr);
		return -1;
    }

    onlpSfpPresenceBitmapGet = dlsym(handle, "onlp_sfp_presence_bitmap_get");
    if ((error = dlerror()) != NULL)  {
        printf(error, stderr);
		return -1;
    }

    onlpSfpRxLosBitmapGet = dlsym(handle, "onlp_sfp_rx_los_bitmap_get");
    if ((error = dlerror()) != NULL)  {
        printf(error, stderr);
		return -1;
    }

    onlpSfpEepromRead = dlsym(handle, "onlp_sfp_eeprom_read");
    if ((error = dlerror()) != NULL)  {
        printf(error, stderr);
		return -1;
    }

    onlpSfpControlGet = dlsym(handle, "onlp_sfp_control_get");
    if ((error = dlerror()) != NULL)  {
        printf(error, stderr);
		return -1;
    }

    return 0;
}

int
Init()
{
	int ret = 0;
	int i = 0;
	onlp_oid_t* oidp;

	ret = loadOnlpSym();
	if (ret < 0) {
		printf("Error loading the ONLP symbols");
		return -1;
	}

	if (onlpInit == NULL) {
		printf("onlpInit ptr to onlp_init() is NULL\n");
		return -1;
	}
	(*onlpInit)();

    if (onlpSysInfoGet == NULL) {
		printf("onlpSysInfoGet ptr to onlp_sys_info_get is NULL\n");
		return -1;
	}
	ret = (*onlpSysInfoGet)(&si);
	if (ret < 0) {
		printf("onlp_sys_info_get() failed during init. Return Value:%d\n", ret);
		return ret;
	}

	ONLP_OID_TABLE_ITER_TYPE(si.hdr.coids, oidp, FAN) {
		fan_oid_table[i++] = *oidp;
	}

	return 0;
}

int
GetMaxNumOfFans()
{
	return ONLP_OID_TABLE_SIZE;
}

int
GetAllFanState(fan_info_t *info, int count)
{
	printf("Count = %d", count);
	int i = 0;
	if (onlpFanInfoGet == NULL) {
		printf("onlpFanInfoGet ptr to onlp_fan_info_get is NULL\n");
		return -1;
	}
	for (i = 0; i < count; i++) {
		onlp_fan_info_t fi;
		int fid = ONLP_OID_ID_GET(fan_oid_table[i]);
		info[i].FanId = fid;
		info[i].valid = 0;
		if (fan_oid_table[i] == 0) {
			continue;
		}
		if ((*onlpFanInfoGet)(fan_oid_table[i], &fi) < 0) {
			printf("Failure retreiving status of Fan Id %d\n", fid);
			continue;
		}
		printf("Fan Id : %d Fan Status : %u\n", fid, fi.status);
		info[i].valid = 1;
		if (!(fi.status & 0x1)) {
			info[i].Status = FAN_STATUS_MISSING;
		} else if (fi.status & ONLP_FAN_STATUS_FAILED) {
			info[i].Status = FAN_STATUS_FAILED;
		} else if (fi.status & 0x1) {
			info[i].Status = FAN_STATUS_PRESENT;
		}

		info[i].Speed = fi.rpm;
		info[i].Direction = (fi.status & (1 << 2)) ? FAN_DIR_B2F : FAN_DIR_F2B;
		if (fi.rpm > 0) {
			info[i].Mode = FAN_MODE_ON;
		} else {
			info[i].Mode = FAN_MODE_OFF;
		}

		strncpy(info[i].Model, fi.model, DEFAULT_SIZE);
		strncpy(info[i].SerialNum, fi.serial, DEFAULT_SIZE);
	}
	return 0;
}

int
GetFanState(fan_info_t *info, int fanId)
{
	int i = 0, fid;
	if (onlpFanInfoGet == NULL) {
		printf("onlpFanInfoGet ptr to onlp_fan_info_get is NULL\n");
		return -1;
	}
	for (i = 0; i < sizeof(fan_oid_table)/sizeof(fan_oid_table[0]); i++) {
        onlp_fan_info_t fi;
		fid = ONLP_OID_ID_GET(fan_oid_table[i]);
		if (fid != fanId) {
			continue;
		}
		info[0].FanId = fid;
		info[0].valid = 0;
		if (fan_oid_table[i] == 0) {
			continue;
		}
		if ((*onlpFanInfoGet)(fan_oid_table[i], &fi) < 0) {
			printf("Failure retreiving status of Fan Id %d\n", fid);
			continue;
		}
		printf("Fan Id : %d Fan Status : %u\n", fid, fi.status);
		info[0].valid = 1;
		if (!(fi.status & 0x1)) {
			info[0].Status = FAN_STATUS_MISSING;
		} else if (fi.status & ONLP_FAN_STATUS_FAILED) {
			info[0].Status = FAN_STATUS_FAILED;
		} else if (fi.status & 0x1) {
			info[0].Status = FAN_STATUS_PRESENT;
		}

		info[0].Speed = fi.rpm;
		info[0].Direction = (fi.status & (1 << 2)) ? FAN_DIR_B2F : FAN_DIR_F2B;
		if (fi.rpm > 0) {
			info[0].Mode = FAN_MODE_ON;
		} else {
			info[0].Mode = FAN_MODE_OFF;
		}

		strncpy(info[0].Model, fi.model, DEFAULT_SIZE);
		strncpy(info[0].SerialNum, fi.serial, DEFAULT_SIZE);
		return 0;
	}
	return -1;
}

int
GetPlatformState(sys_info_t *info_p)
{
    int ret = 0;

    onlp_sys_info_t onlp_info;

    if (onlpSysInfoGet == NULL) {
		printf("onlpSysInfoGet ptr to onlp_sys_info_get is NULL\n");
		return -1;
	}
	ret = (*onlpSysInfoGet)(&onlp_info);
	if (ret < 0) {
		printf("onlp_sys_info_get() failed(%d)\n", ret);
		return ret;
	}

    strncpy(info_p->product_name, onlp_info.onie_info.product_name, DEFAULT_SIZE);
    strncpy(info_p->serial_number, onlp_info.onie_info.serial_number, DEFAULT_SIZE);
    strncpy(info_p->manufacturer, onlp_info.onie_info.manufacturer, DEFAULT_SIZE);
    strncpy(info_p->vendor, onlp_info.onie_info.vendor, DEFAULT_SIZE);
    strncpy(info_p->platform_name, onlp_info.onie_info.platform_name, DEFAULT_SIZE);
    strncpy(info_p->onie_version, onlp_info.onie_info.onie_version, DEFAULT_SIZE);
    strncpy(info_p->label_revision, onlp_info.onie_info.label_revision, DEFAULT_SIZE);

    return ret;
}

int
GetSfpCnt()
{
    int rt;
    onlp_sfp_bitmap_t bMap;

    return 32;
    // TODO
    memset(&bMap, 0, sizeof(onlp_sfp_bitmap_t));
    if (onlpSfpBitmapGet == NULL) {
		printf("onlpSfpBitmapGet NULL\n");
		return 0;
	}
    rt = (*onlpSfpBitmapGet)(&bMap);
    if (rt < 0)
        return 0;

    return AIM_BITMAP_COUNT(&bMap);
}

SFP_RET
GetSfpState(sfp_info_t *sfpInfo, int sfpId)
{
    int rt, rval;
    unsigned char *eeprom_p = NULL;

    rt = (*onlpSfpIsPresent)(sfpId);
    if (rt < 0) {
		printf("%s :(%d) INVALID SFP(%d) \n",__FUNCTION__, rt, sfpId);
		return SFP_ERROR;
    }

    memset(sfpInfo, 0, sizeof(sfp_info_t));
    if (!rt)
        return SFP_MISSING;

    sfpInfo->sfp_id = sfpId;
    sfpInfo->sfp_present = 1;

    /* RX LOS */
    (*onlpSfpControlGet)(sfpId, ONLP_SFP_CONTROL_RX_LOS, &rval);
    sfpInfo->sfp_los = rval?1:0;

    rt = (*onlpSfpEepromRead)(sfpId, &eeprom_p);
    if (rt < 0) {
		printf("%s :(%d) Eeprom read failed for SFP(%d)\n",__FUNCTION__, rt, sfpId);
		return SFP_ERROR;
    }
    memcpy(sfpInfo->eeprom, eeprom_p, 256);
    return SFP_OK;
}

int
DeInit() {
	dlclose(handle);
}

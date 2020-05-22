/** @file
 *  @brief HoG Service sample
 */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr.h>
#include "byteorder.h"
#include "bluetooth.h"
#include "hci_host.h"
#include "conn.h"
#include "uuid.h"
#include "gatt.h"
#include "hog.h"
#include "log.h"
#include "voice.h"

enum {
	HIDS_REMOTE_WAKE = BIT(0),
	HIDS_NORMALLY_CONNECTABLE = BIT(1),
};

struct hids_info {
	u16_t version; /* version number of base USB HID Specification */
	u8_t code; /* country HID Device hardware is localized for. */
	u8_t flags;
} __packed;

struct hids_report {
	u8_t id; /* report id */
	u8_t type; /* report type */
} __packed;

struct hids_remote_key {
    u8_t hid_page;
    u16_t hid_usage;
} __packed;

static struct hids_info info = {
	.version = 256,
	.code = 0x00,
	.flags = 0,
};

enum {
	HIDS_INPUT = 0x01,
	HIDS_OUTPUT = 0x02,
	HIDS_FEATURE = 0x03,
};

#define HID_REPORT_REGISTER(_id, _type)		\
{						\
	.id = _id,	\
	.type =_type,	\
}

static struct hids_report report[7]= {

	/*input_1 */	
	HID_REPORT_REGISTER(0x01,HIDS_INPUT),
	/*input_2 */
	HID_REPORT_REGISTER(0x02,HIDS_INPUT),
	/*input_3 */
	HID_REPORT_REGISTER(0xF0,HIDS_INPUT),
	/*input_4 */	
	HID_REPORT_REGISTER(0xF1,HIDS_INPUT),
	/*output_1 */
	HID_REPORT_REGISTER(0xF2,HIDS_OUTPUT),
	/*output_2 */
	HID_REPORT_REGISTER(0xF3,HIDS_OUTPUT),
	/*feature */
	HID_REPORT_REGISTER(0xF4,HIDS_FEATURE),

};

static ssize_t write_output(struct bt_conn *conn,
							  const struct bt_gatt_attr *bt_attr, 
							  void *buf,
			 				  u16_t len, 
			 				  u16_t offset,
			 				  u8_t flags);

static struct bt_gatt_ccc_cfg input_ccc_cfg[BT_GATT_CCC_MAX] = {};
volatile u8_t Voicekey_is_press = 0;
static u8_t simulate_input;
static u8_t ctrl_point;
static u8_t report_map[] = {
	
	       /*
	         * Keyboard 
	         */      
		0x05, 0x01, /* Usage Page (Generic Desktop Ctrls) */
		0x09, 0x06, /* Usage (Keyboard) */
		0xA1, 0x01, /* Collection (Application) */
		0x05, 0x07, /*   Usage (Keyboard) */
		0x85, 0x01, /*   Report Id (1) */
		0x95, 0x03, /*   Report Count (3) */
		0x75, 0x08, /*   Report Size (1) */
		0x15, 0x00, /*   Logical minimum (0)*/
		0x25, 0xFF, /*   Logical maxmum (255)*/
		0x19, 0x00, /*   Usage Minimum (No event indicated)*/
		0x29, 0xFF, /*   Usage Maxmum (Reserved(0x00FF))*/
		0x81, 0x00, /*   Input (Data,Array,Absolute,Bit Field) */
		0xC0,         /* End Collection */
		
		 /*
		   *  Consumer Control
		   */
		0x05, 0x0C, /* Usage Page (Consumer) */
		0x09, 0x01, /* Usage (Consumer Control) */
		0xA1, 0x01, /* Collection (Application) */
		0x85, 0x02, /*   Report Id (2) */
		0x95, 0x02, /*   Report Count (2) */
		0x75, 0x10, /*   Report Size (16) */
		0x15, 0x00, /*   Logical minimum (0)*/
		0x26, 0x9C,0x02, /* Logical maxmum (668)*/
		0x19, 0x00, /*   Usage Minimum (Unassigned)*/
		0x2A, 0x9C, 0x02, /*Usage Maxmum (AC Distribute Vertically)*/
		0x81, 0x00, /*   Input (Data,Array,Absolute,Bit Field) */
		0xC0,       /* End Collection */

		 /*
		   *  Vendor Defined
		   */
		0x06, 0x00, 0xFF, /* Usage Page (Vendor-defined 0xFF00) */
		0x09, 0x00, /* Usage (Vendor-defined 0x0000) */
		0xA1, 0x01, /* Collection (Application) */
		0x85, 0xF0, /*   Report Id (240) */
		0x95, 0x50, /*   Report Count (80) */
		0x75, 0x08, /*   Report Size (8) */
		0x15, 0x00, /*   Logical minimum (0)*/
		0x25, 0xFF, /*   Logical maxmum (255)*/
		0x81, 0x00, /*   Input (Data,Array,Absolute,Bit Field) */
		
		/*
		  * Audio Config
		  */
		0x85, 0xF1, /*   Report Id (241) */
		0x95, 0x03, /*   Report Count (3) */
		0x75, 0x08, /*   Report Size (8) */
		0x09, 0x00, /*   Usage (Vendor-defined 0x0000) */
		0x81, 0x02, /*   Input (Data,Array,Absolute,Bit Field) */
		
		/*
		  * Audio State
                */
		0x85, 0xF2, /*   Report Id (242) */
		0x95, 0x01, /*   Report Count (1) */
		0x75, 0x08, /*   Report Size (8) */
		0x09, 0x00, /*   Usage (Vendor-defined 0x0000) */
		0x91, 0x02, /*   Output (Data,Array,Absolute,Non-volatile...) */
		
		/*
		  * Dail State
		  */
		0x85, 0xF3, /*   Report Id (243) */
		0x95, 0x0A, /*   Report Count (10) */
		0x75, 0x08, /*   Report Size (8) */
		0x09, 0x00, /*   Usage (Vendor-defined 0x0000) */
		0x91, 0x02, /*   Output (Data,Array,Absolute,Non-volatile...) */
		0xC0,       /* End Collection */

		/*
		  *  Consumer Control
		  */  
		0x05, 0x0C, /* Usage Page (Consumer) */
		0x09, 0x01, /* Usage (Consumer Control) */
		0xA1, 0x01, /* Collection (Application) */
		0x85, 0x03, /*   Report Id (3) */
		0x05, 0x01, /*   Usage Page (Generic Desktop) */
		0x09, 0x06, /*   Usage (Keyboard) */
		0xA1, 0x02, /*   Collection(logical)*/
		0x05, 0x06, /*       Usage Page(Generic Device)*/   
		0x09, 0x20, /*       Usage(Battery Strength)*/
		0x15, 0x00, /*       Logical minimum (0)*/
		0x26, 0x64,0x00, /*   Logical maxmum (100)*/
		0x75, 0x08, /*       Report Size (8) */
		0x95, 0x01, /*       Report Count (1) */
		0x81, 0x02, /*       Input (Data,Array,Absolute,Bit Field) */
		0xC0,        /*   End Collection */
		0xC0,        /* End Collection */
};

static struct hids_remote_key remote_kbd_map_tab[] = {
	
	    {HID_PAGE_KBD, KEY_RIGHT},
	    {HID_PAGE_KBD, KEY_LEFT},
	    {HID_PAGE_KBD, KEY_DOWN},
	    {HID_PAGE_KBD, KEY_UP},
	    {HID_PAGE_KBD, KEY_SELECT},
	    {HID_PAGE_KBD, KEY_BACK},
	    {HID_PAGE_CONS, KEY_FF},
	    {HID_PAGE_CONS, KEY_PREV},
	    {HID_PAGE_CONS, KEY_PLAY_PSE},
	    {HID_PAGE_CONS, KEY_MENU},
	    {HID_PAGE_CONS, KEY_HOME},
	    {HID_PAGE_CONS, KEY_VOICE},
	    {HID_PAGE_KBD, KEY_TV_PWR},    
	    {HID_PAGE_CONS, KEY_TV_INPUT},
	    {HID_PAGE_CONS, KEY_TV_MUTE},
	    {HID_PAGE_CONS, KEY_TV_VOL_INS},
	    {HID_PAGE_CONS, KEY_TV_VOL_DES},
};


static ssize_t read_info(struct bt_conn *conn,
			  const struct bt_gatt_attr *attr, void *buf,
			  u16_t len, u16_t offset)
{
		return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data,sizeof(struct hids_info));
}

static ssize_t read_report_map(struct bt_conn *conn,
			       const struct bt_gatt_attr *attr, void *buf,
			       u16_t len, u16_t offset)
{
		return bt_gatt_attr_read(conn, attr, buf, len, offset, report_map,sizeof(report_map));
}

#if defined(BFLB_BLE)
static ssize_t read_ext_report_ref(struct bt_conn *conn,
			       const struct bt_gatt_attr *attr, void *buf,
			       u16_t len, u16_t offset)			       
{
		return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data, sizeof(u16_t));
}
#endif

static ssize_t read_report(struct bt_conn *conn,
						   const struct bt_gatt_attr *attr, void *buf,
						   u16_t len, u16_t offset)
{
		return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data,sizeof(struct hids_report));
}

static void input_ccc_changed(const struct bt_gatt_attr *attr, u16_t value)
{
		simulate_input = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

static ssize_t read_input_report(struct bt_conn *conn,
								 const struct bt_gatt_attr *attr, void *buf,
								 u16_t len, u16_t offset)
{
		return bt_gatt_attr_read(conn, attr, buf, len, offset, NULL, 0);
}
	



static ssize_t write_ctrl_point(struct bt_conn *conn,
							  const struct bt_gatt_attr *attr,
							  const void *buf, u16_t len, u16_t offset,
							  u8_t flags)
{
		u8_t *value = attr->user_data;

		if (offset + len > sizeof(ctrl_point)) {
			return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
		}

		memcpy(value + offset, buf, len);

		return len;
}


static struct bt_gatt_attr bas_srv = BT_GATT_PRIMARY_SERVICE(BT_UUID_BAS);
u16_t BATTERY_LEVEL_UUID_VAL = 0x2a19;

extern struct bt_gatt_service bas;
#define BT_GATT_HIDS_EXT_REPORT(_uuid)				\
	BT_GATT_ATTRIBUTE(BT_UUID_HIDS_EXT_REPORT, BT_GATT_PERM_READ,	\
			  read_ext_report_ref, NULL, _uuid)

	/*
	  *  gatt attribute list,
	  *  
	  *  note : Macro definition include several gatt attribute 
	  *  eg : attrs[2] : BT_GATT_INCLUDE_SERVICE(x), 
	  *         attrs[3] \attrs[4] : BT_GATT_CHARACTERISTIC
	  */ 
		  
static struct bt_gatt_attr attrs[]= 
{
		/*primary server uuid*/
		BT_GATT_PRIMARY_SERVICE(BT_UUID_HIDS),
		
		/*include bas server */	
		BT_GATT_INCLUDE_SERVICE(&bas_srv),
		
		BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_INFO, 
							   BT_GATT_CHRC_READ,
		       				   BT_GATT_PERM_READ,
		       				   read_info,
		       				   NULL, 
		       				   &info),	    
		   
		BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_REPORT_MAP, 
							   BT_GATT_CHRC_READ,
		       				   BT_GATT_PERM_READ,
		       				   read_report_map, 
		       				   NULL, 
		       				   NULL),

		BT_GATT_HIDS_EXT_REPORT(&BATTERY_LEVEL_UUID_VAL),
		 /*
	 	
	         * report characteristic 
	         
	         * descriptor declaration "input"
	         
		  */	
		BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_REPORT,
							    BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
							    BT_GATT_PERM_READ_AUTHEN,
							    read_input_report, 
							    NULL, 
							    NULL),	
							    
		BT_GATT_CCC(input_ccc_cfg, input_ccc_changed),
		
		BT_GATT_DESCRIPTOR(BT_UUID_HIDS_REPORT_REF, 
							   BT_GATT_PERM_READ,
			  				   read_report, 
			  				   NULL, 
			  				   &report[0]),
	 	 /*
	 	
	         * report characteristic 
	         
	         * descriptor declaration "input_2"
	         
		  */		   
		BT_GATT_CHARACTERISTIC( BT_UUID_HIDS_REPORT,
		      					    BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		       				    BT_GATT_PERM_READ_AUTHEN,
		       	                         read_input_report, 
		       	                         NULL,
		       	                         NULL),
		       	                         
		BT_GATT_CCC(input_ccc_cfg, input_ccc_changed),
		 
		BT_GATT_DESCRIPTOR(BT_UUID_HIDS_REPORT_REF, 
	 					   	   BT_GATT_PERM_READ,
	      				                 read_report,
	      				           	   NULL, 
	      				           	    &report[1]),

	 	 /*
	 	
	         * report characteristic 
	         
	         * descriptor declaration "input_3"
	         
		  */		
		BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_REPORT,
	       				           	   BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY ,
	                                                   BT_GATT_PERM_READ_AUTHEN ,
	                                             	   read_input_report, 
	                                                   NULL, 
	                                                   NULL),
	                                             
		BT_GATT_CCC(input_ccc_cfg, input_ccc_changed),
		
	 	BT_GATT_DESCRIPTOR(BT_UUID_HIDS_REPORT_REF, 
							   BT_GATT_PERM_READ,
	       					   read_report,
	       					   NULL, 
	       					    &report[2]),
	     	 /*
	     	 
	         * report characteristic 
	         
	         * descriptor declaration "input_4"
	         
		  */		   				
		BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_REPORT,
		       					   BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
		       					   BT_GATT_PERM_READ_AUTHEN,
		       					   read_input_report,
		       					   NULL, 
		       					   NULL),
		       					   
		BT_GATT_CCC(input_ccc_cfg, input_ccc_changed),
		
		BT_GATT_DESCRIPTOR(BT_UUID_HIDS_REPORT_REF, 
							   BT_GATT_PERM_READ,
	       					   read_report, 
	       					   NULL, 
	       					    &report[3]),
	       /*
	       
	         * report characteristic 
	         
	         * descriptor declaration "output_1"
	         
		  */						   
		BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_REPORT,
		      						   BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
		       					   BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE,
		       					   read_input_report, 
		       					   write_output, 
		       					   NULL),
		       					   
		BT_GATT_DESCRIPTOR(BT_UUID_HIDS_REPORT_REF, 
							   BT_GATT_PERM_READ ,
	                                            read_report,
	                                            NULL,
	                                            &report[4]),
	    	/*
	         * report characteristic 
	         
	         * descriptor declaration "output_2"
		  */	                                        
		BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_REPORT,
		       					   BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
		       					    BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE,
		       					   read_input_report, 
		       					   write_output, 
		       					   NULL),
		       					   
		BT_GATT_DESCRIPTOR(BT_UUID_HIDS_REPORT_REF, 
							   BT_GATT_PERM_READ,
	       					   read_report, 
	       					   NULL,
	       					   &report[5]),
	     	/*
	     	
	         * report characteristic 
	         
	         * descriptor declaration "feature"
	         
		  */	    					   
		BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_REPORT,
		       					   BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
		       					   BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE,
		       					   read_input_report,
		       					   write_output, 
		       					   NULL),
	    					   
		BT_GATT_DESCRIPTOR(BT_UUID_HIDS_REPORT_REF, 
							   BT_GATT_PERM_READ,
	       					   read_report, 
	       					   NULL, 
	       					   &report[6]),
	       /*
	       
	         * control point characteristic 
	         
		  */	
		BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_CTRL_POINT,
		       				   BT_GATT_CHRC_WRITE_WITHOUT_RESP,
		       				   BT_GATT_PERM_WRITE,
		       				   NULL, 
		       				   write_ctrl_point, 
		       				   &ctrl_point),
};

static ssize_t write_output ( struct bt_conn *conn,
							  const struct bt_gatt_attr *bt_attr, 
							  void *buf,
			 				  u16_t len, 
			 				  u16_t offset,
			 				  u8_t flags)

{
		u8_t *s_buf = (u8_t *)buf;
		u8_t v_len = 0;
		
		/*not start */
		if(*s_buf != 0x01){
			return len;
		}

		while(Voicekey_is_press){		

			if((OPUS_BUFFER->wptr != OPUS_BUFFER->rptr)){
				
				u8_t *v_buf = OPUS_BUFFER->buffer+OPUS_BUFFER->rptr;
				/* get  data length */
				v_len = *v_buf;		
				
				bt_gatt_notify(conn,&attrs[15],v_buf+1,v_len);
				OPUS_BUFFER->rptr = (OPUS_BUFFER->rptr + v_len + 1)% OPUS_BUFFER->size;

				BL70X_Delay_MS(5);
			}
		}
		
		
}

static struct bt_gatt_service hog_svc = BT_GATT_SERVICE(attrs);

int bt_hog_notify(struct bt_conn *conn, u16_t hid_usage,u8_t press)
{
		struct bt_gatt_attr *attr;
		struct hids_remote_key *remote_key = NULL;
		u8_t len = 4, data[4];
		int err;
		
		for(int i = 0; i < (sizeof(remote_kbd_map_tab)/sizeof(remote_kbd_map_tab[0])); i++){

			if(remote_kbd_map_tab[i].hid_usage == hid_usage){
				remote_key = &remote_kbd_map_tab[i];
				break;
			}
		}

		if(!remote_key)
			return EINVAL;

		if(remote_key->hid_page == HID_PAGE_KBD){
			attr = &attrs[8];
			len = 3;		
		}else if(remote_key->hid_page == HID_PAGE_CONS)
			attr = &attrs[12];
		else
			return EINVAL;	
		
		sys_put_le16(hid_usage, data);
		data[2] = 0;
		data[3] = 0;

		if(press)
			return bt_gatt_notify(conn, attr, data, len);
		else{ 	
			memset(data, 0, len);
			return bt_gatt_notify(conn, attr, data, len);
		}
}

void hog_init(void)
{
	struct bt_gatt_attr *bas_srv_attr = attrs[1].user_data;

	bt_gatt_service_register(&hog_svc);
	bas_srv_attr->handle = bas.attrs[0].handle;
		
}

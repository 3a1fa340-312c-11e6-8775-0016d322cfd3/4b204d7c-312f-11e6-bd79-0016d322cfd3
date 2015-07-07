#ifndef _R871X_TOOLS_H
#define _R871X_TOOLS_H

#include "rtl871x_mp_ioctl.h"

struct mp_status_param{
	unsigned int mode;
};

typedef struct _TXPOWERTABLE_{
	unsigned char CCK[4];
	unsigned char OFDM[4];
}TXPOWERTABLE;

typedef struct _TSSI_STRUCT_{
	unsigned char CCK[4];
	unsigned char OFDM[4];
}TSSISTRUCT;

typedef struct _EEPROM_INF_{
	unsigned char	ID[2];
	unsigned char	MACAddr[6];
	unsigned char	VID[2];
	unsigned char	PID[2];
	unsigned char	ChannPlan;
	unsigned char	EEVersion[2];
	TXPOWERTABLE	TxPower;
	TSSISTRUCT	TSSI;
	unsigned char	CAP[2];
	unsigned char	Conutry[3];
}EEPROMINF;

typedef struct _tx_desc_8712 {
	// DWORD 1
	unsigned int	txpktsize:16;
	unsigned int	offset:8;
	unsigned int	frame_type:2;
	unsigned int	ls:1;
	unsigned int	fs:1;
	unsigned int	linip:1;
	unsigned int	amsdu:1;
	unsigned int	gf:1;
	unsigned int	own:1;
	// DWORD 2
	unsigned int	macid:5;
	unsigned int	moredata:1;
	unsigned int	morefrag:1;
	unsigned int	pifs:1;
	unsigned int	qsel:5;
	unsigned int	ack_policy:2;
	unsigned int	noacm:1;
	unsigned int	non_qos:1;
	unsigned int	key_id:2;
	unsigned int	oui:1;
	unsigned int	pkt_type:1;
	unsigned int	en_desc_id:1;
	unsigned int	sec_type:2;
	unsigned int	wds:1;//padding0
	unsigned int	htc:1;//padding1
	unsigned int	pkt_offset:5;//padding_len (hw)
	unsigned int	hwpc:1;
	// DWORD 3
	unsigned int	data_retry_lmt:6;
	unsigned int	rty_lmt_en:1;
	unsigned int	rsvd:1;
	unsigned int	tsfl:4;
	unsigned int	rts_rc:6;
	unsigned int	data_rc:6;
	unsigned int	rsvd_macid:5;
	unsigned int	agg_en:1;
	unsigned int	bk:1;
	unsigned int	own_mac:1;
	// DWORD 4
	unsigned int	next_head_page:8;
	unsigned int	tail_page:8;
	unsigned int	seq:12;
	unsigned int	frag:4;
	// DWORD 5
	unsigned int	rts_rate:6;
	unsigned int	dis_rts_fb:1;
	unsigned int	rts_rate_fb_lmt:4;
	unsigned int	cts_2_self:1;
	unsigned int	rts_en:1;
	unsigned int	ra_brsr_id:3;
	unsigned int	tx_ht:1;
	unsigned int	tx_short:1;//for data
	unsigned int	tx_bw:1;
	unsigned int	tx_sc:2;
	unsigned int	stbc:2;
	unsigned int	rd:1;
	unsigned int	rts_ht:1;
	unsigned int	rts_short:1;
	unsigned int	rts_bw:1;
	unsigned int	rts_sc:2;
	unsigned int	rts_stbc:2;
	unsigned int	use_rate:1;
	// DWORD 6
	unsigned int	packet_id:9;
	unsigned int	tx_rate:6;
	unsigned int	dis_fb:1;
	unsigned int	data_ratefb_lmt:5;
	unsigned int	tx_agc:11;
	// DWORD 7
	unsigned int	ip_chksum:16;
	unsigned int	tcp_chksum:16;
	// DWORD 8
	unsigned int	tx_buff_size:16;//pcie
	unsigned int	ip_hdr_offset:8;
	unsigned int	rsvd3:7;
	unsigned int	tcp_en:1;
/*
	// DWORD 9
	unsigned int	tx_buffer_address:32;	//pcie
	// DWORD 10
	unsigned int	next_tx_desc_address:32;	//pcie
*/
}tx_desc_8712;

struct wpsctrl_adapter_ops
{
	int (*init)(void *pdata);
	int (*open)(void *pdata);
	int (*deint)(void *pdata);
	int (*close)(void *pdata);
	int (*iocontrol)(int skfd, unsigned int direction, unsigned int iocode, void *pInBuffer, unsigned int InBufferSize, void *pOutBuffer, unsigned int OutBufferSize);

	//
	int (*set_scan)(void *pdata);
	int (*get_scan_list)(void *pdata);
	int (*get_scan_results)(void *pdata, unsigned int index, void *pbuf);
};

struct wpsctrl
{
	int skfd;
	int skpkt;//use for packet xmit
	char ifrn_name[IFNAMSIZ];	/* if name, e.g. "wlan0" */
	unsigned char enter_mp_test_mode;

	unsigned char chan_index;
//	unsigned char modulation;
	unsigned int power_index;
	unsigned int rate_index;
	unsigned int bandwidth;
	unsigned int antenna;

	unsigned char dig_mode;
	unsigned char cca_leve;
	unsigned char mp_test_mode;

	unsigned int bstarttx;
	unsigned int bstartrx;
	unsigned int tx_pkt_cnts;
	unsigned int rx_pkt_ok_cnts;
	unsigned int rx_pkt_err_cnts;

	unsigned int record_rx_pkt_ok_cnts;
	unsigned int record_rx_pkt_err_cnts;

	unsigned int payload_len;
	unsigned int payload_type;
	unsigned char da[ETH_ALEN];
	unsigned int bGENTSSI;
//	TSSISTRUCT tssi_table;

	EEPROMINF eeprom_info;
	unsigned char efuse_map[EFUSE_MAP_MAX_SIZE];

	unsigned int method;
	unsigned int cnt;
	unsigned int status;
	unsigned int cardinfo;
	unsigned int psk;
	unsigned int legacy_wep;

	unsigned int connect_status;   //0: disconnected; 1:connected
//	unsigned int extraie[MAX_PADDEDIE>>2];
	unsigned char macaddr[ETH_ALEN];
//	unsigned int iocontrolbuf[MAX_IOCONTROLBUFSZ >>2]; //we need the buf to be 4 bytes aligned

	unsigned int numofmasters;

	tx_desc_8712 TxDesc;

	//eason 20100730	pthread_mutex_t iocontrol_mutex;
	spinlock_t		iocontrol_mutex;
	//eason 20100730	struct sockaddr_ll socket_address;

	struct wpsctrl_adapter_ops *pwpsc_ops;
	void *priv_ptr;
	unsigned int priv_sz;
	unsigned char priv_data[1];
};

enum MP_TEST_MODE{
	TEST_NONE,
	PACKET_TX,
	CONTINUOUS_TX,
	SINGLE_CARRIER_TX,
	CARRIER_SUPPRISSION_TX,
	PACKET_RX
};

#define IOCONTROL_SET		0x01
#define IOCONTROL_QUERY 	0x00

#define GET_TX_ANTENNA(ant) ((ant&0xF0)>>4)
#define GET_RX_ANTENNA(ant) (ant&0x0F)
#define SET_TX_ANTENNA(ant, tx) ant=(ant&0x0F)|((tx&0xF)<<4)
#define SET_RX_ANTENNA(ant, rx) ant=(ant&0xF0)|(rx&0xF)

#define MAX_CMD_LEN		20
#define MAX_PARAMETER_NUM	20

typedef struct _CMD_STRUCTURE_ {
	char command[MAX_CMD_LEN];
	int param_cnts;
	void (*func)(struct wpsctrl*, char**);
	const char *description;
}CMD_STRUCTURE;

#define CMDCNTS(cmds) sizeof(cmds)/sizeof(CMD_STRUCTURE)

typedef struct _FUNCTION_MENU_ FUNCTION_MENU, *PFUNCTION_MENU;

struct _FUNCTION_MENU_ {
	PFUNCTION_MENU prev;
	CMD_STRUCTURE *cmds;
	int cnts;
	char prompt[50];
};

typedef struct _INPUT_CMD_ {
	char cmd[MAX_CMD_LEN];
	int  param_cnts;
	char* param[MAX_PARAMETER_NUM];
}INPUT_CMD;


void cmd_help_hdl(struct wpsctrl *pwpsc, char **param);
void exit_hdl(struct wpsctrl *pwpsc, char **param);

/* EMI */
void emi_hdl(struct wpsctrl *pwpsc, char **param);
void mpstart_hdl(struct wpsctrl *pwpsc, char **param);

void set_channel_hdl(struct wpsctrl *pwpsc, char **param);
void set_output_power_hdl(struct wpsctrl *pwpsc, char **param);
void set_data_rate_hdl(struct wpsctrl *pwpsc, char **param);
void set_bandwidth_hdl(struct wpsctrl *pwpsc, char **param);

void set_continuous_tx_hdl(struct wpsctrl *pwpsc,  char **param);
void set_single_carrier_tx_hdl(struct wpsctrl *pwpsc,  char **param);
void set_carrier_suppression_tx_hdl(struct wpsctrl *pwpsc, char **param);
void set_pkt_tx_hdl(struct wpsctrl *pwpsc, unsigned int param);
void get_pkt_phy_rx_hdl(struct wpsctrl *pwpsc, unsigned int param);

void set_dynamic_initial_gain_state_hdl(struct wpsctrl *pwpsc, char **param);
void set_cca_level_hdl(struct wpsctrl *pwpsc, char **param);
void set_power_save_mode_hdl(struct wpsctrl *pwpsc, char **param);

void set_power_tracking_mode_hdl(struct wpsctrl *pwpsc, char **param);
void get_target_tssi_hdl(struct wpsctrl *pwpsc, char **param);
void auto_gen_tssi_hdl(struct wpsctrl *pwpsc, char **param);
void read_tssi_hdl(struct wpsctrl *pwpsc, char **param);
void write_tssi_hdl(struct wpsctrl *pwpsc, char **param);

void read_txpower_hdl(struct wpsctrl *pwpsc, char **param);
void write_txpower_hdl(struct wpsctrl *pwpsc, char **param);


/* E-Fuse */
void read_eeprom_byte_hdl(struct wpsctrl *pwpsc, char **param);
void write_eeprom_byte_hdl(struct wpsctrl *pwpsc, char **param);

void efuse_hdl(struct wpsctrl *pwpsc, char **param);
void read_efuse_map_hdl(struct wpsctrl *pwpsc, char **param);
void write_efuse_map_hdl(struct wpsctrl *pwpsc, char **param);
void show_efuse_map_hdl(struct wpsctrl *pwpsc, char **param);
void read_efuse_hdl(struct wpsctrl *pwpsc, char **param);
void write_efuse_hdl(struct wpsctrl *pwpsc, char **param);

void read_efuse_raw_hdl(struct wpsctrl *pwpsc, char **param);
void write_efuse_raw_hdl(struct wpsctrl *pwpsc, char **param);

void change_mac_address_hdl(struct wpsctrl *pwpsc, char **param);

void read_thermal_meter_hdl(struct wpsctrl *pwpsc, char **param);

/* Register Read/Write */
void regio_hdl(struct wpsctrl *pwpsc, char **param );
void read_reg_hdl(struct wpsctrl *pwpsc, char **param );
void write_reg_hdl(struct wpsctrl *pwpsc, char **param );
void read_rfreg_hdl(struct wpsctrl *pwpsc, char **param );
void write_rfreg_hdl(struct wpsctrl *pwpsc, char **param );
void read_bbreg_hdl(struct wpsctrl *pwpsc, char **param );
void write_bbreg_hdl(struct wpsctrl *pwpsc, char **param );


/* Others */
void power_down_hdl(struct wpsctrl *pwpsc, char **param);

#ifdef CONFIG_MP871X_DBG
#include "r871x_tools_dbg.h"
#endif
#endif


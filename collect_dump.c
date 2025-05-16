/*
 * airdump - a lightweight yet powerful tool for monitoring WiFi networks
 *
 * Copyright (c) 2025 Antonio Mancuso <profmancusoa@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 * This tools is eavily inspired by wavemon (https://github.com/uoaerg/wavemon) and reuse part of its code
 *
 */

#include <unistd.h>
#include <time.h>
#include "iw_if.h"
#include "iw_nl80211.h"
#include "helper.h"

#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif


/* GLOBALS */
static struct iw_nl80211_linkstat *ls_cur = NULL;

static struct __iw_collect__data {
	uint8_t     sig_max_quality;               //sig_max_qual
	float       sig_quality;                   //sig_qual
	float       sig_quality_percentage;        //sig_percent
	float       sig_sig;                       //sig_power_dbm -- dBm
	float       sig_power_mw;                  //sig_power_mw -- mw
	char        *sig_power_unit;               //sig_power_unit
	float       sig_noise;                     //sig_noise_dbm -- dBm            
	float       sig_noise_mw;                  //sig_noise_mw
	char        *sig_noise_unit;               //sign_noise_unit
	float       sig_ssnr;                      //sign_ssnr_dbm 
	uint32_t    rx_packet_count;               //rx_packets
	uint64_t    rx_packet_bytes;               //rx_bytes
	uint64_t    rx_packet_drop;                //rx_packets_drop
	float       rx_packet_drop_percentage;     //rx_packets_drop_percentage
	uint32_t    tx_packet_count;               //tx_packets
	uint64_t    tx_packet_bytes;               //tx_bytes
	uint64_t    tx_packet_retries;             //tx_retries
	float       tx_packet_retries_percentage;  //tx_retries_percentage
	uint32_t    tx_packet_failure;             //tx_packet_failure
	bool        if_is_up;                      //if_status
	bool        if_has_carrier;                //if_carrier
	const char  *if_mode;                      //if_mode
	char        *bss_status;                   //wifi_bss_status
	char        *bssid;                        //wifi_bssid -- normally MAC address
	uint32_t    connected_time;                //bssid_active_time -- s
	uint32_t    inactive_time;                 //bssid_inactive_time -- ms
	uint32_t    wifi_freq;                     //channel_freq -- Mhz
	int         wifi_channel;                  //channel_num
	const char  *wifi_channel_width;           //channel_width
	uint8_t     wifi_channel_bands;            //channel_bands
	uint64_t    wifi_beacons;                  //beacons_rx
	uint32_t    wifi_beacons_loss;             //beacons_loss
	int8_t      wifi_beacons_avg_signal;       //beacons_avg_signal -- dBm
	float       wifi_beacons_interval;         //beacons_interval
	uint8_t     wifi_beacons_dtim;             //dtim
	bool        wifi_cts_protection;           //cts_protection
	bool        wifi_wme;                      //wme
	bool        wifi_tdls;                     //tdls
	bool        wifi_mfp;                      //mfp
	bool        wifi_long_preamble;            //long_preamble
	bool        wifi_short_slot_time;          //short_slot_time
	double      wifi_tx_power_dbm;             //tx_power_dbm
	double      wifi_tx_power_mw;              //tx_power_mw
	bool        wifi_power_save;               //power_save
	uint8_t     wifi_retry_short;              //retry_short
	uint32_t    wifi_retry_long;               //retry_long
	uint32_t    wifi_rts_threshold;            //rts_threshold --  4294967295 means -1 off
	uint32_t    wifi_frag_threshold;           //frag_threshold --  4294967295 means -1 off
	char        wifi_mode[16];                 //wifi_mode
	char        wifi_qdisc[16];                //if_qdisc
	uint16_t    wifi_tx_qlen;                  //if_qlen
	char        *wifi_mac;                     //if_mac
	uint16_t    wifi_mtu;                      //if_mtu
	char        wifi_ipv4[64];                 //if_ip
} iw_collect_data;

static void collect_levels(void) {;	
	bool noise_data_valid;
	int sig_qual = -1, sig_level = 0;
	
	noise_data_valid = iw_nl80211_have_survey_data(ls_cur);
	sig_level = ls_cur->signal;

	/* See comments in iw_cache_update */
	if (sig_level == 0)
		sig_level = ls_cur->signal_avg;
	if (sig_level == 0)
		sig_level = ls_cur->bss_signal;

	/* If the signal level is positive, assume it is an absolute value (#100). */
	if (sig_level > 0)
		sig_level *= -1;

	if (ls_cur->bss_signal_qual) {
		/* BSS_SIGNAL_UNSPEC is scaled 0..100 */
		sig_qual = ls_cur->bss_signal_qual;
		iw_collect_data.sig_max_quality = 100;
	} else if (sig_level) {
		if (sig_level < -110)
			sig_qual = 0;
		else if (sig_level > -40)
			sig_qual = 70;
		else
			sig_qual = sig_level + 110;
		iw_collect_data.sig_max_quality = 70;
	}

	iw_collect_data.sig_quality = ewma(iw_collect_data.sig_quality, sig_qual, 0);
	iw_collect_data.sig_quality_percentage = (1e2 * iw_collect_data.sig_quality)/iw_collect_data.sig_max_quality;
	DEBUG_PRINT(("qual(%0.f) - max(%d) - perc(%0.f%%)\n", iw_collect_data.sig_quality, iw_collect_data.sig_max_quality, iw_collect_data.sig_quality_percentage));

	if (sig_level != 0) {
		iw_collect_data.sig_sig = ewma(iw_collect_data.sig_sig, sig_level, 0); 
		iw_collect_data.sig_power_mw = dbm2mw(iw_collect_data.sig_sig);
		iw_collect_data.sig_power_unit = dbm2units(iw_collect_data.sig_sig);
		DEBUG_PRINT(("signal_level(%.0f) -- signal_power(%f) -- signal_power(%s)\n", iw_collect_data.sig_sig, iw_collect_data.sig_power_mw , iw_collect_data.sig_power_unit));
	}

	if (noise_data_valid) {
        iw_collect_data.sig_noise = ewma(iw_collect_data.sig_noise, ls_cur->survey.noise, 0);
		iw_collect_data.sig_noise_mw = dbm2mw(iw_collect_data.sig_noise);
		iw_collect_data.sig_noise_unit = dbm2units(iw_collect_data.sig_noise);
		DEBUG_PRINT(("%.0f dBm (%s) %.0f\n", iw_collect_data.sig_noise, iw_collect_data.sig_noise_unit, iw_collect_data.sig_noise_mw));
		
		if (sig_level) {
            iw_collect_data.sig_ssnr = ewma(iw_collect_data.sig_ssnr, sig_level - ls_cur->survey.noise, 0);
			DEBUG_PRINT(("%.0f dB\n", iw_collect_data.sig_ssnr));
		}
	} 
}

static void collect_packet_count(void) {
	if (ls_cur->rx_packets) {
		iw_collect_data.rx_packet_count = ls_cur->rx_packets;
		iw_collect_data.rx_packet_bytes = ls_cur->rx_bytes;
		DEBUG_PRINT(("RX: packets(%" PRIu32 ") bytes(%" PRIu64 ")\n", iw_collect_data.rx_packet_count, iw_collect_data.rx_packet_bytes));
	} 

	if (ls_cur->rx_drop_misc) {
		iw_collect_data.rx_packet_drop = ls_cur->rx_drop_misc;
		iw_collect_data.rx_packet_drop_percentage = (1e2 * iw_collect_data.rx_packet_drop)/iw_collect_data.rx_packet_count;

		DEBUG_PRINT(("packet drop:%llu - percentage:%.3f\n", (unsigned long long)iw_collect_data.rx_packet_drop, iw_collect_data.rx_packet_drop_percentage));
	}

	if (ls_cur->tx_packets) {
		iw_collect_data.tx_packet_count = ls_cur->tx_packets;
		iw_collect_data.tx_packet_bytes = ls_cur->tx_bytes;
		DEBUG_PRINT(("TX: packets(%" PRIu32 ") bytes(%" PRIu64 ")\n", iw_collect_data.tx_packet_count, iw_collect_data.tx_packet_bytes));
	} 

	if (ls_cur->tx_retries) {
		iw_collect_data.tx_packet_retries = ls_cur->tx_retries;
		iw_collect_data.tx_packet_retries_percentage = (1e2 * iw_collect_data.tx_packet_retries)/iw_collect_data.tx_packet_count;
		DEBUG_PRINT(("tx reties count: %lu  -- percent:%.2f\n", iw_collect_data.tx_packet_retries,iw_collect_data.tx_packet_retries_percentage));
	}

	if (ls_cur->tx_failed) {
		iw_collect_data.tx_packet_failure = ls_cur->tx_failed;
		DEBUG_PRINT(("tx failure counts: %u \n",iw_collect_data.tx_packet_failure));
	}
}


/** General information section */
static void collect_iface_info() {
	struct iw_nl80211_ifstat ifs;

	iw_nl80211_getifstat(&ifs);
	iw_nl80211_get_power_save(&ifs);
	iw_nl80211_get_phy(&ifs);

	iw_collect_data.if_mode = iftype_name(ifs.iftype); 
	DEBUG_PRINT(("mode:%s\n", iw_collect_data.if_mode));

	if (!ether_addr_is_zero(&ls_cur->bssid)) {
		switch (ls_cur->status) {
		case NL80211_BSS_STATUS_ASSOCIATED:
			iw_collect_data.bss_status = "ASSOCIATED";
			break;
		case NL80211_BSS_STATUS_AUTHENTICATED:
			iw_collect_data.bss_status = "AUTHENTICATED";
			break;
		case NL80211_BSS_STATUS_IBSS_JOINED:
			iw_collect_data.bss_status = "JOINED";
			break;
		default:
			iw_collect_data.bss_status = "STATION";
		}

		iw_collect_data.bssid = ether_lookup(&ls_cur->bssid);
		DEBUG_PRINT(("%s %s\n", iw_collect_data.bss_status, iw_collect_data.bssid));

		if (ls_cur->status == NL80211_BSS_STATUS_ASSOCIATED) {
			iw_collect_data.connected_time = ls_cur->connected_time;
			iw_collect_data.inactive_time = ls_cur->inactive_time;
			DEBUG_PRINT(("time: %u seconds\n", ls_cur->connected_time));
			DEBUG_PRINT(("inactive: %dms\n", ls_cur->inactive_time));
		}
	}

	/* Frequency / channel */
	iw_collect_data.wifi_freq = ifs.freq;  
	DEBUG_PRINT(("freq: %u\n", iw_collect_data.wifi_freq));
	iw_collect_data.wifi_channel = ieee80211_frequency_to_channel(iw_collect_data.wifi_freq); 
	iw_collect_data.wifi_channel_width = channel_width_name(ifs.chan_width);
	DEBUG_PRINT(("channel width: %s\n", iw_collect_data.wifi_channel_width));
	iw_collect_data.wifi_channel_bands = ifs.phy.bands;
	DEBUG_PRINT(("bands: %d\n", iw_collect_data.wifi_channel_bands));
	
	/* Beacons */
	iw_collect_data.wifi_beacons = ls_cur->beacons;
	iw_collect_data.wifi_beacons_loss = ls_cur->beacon_loss;
	iw_collect_data.wifi_beacons_avg_signal = (int8_t)ls_cur->beacon_avg_sig;
	iw_collect_data.wifi_beacons_interval = (ls_cur->beacon_int * 1024.0)/1e6;
	iw_collect_data.wifi_beacons_dtim = ls_cur->dtim_period;
	DEBUG_PRINT(("beacons: %llu\n", (unsigned long long)iw_collect_data.wifi_beacons));
	DEBUG_PRINT(("lost beacons: %d\n", iw_collect_data.wifi_beacons_loss));
	DEBUG_PRINT(("avg beacon signal: %d dBm\n", iw_collect_data.wifi_beacons_avg_signal));
	DEBUG_PRINT(("beacon interval: %.1fs\n", iw_collect_data.wifi_beacons_interval));
	DEBUG_PRINT(("DTIM: %u\n", iw_collect_data.wifi_beacons_dtim));

	iw_collect_data.wifi_cts_protection = ls_cur->cts_protection;
	iw_collect_data.wifi_wme = ls_cur->wme;
	iw_collect_data.wifi_tdls = ls_cur->tdls;
	iw_collect_data.wifi_mfp = ls_cur->mfp;
	iw_collect_data.wifi_long_preamble = ls_cur->long_preamble;
	iw_collect_data.wifi_short_slot_time = ls_cur->short_slot_time;
	DEBUG_PRINT(("long_preamble:%d\n", iw_collect_data.wifi_long_preamble));
	DEBUG_PRINT(("short_slot_time:%d\n", iw_collect_data.wifi_short_slot_time));
	
	/* TX Power */
	iw_collect_data.wifi_tx_power_dbm = ifs.tx_power;
	iw_collect_data.wifi_tx_power_mw = dbm2mw(ifs.tx_power);
	iw_collect_data.wifi_power_save = ifs.power_save; 
	DEBUG_PRINT(("tx power: %g dBm (%.2f mW)\n", iw_collect_data.wifi_tx_power_dbm, iw_collect_data.wifi_tx_power_mw));	
	DEBUG_PRINT(("power saving: %d\n", iw_collect_data.wifi_power_save));

	/* Retry handling */
	iw_collect_data.wifi_retry_short = ifs.phy.retry_short;
	iw_collect_data.wifi_retry_long = ifs.phy.retry_long;
	DEBUG_PRINT(("retry short/long: %d/%d\n", iw_collect_data.wifi_retry_short, iw_collect_data.wifi_retry_long));
	iw_collect_data.wifi_rts_threshold = ifs.phy.rts_threshold;
	DEBUG_PRINT(("rts/cts: %u\n", iw_collect_data.wifi_rts_threshold));
	iw_collect_data.wifi_frag_threshold = ifs.phy.frag_threshold; 
	DEBUG_PRINT(("frag: %u\n", iw_collect_data.wifi_frag_threshold));
}

/** Network information pertaining to interface with interface index @ifindex. */
static void collect_netinfo(struct if_info *info) {
	struct if_info *active = info->master ? info->master : info;
	
	if (ifinfo_is_up(info)) {
		strncpy(iw_collect_data.wifi_mode, info->mode, 16); 
		DEBUG_PRINT(("mode: %s\n",iw_collect_data.wifi_mode));
		strncpy(iw_collect_data.wifi_qdisc, info->qdisc, 16);
		DEBUG_PRINT(("qdisc: %s\n", iw_collect_data.wifi_qdisc));
		iw_collect_data.wifi_tx_qlen = info->txqlen;
		DEBUG_PRINT(("qlen: %u\n", iw_collect_data.wifi_tx_qlen));
	}
	iw_collect_data.wifi_mac = !ether_addr_is_zero(&active->hwaddr) ? ether_lookup(&active->hwaddr) : "none";
	DEBUG_PRINT(("MAC: %s\n", iw_collect_data.wifi_mac));
	iw_collect_data.wifi_mtu = ifinfo_is_up(info) ? info->mtu : 0;  
	DEBUG_PRINT(("mtu: %u\n", iw_collect_data.wifi_mtu));
	strncpy(iw_collect_data.wifi_ipv4, *active->v4.addr ? active->v4.addr : "none", 64);
	DEBUG_PRINT(("IP: %s\n", iw_collect_data.wifi_ipv4));
	
	free(info->master);
}

/** Wireless interface information */
static void collect_iface_status() {
	struct if_info net_info;
	if_getinf(conf_ifname(), &net_info);

	iw_collect_data.if_is_up = net_info.flags & IFF_UP;
	iw_collect_data.if_has_carrier = net_info.carrier;
	DEBUG_PRINT(("IFACCE:%s -- UP: %d CARRIER: %d\n", conf_ifname(), iw_collect_data.if_is_up, iw_collect_data.if_has_carrier));

	collect_netinfo(&net_info);
}

void collect_dump_init() {
	ls_cur = calloc(1, sizeof(*ls_cur));
}

FILE *open_dump_file(const char* dump_file) {
	FILE *fpt = fopen(dump_file, "w+");
	if(fpt == NULL) 
		err_quit("ERROR: cannot open dump file %s\n", dump_file);
	
	//disable buggering
	setvbuf(fpt, NULL, _IONBF, 0);

	return fpt;
}

void dump_file_header(FILE *fpt) {
	fprintf(fpt, "if_name,if_status,if_carrier,if_mode,if_qdisc,if_qlen,if_mtu,if_mac,if_ip,"
				 "wifi_mode,wifi_bssid,wifi_bss_status,bssid_active_time,bssid_inactive_time,channel_freq,channel_num,channel_width,channel_bands,beacons_rx,beacons_loss,beacons_avg_signal,beacons_interval,dtim,cts_protection,wme,tdls,mfp,long_preamble,short_slot_time,"
				 "sig_qual,sig_max_qual,sig_percent,sig_power_dbm,sig_power_mw,sig_power_unit,sig_noise_dbm,sig_noise_mw,sign_noise_unit,sign_ssnr_dbm"
				 "tx_power_dbm,tx_power_mw,power_save,retry_short,retry_long,rts_threshold,frag_threshold"
				 "rx_packets,rx_bytes,rx_packets_drop,rx_packets_drop_percentage"
				 "tx_packets,tx_bytes,tx_retries,tx_retries_percentage,tx_packet_failure"
				 "\n"
			);
}

void dump_collect_data(FILE *fpt, struct timespec *time, uint32_t collect_count) {
	fprintf(fpt,
		"%u,%lu,"
		"%s,%d,%d,%s,%s,%d,%d,%s,%s,"
		"%s,%s,%s,%u,%u,%u,%d,%s,%d,%lu,%u,%d,%f,%d,%d,%d,%d,%d,%d,%d,"
		"%f,%d,%f,%f,%f,%s,%f,%f,%s,%f"
		"%f,%f,%d,%d,%u,%u,%u"
		"%u,%lu,%lu,%f"
		"%u,%lu,%lu,%f,%u" 
		"\n", 
		collect_count++, (time->tv_sec * 1000) + (time->tv_nsec / 1000000),
		conf_ifname(),
		iw_collect_data.if_is_up,
		iw_collect_data.if_has_carrier,
		iw_collect_data.if_mode,
		iw_collect_data.wifi_qdisc,
		iw_collect_data.wifi_tx_qlen,
		iw_collect_data.wifi_mtu,
		iw_collect_data.wifi_mac,
		iw_collect_data.wifi_ipv4,
		iw_collect_data.wifi_mode,
		iw_collect_data.bssid,
		iw_collect_data.bss_status,
		iw_collect_data.connected_time,
		iw_collect_data.inactive_time,
		iw_collect_data.wifi_freq,
		iw_collect_data.wifi_channel,
		iw_collect_data.wifi_channel_width,
		iw_collect_data.wifi_channel_bands,
		iw_collect_data.wifi_beacons,
		iw_collect_data.wifi_beacons_loss,
		iw_collect_data.wifi_beacons_avg_signal,
		iw_collect_data.wifi_beacons_interval,
		iw_collect_data.wifi_beacons_dtim,
		iw_collect_data.wifi_cts_protection,
		iw_collect_data.wifi_wme,
		iw_collect_data.wifi_tdls,
		iw_collect_data.wifi_mfp,
		iw_collect_data.wifi_long_preamble,
		iw_collect_data.wifi_short_slot_time,
		iw_collect_data.sig_quality,
		iw_collect_data.sig_max_quality,
		iw_collect_data.sig_quality_percentage,
		iw_collect_data.sig_sig,
		iw_collect_data.sig_power_mw,
		iw_collect_data.sig_power_unit,
		iw_collect_data.sig_noise,
		iw_collect_data.sig_noise_mw,
		iw_collect_data.sig_noise_unit,
		iw_collect_data.sig_ssnr,
		iw_collect_data.wifi_tx_power_dbm,
		iw_collect_data.wifi_tx_power_mw,
		iw_collect_data.wifi_power_save,
		iw_collect_data.wifi_retry_short,
		iw_collect_data.wifi_retry_long,
		iw_collect_data.wifi_rts_threshold,
		iw_collect_data.wifi_frag_threshold,
		iw_collect_data.rx_packet_count,
		iw_collect_data.rx_packet_bytes,
		iw_collect_data.rx_packet_drop,
		iw_collect_data.rx_packet_drop_percentage,
		iw_collect_data.tx_packet_count,
		iw_collect_data.tx_packet_bytes,
		iw_collect_data.tx_packet_retries,
		iw_collect_data.tx_packet_retries_percentage,
		iw_collect_data.tx_packet_failure
	);
}

void collect_dump_loop(const char *dump_file, uint32_t period, uint32_t count) {
	uint32_t collect_count = 0;
	struct timespec start;
	FILE *fpt = open_dump_file(dump_file);
	dump_file_header(fpt);
	do {
			clock_gettime(CLOCK_REALTIME, &start);
			memset(&iw_collect_data, 0, sizeof(iw_collect_data));
			collect_iface_status();
			collect_iface_info();
			iw_nl80211_get_linkstat(ls_cur);
			collect_levels();
			collect_packet_count();
			
			dump_collect_data(fpt, &start, ++collect_count);
	} while (--count > 0 && usleep((period > 10 ? period - 2 : period) * 1000) == 0);
	fclose(fpt);
}


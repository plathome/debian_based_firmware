/******************************************************************************
 *
 * Copyright(c) 2009-2014  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

#include "../wifi.h"
#include "../pci.h"
#include "../base.h"
#include "../stats.h"
#include "reg.h"
#include "def.h"
#include "phy.h"
#include "trx.h"
#include "led.h"
#include "fw.h"

static u8 _rtl8723de_map_hwqueue_to_fwqueue(struct sk_buff *skb, u8 hw_queue)
{
	__le16 fc = rtl_get_fc(skb);

	if (unlikely(ieee80211_is_beacon(fc)))
		return QSLT_BEACON;
	if (ieee80211_is_mgmt(fc) || ieee80211_is_ctl(fc))
		return QSLT_MGNT;

	return skb->priority;
}

static void _rtl8723de_query_rxphystatus(struct ieee80211_hw *hw, u8 *phystrpt,
					 struct ieee80211_hdr *hdr,
					 struct rtl_stats *pstatus)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);

	rtlpriv->phydm.ops->phydm_query_phy_status(rtlpriv, phystrpt, hdr,
						   pstatus);

	/* UI BSS List signal strength(in percentage),
	 * make it good looking, from 0~100.
	 */
	pstatus->signalstrength =
		(u8)(rtl_signal_scale_mapping(hw, pstatus->rx_pwdb_all));
}

static void _rtl8723de_translate_rx_signal_stuff(struct ieee80211_hw *hw,
					struct sk_buff *skb,
					struct rtl_stats *status,
					u8 *pdesc,
					u8 *p_drvinfo)
{
	struct ieee80211_hdr *hdr;
	u8 *tmp_buf;

	tmp_buf = skb->data + status->rx_drvinfo_size + status->rx_bufshift +
		  RX_DESC_SIZE;

	hdr = (struct ieee80211_hdr *)tmp_buf;

	_rtl8723de_query_rxphystatus(hw, p_drvinfo, hdr, status);

	rtl_process_phyinfo(hw, tmp_buf, status);
}

static void _rtl8723de_insert_emcontent(struct rtl_tcb_desc *ptcb_desc,
					u8 *virtualaddress)
{
	u32 dwtmp = 0;
	memset(virtualaddress, 0, 8);

	SET_EARLYMODE_PKTNUM(virtualaddress, ptcb_desc->empkt_num);
	if (ptcb_desc->empkt_num == 1) {
		dwtmp = ptcb_desc->empkt_len[0];
	} else {
		dwtmp = ptcb_desc->empkt_len[0];
		dwtmp += ((dwtmp % 4) ? (4 - dwtmp % 4) : 0) + 4;
		dwtmp += ptcb_desc->empkt_len[1];
	}
	SET_EARLYMODE_LEN0(virtualaddress, dwtmp);

	if (ptcb_desc->empkt_num <= 3) {
		dwtmp = ptcb_desc->empkt_len[2];
	} else {
		dwtmp = ptcb_desc->empkt_len[2];
		dwtmp += ((dwtmp % 4) ? (4 - dwtmp % 4) : 0) + 4;
		dwtmp += ptcb_desc->empkt_len[3];
	}
	SET_EARLYMODE_LEN1(virtualaddress, dwtmp);
	if (ptcb_desc->empkt_num <= 5) {
		dwtmp = ptcb_desc->empkt_len[4];
	} else {
		dwtmp = ptcb_desc->empkt_len[4];
		dwtmp += ((dwtmp % 4) ? (4 - dwtmp % 4) : 0) + 4;
		dwtmp += ptcb_desc->empkt_len[5];
	}
	SET_EARLYMODE_LEN2_1(virtualaddress, dwtmp & 0xF);
	SET_EARLYMODE_LEN2_2(virtualaddress, dwtmp >> 4);
	if (ptcb_desc->empkt_num <= 7) {
		dwtmp = ptcb_desc->empkt_len[6];
	} else {
		dwtmp = ptcb_desc->empkt_len[6];
		dwtmp += ((dwtmp % 4) ? (4 - dwtmp % 4) : 0) + 4;
		dwtmp += ptcb_desc->empkt_len[7];
	}
	SET_EARLYMODE_LEN3(virtualaddress, dwtmp);
	if (ptcb_desc->empkt_num <= 9) {
		dwtmp = ptcb_desc->empkt_len[8];
	} else {
		dwtmp = ptcb_desc->empkt_len[8];
		dwtmp += ((dwtmp % 4) ? (4 - dwtmp % 4) : 0) + 4;
		dwtmp += ptcb_desc->empkt_len[9];
	}
	SET_EARLYMODE_LEN4(virtualaddress, dwtmp);
}

bool rtl8723de_rx_query_desc(struct ieee80211_hw *hw,
			     struct rtl_stats *status,
			     struct ieee80211_rx_status *rx_status,
			     u8 *pdesc, struct sk_buff *skb)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	u8 *p_drvinfo;
	struct ieee80211_hdr *hdr;
	u8 wake_match;
	u32 phystatus = GET_RX_DESC_PHYST_8723D(pdesc);

	status->length = (u16)GET_RX_DESC_PKT_LEN_8723D(pdesc);
	status->rx_drvinfo_size = (u8)GET_RX_DESC_DRV_INFO_SIZE_8723D(pdesc) *
				  RX_DRV_INFO_SIZE_UNIT;
	status->rx_bufshift = (u8)(GET_RX_DESC_SHIFT_8723D(pdesc) & 0x03);
	status->icv = (u16) GET_RX_DESC_ICV_8723D(pdesc);
	status->crc = (u16) GET_RX_DESC_CRC32_8723D(pdesc);
	status->hwerror = (status->crc | status->icv);
	status->decrypted = !GET_RX_DESC_SWDEC_8723D(pdesc);
	status->rate = (u8)GET_RX_DESC_RX_RATE_8723D(pdesc);
	status->shortpreamble = 0;
	status->isampdu = (bool)(GET_RX_DESC_PAGGR_8723D(pdesc) == 1);
	status->isfirst_ampdu = (bool)(GET_RX_DESC_PAGGR_8723D(pdesc) == 1);
	status->timestamp_low = GET_RX_DESC_TSFL_8723D(pdesc);
	status->rx_is40Mhzpacket = 0;
	status->bandwidth = 0;
	status->macid = GET_RX_DESC_MACID_8723D(pdesc);
	status->is_ht = (bool)GET_RX_DESC_RXHT_8723D(pdesc);

	status->is_cck = RX_HAL_IS_CCK_RATE(status->rate);

	if (GET_RX_DESC_RPT_SEL_8723D(pdesc))
		status->packet_report_type = C2H_PACKET;
	else
		status->packet_report_type = NORMAL_RX;

	if (GET_RX_DESC_PATTERN_MATCH_8723D(pdesc))
		wake_match = BIT(2);
	else if (GET_RX_DESC_MAGIC_MATCH_8723D(pdesc))
		wake_match = BIT(1);
	else if (GET_RX_DESC_UNICAST_MATCH_8723D(pdesc))
		wake_match = BIT(0);
	else
		wake_match = 0;
	if (wake_match)
		RT_TRACE(rtlpriv, COMP_RXDESC, DBG_LOUD,
		"GGGGGGGGGGGGGet Wakeup Packet!! WakeMatch=%d\n",
		wake_match);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
	rx_status->freq = hw->conf.chandef.chan->center_freq;
	rx_status->band = hw->conf.chandef.chan->band;
#else
	rx_status->freq = hw->conf.channel->center_freq;
	rx_status->band = hw->conf.channel->band;
#endif

	hdr = (struct ieee80211_hdr *)(skb->data + status->rx_drvinfo_size +
				       status->rx_bufshift + RX_DESC_SIZE);

	if (status->crc)
		rx_status->flag |= RX_FLAG_FAILED_FCS_CRC;

	if (status->rx_is40Mhzpacket)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0))
		rx_status->bw = RATE_INFO_BW_40;
#else
		rx_status->flag |= RX_FLAG_40MHZ;
#endif

	if (status->is_ht)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0))
		rx_status->encoding = RX_ENC_HT;
#else
		rx_status->flag |= RX_FLAG_HT;
#endif

	rx_status->flag |= RX_FLAG_MACTIME_START;

	/* hw will set status->decrypted true, if it finds the
	 * frame is open data frame or mgmt frame.
	 * So hw will not decryption robust managment frame
	 * for IEEE80211w but still set status->decrypted
	 * true, so here we should set it back to undecrypted
	 * for IEEE80211w frame, and mac80211 sw will help
	 * to decrypt it
	 */

	if (status->decrypted) {
		if ((!_ieee80211_is_robust_mgmt_frame(hdr)) &&
		    (ieee80211_has_protected(hdr->frame_control)))
			rx_status->flag |= RX_FLAG_DECRYPTED;
		else
			rx_status->flag &= ~RX_FLAG_DECRYPTED;
	}

	/* rate_idx: index of data rate into band's
	 * supported rates or MCS index if HT rates
	 * are use (RX_FLAG_HT)
	 */
	rx_status->rate_idx = rtlwifi_rate_mapping(hw, status->is_ht,
						   false, status->rate);

	rx_status->mactime = status->timestamp_low;
	if (phystatus)
		p_drvinfo = skb->data + status->rx_bufshift + RX_DESC_SIZE;
	else
		p_drvinfo = NULL;

	_rtl8723de_translate_rx_signal_stuff(hw, skb, status, pdesc, p_drvinfo);

	if (phystatus)
		rx_status->signal = status->recvsignalpower + 10;
	else
		rx_status->flag |= RX_FLAG_NO_SIGNAL_VAL;

#if 0
	if (status->packet_report_type == TX_REPORT2) {
		status->macid_valid_entry[0] =
		  GET_RX_RPT2_DESC_MACID_VALID_1(pdesc);
		status->macid_valid_entry[1] =
		  GET_RX_RPT2_DESC_MACID_VALID_2(pdesc);
	}
#endif
	return true;
}


void rtl8723de_rx_check_dma_ok(struct ieee80211_hw *hw, u8 *header_desc,
			       u8 queue_index)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	u8 first_seg;
	u8 last_seg;
	u16 total_len;
	u16 read_cnt = 0;

	if (!header_desc)
		return;

	do {
		total_len = (u16)GET_RX_BUFFER_DESC_TOTAL_LENGTH(header_desc);
		first_seg = (u8)GET_RX_BUFFER_DESC_FS(header_desc);
		last_seg = (u8)GET_RX_BUFFER_DESC_LS(header_desc);

		if (read_cnt++ > 20) {
			RT_TRACE(rtlpriv, COMP_RECV, DBG_DMESG,
				 "RX chk DMA over %d times\n", read_cnt);
			break;
		}

	} while (total_len == 0 && first_seg == 0 && last_seg == 0);
}

u16 rtl8723de_rx_desc_buff_remained_cnt(struct ieee80211_hw *hw, u8 queue_index)
{
	struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	u16 desc_idx_hw = 0, desc_idx_host = 0, remind_cnt = 0;
	u32 tmp_4byte = 0;
	u32 rw_mask = 0x1ff;

	tmp_4byte = rtl_read_dword(rtlpriv, REG_RXQ_RXBD_IDX_8723D);
	desc_idx_hw = (u16)((tmp_4byte >> 16) & rw_mask);
	desc_idx_host = (u16)(tmp_4byte & rw_mask);

	/* may be no data, donot rx */
	if (desc_idx_hw == desc_idx_host)
		return 0;

	remind_cnt =
		(desc_idx_hw > desc_idx_host) ?
			(desc_idx_hw - desc_idx_host) :
			(RX_DESC_NUM_8723DE - (desc_idx_host - desc_idx_hw));

	rtlpci->rx_ring[queue_index].next_rx_rp = desc_idx_host;

	return remind_cnt;
}

static u16 get_desc_address_from_queue_index(u16 queue_index)
{
	/*
	 * Note: Access these registers will take a lot of cost.
	 */
	u16 desc_address = REG_BEQ_TXBD_IDX_8723D;

	switch (queue_index) {
	case BK_QUEUE:
		desc_address = REG_BKQ_TXBD_IDX_8723D;
		break;
	case BE_QUEUE:
		desc_address = REG_BEQ_TXBD_IDX_8723D;
		break;
	case VI_QUEUE:
		desc_address = REG_VIQ_TXBD_IDX_8723D;
		break;
	case VO_QUEUE:
		desc_address = REG_VOQ_TXBD_IDX_8723D;
		break;
	case BEACON_QUEUE:
		desc_address = REG_BEQ_TXBD_IDX_8723D;
		break;
	case MGNT_QUEUE:
		desc_address = REG_MGQ_TXBD_IDX_8723D;
		break;
	case HIGH_QUEUE:
		desc_address = REG_HI0Q_TXBD_IDX_8723D;
		break;
	case HCCA_QUEUE:
		desc_address = REG_BEQ_TXBD_IDX_8723D;
		break;
	default:
		break;
	}
	return desc_address;
}

/*free  desc that can be used */
u16 rtl8723de_get_available_desc(struct ieee80211_hw *hw, u8 q_idx)
{
	struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
	struct rtl8192_tx_ring *ring = &rtlpci->tx_ring[q_idx];

	return calc_fifo_space(ring->cur_tx_rp, ring->cur_tx_wp,
			       TX_DESC_NUM_8723D);
}

void rtl8723de_pre_fill_tx_bd_desc(struct ieee80211_hw *hw, u8 *tx_bd_desc,
				   u8 *desc, u8 queue_index,
				   struct sk_buff *skb, dma_addr_t data_addr)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
	u32 pkt_len = skb->len;
	u16 desc_size = TX_DESC_SIZE;
	u32 psblen = 0;
	u32 total_packet_size = 0;
	u16 current_bd_desc;
	u8 i = 0;
	/*u16 real_desc_size = 0x28;*/
	u16 append_early_mode_size = 0;
	u8 segmentnum = 1 << (RTL8822BE_SEG_NUM + 1);
	dma_addr_t desc_dma_addr;
	bool dma64 = rtlpriv->cfg->mod_params->dma64;

	current_bd_desc = rtlpci->tx_ring[queue_index].cur_tx_wp;

	total_packet_size = desc_size + pkt_len;

	if (rtlpriv->rtlhal.earlymode_enable) {
		if (queue_index < BEACON_QUEUE) {
			append_early_mode_size = 8;
			total_packet_size += append_early_mode_size;
		}
	}

	/* page number (round up) */
	psblen = (total_packet_size - 1) / 128 + 1;

	/* tx desc addr */
	desc_dma_addr = rtlpci->tx_ring[queue_index].dma +
			(current_bd_desc * TX_DESC_ALLOC_SIZE);

	/* Reset */
	SET_TX_BUFF_DESC_LEN_0(tx_bd_desc, 0);
	SET_TX_BUFF_DESC_PSB(tx_bd_desc, 0);
	SET_TX_BUFF_DESC_OWN(tx_bd_desc, 0);

	for (i = 1; i < segmentnum; i++) {
		SET_TXBUFFER_DESC_LEN_WITH_OFFSET(tx_bd_desc, i, 0);
		SET_TXBUFFER_DESC_AMSDU_WITH_OFFSET(tx_bd_desc, i, 0);
		SET_TXBUFFER_DESC_ADD_LOW_WITH_OFFSET(tx_bd_desc, i, 0);
		SET_TXBUFFER_DESC_ADD_HIGH_WITH_OFFSET(tx_bd_desc, i, 0, dma64);
	}

	/* Clear all status */
	CLEAR_PCI_TX_DESC_CONTENT(desc, TX_DESC_SIZE);

	if (rtlpriv->rtlhal.earlymode_enable) {
		if (queue_index < BEACON_QUEUE)
			SET_TX_BUFF_DESC_LEN_0(tx_bd_desc, desc_size + 8);
		else
			SET_TX_BUFF_DESC_LEN_0(tx_bd_desc, desc_size);
	} else {
		SET_TX_BUFF_DESC_LEN_0(tx_bd_desc, desc_size);
	}
	SET_TX_BUFF_DESC_PSB(tx_bd_desc, psblen);
	SET_TX_BUFF_DESC_ADDR_LOW_0(tx_bd_desc, desc_dma_addr);
	SET_TX_BUFF_DESC_ADDR_HIGH_0(tx_bd_desc, ((u64)desc_dma_addr >> 32),
				     dma64);

	SET_TXBUFFER_DESC_LEN_WITH_OFFSET(tx_bd_desc, 1, pkt_len);
	SET_TXBUFFER_DESC_AMSDU_WITH_OFFSET(tx_bd_desc, 1, 0);
	SET_TXBUFFER_DESC_ADD_LOW_WITH_OFFSET(tx_bd_desc, 1, data_addr);
	SET_TXBUFFER_DESC_ADD_HIGH_WITH_OFFSET(tx_bd_desc, 1,
					       ((u64)data_addr >> 32), dma64);

	SET_TX_DESC_PKT_SIZE_8723D(desc, (u16)(pkt_len));
}

void rtl8723de_tx_fill_desc(struct ieee80211_hw *hw,
			    struct ieee80211_hdr *hdr, u8 *pdesc_tx,
			    u8 *pbd_desc_tx, struct ieee80211_tx_info *info,
			    struct ieee80211_sta *sta, struct sk_buff *skb,
			    u8 hw_queue, struct rtl_tcb_desc *ptcb_desc)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_mac *mac = rtl_mac(rtl_priv(hw));
	struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
	struct rtl_hal *rtlhal = rtl_hal(rtlpriv);
	u8 *pdesc = (u8 *)pdesc_tx;
	u16 seq_number;
	__le16 fc = hdr->frame_control;
	unsigned int buf_len = 0;
	unsigned int skb_len = skb->len;
	u8 fw_qsel = _rtl8723de_map_hwqueue_to_fwqueue(skb, hw_queue);
	bool firstseg = ((hdr->seq_ctrl &
			    cpu_to_le16(IEEE80211_SCTL_FRAG)) == 0);
	bool lastseg = ((hdr->frame_control &
			   cpu_to_le16(IEEE80211_FCTL_MOREFRAGS)) == 0);
	dma_addr_t mapping;
	u8 bw_40 = 0;
	u8 short_gi = 0;

	if (mac->opmode == NL80211_IFTYPE_STATION) {
		bw_40 = mac->bw_40;
	} else if (mac->opmode == NL80211_IFTYPE_AP ||
		mac->opmode == NL80211_IFTYPE_ADHOC) {
		if (sta)
			bw_40 = sta->ht_cap.cap &
				IEEE80211_HT_CAP_SUP_WIDTH_20_40;
	}
	seq_number = (le16_to_cpu(hdr->seq_ctrl) & IEEE80211_SCTL_SEQ) >> 4;
	rtl_get_tcb_desc(hw, info, sta, skb, ptcb_desc);
	/* reserve 8 byte for AMPDU early mode */
	if (rtlhal->earlymode_enable) {
		skb_push(skb, EM_HDR_LEN);
		memset(skb->data, 0, EM_HDR_LEN);
	}
	buf_len = skb->len;
	mapping = pci_map_single(rtlpci->pdev, skb->data, skb->len,
				 PCI_DMA_TODEVICE);
	if (pci_dma_mapping_error(rtlpci->pdev, mapping)) {
		RT_TRACE(rtlpriv, COMP_SEND, DBG_TRACE, "DMA mapping error\n");
		return;
	}

	if (pbd_desc_tx)
		rtl8723de_pre_fill_tx_bd_desc(hw, pbd_desc_tx, pdesc, hw_queue,
					      skb, mapping);

	CLEAR_PCI_TX_DESC_CONTENT(pdesc, sizeof(struct tx_desc_8723de));
	if (ieee80211_is_nullfunc(fc) || ieee80211_is_ctl(fc)) {
		firstseg = true;
		lastseg = true;
	}
	if (firstseg) {
		if (rtlhal->earlymode_enable) {
			SET_TX_DESC_PKT_OFFSET_8723D(pdesc, 1);
			SET_TX_DESC_OFFSET_8723D(pdesc, TX_DESC_SIZE +
					   EM_HDR_LEN);
			if (ptcb_desc->empkt_num) {
				RT_TRACE(rtlpriv, COMP_SEND, DBG_TRACE,
					 "Insert 8 byte.pTcb->EMPktNum:%d\n",
					  ptcb_desc->empkt_num);
				_rtl8723de_insert_emcontent(ptcb_desc,
							    (u8 *)(skb->data));
			}
		} else {
			SET_TX_DESC_OFFSET_8723D(pdesc, TX_DESC_SIZE);
		}

		/* tx report */
		rtl_get_tx_report(ptcb_desc, pdesc, hw);

		/* ptcb_desc->use_driver_rate = true; */
		SET_TX_DESC_TX_RATE_8723D(pdesc, ptcb_desc->hw_rate);
		if (ptcb_desc->hw_rate > DESC92C_RATEMCS0)
			short_gi = (ptcb_desc->use_shortgi) ? 1 : 0;
		else
			short_gi = (ptcb_desc->use_shortpreamble) ? 1 : 0;

		SET_TX_DESC_DATA_SHORT_8723D(pdesc, short_gi);

		if (info->flags & IEEE80211_TX_CTL_AMPDU) {
			SET_TX_DESC_AGG_ENABLE_8723D(pdesc, 1);
			SET_TX_DESC_MAX_AGG_NUM_8723D(pdesc, 0x14);
		}
		SET_TX_DESC_SEQ_8723D(pdesc, seq_number);
		SET_TX_DESC_RTS_ENABLE_8723D(pdesc, ((ptcb_desc->rts_enable &&
						!ptcb_desc->cts_enable) ?
						1 : 0));
		SET_TX_DESC_HW_RTS_ENABLE_8723D(pdesc, 0);
		SET_TX_DESC_CTS2SELF_8723D(pdesc, ((ptcb_desc->cts_enable) ?
					      1 : 0));

		SET_TX_DESC_RTS_RATE_8723D(pdesc, ptcb_desc->rts_rate);

		SET_TX_DESC_RTS_SC_8723D(pdesc, ptcb_desc->rts_sc);
		SET_TX_DESC_RTS_SHORT_8723D(pdesc,
			((ptcb_desc->rts_rate <= DESC92C_RATE54M) ?
			 (ptcb_desc->rts_use_shortpreamble ? 1 : 0) :
			 (ptcb_desc->rts_use_shortgi ? 1 : 0)));

		if (ptcb_desc->tx_enable_sw_calc_duration)
			SET_TX_DESC_NAV_USE_HDR_8723D(pdesc, 1);

		if (bw_40) {
			if (ptcb_desc->packet_bw == HT_CHANNEL_WIDTH_20_40) {
				SET_TX_DESC_DATA_BW_8723D(pdesc, 1);
				SET_TX_DESC_DATA_SC_8723D(pdesc, 3);
			} else {
				SET_TX_DESC_DATA_BW_8723D(pdesc, 0);
				SET_TX_DESC_DATA_SC_8723D(pdesc, mac->cur_40_prime_sc);
			}
		} else {
			SET_TX_DESC_DATA_BW_8723D(pdesc, 0);
			SET_TX_DESC_DATA_SC_8723D(pdesc, 0);
		}

		SET_TX_DESC_PKT_SIZE_8723D(pdesc, (u16) skb_len);
		if (sta) {
			u8 ampdu_density = sta->ht_cap.ampdu_density;
			SET_TX_DESC_AMPDU_DENSITY_8723D(pdesc, ampdu_density);
		}
		if (info->control.hw_key) {
			struct ieee80211_key_conf *keyconf =
						info->control.hw_key;
			switch (keyconf->cipher) {
			case WLAN_CIPHER_SUITE_WEP40:
			case WLAN_CIPHER_SUITE_WEP104:
			case WLAN_CIPHER_SUITE_TKIP:
				SET_TX_DESC_SEC_TYPE_8723D(pdesc, 0x1);
				break;
			case WLAN_CIPHER_SUITE_CCMP:
				SET_TX_DESC_SEC_TYPE_8723D(pdesc, 0x3);
				break;
			default:
				SET_TX_DESC_SEC_TYPE_8723D(pdesc, 0x0);
				break;
			}
		}

		SET_TX_DESC_QUEUE_SEL_8723D(pdesc, fw_qsel);
		SET_TX_DESC_DATA_RATE_FB_LIMIT_8723D(pdesc, 0x1F);
		SET_TX_DESC_RTS_RATE_FB_LIMIT_8723D(pdesc, 0xF);
		SET_TX_DESC_DISABLE_FB_8723D(pdesc, ptcb_desc->disable_ratefallback ?
					      1 : 0);
		SET_TX_DESC_USE_RATE_8723D(pdesc, ptcb_desc->use_driver_rate ? 1 : 0);

		/* Set TxRate and RTSRate in TxDesc  */
		/* This prevent Tx initial rate of new-coming packets */
		/* from being overwritten by retried  packet rate.*/
		if (ieee80211_is_data_qos(fc)) {
			if (mac->rdg_en) {
				RT_TRACE(rtlpriv, COMP_SEND, DBG_TRACE,
					 "Enable RDG function.\n");
				SET_TX_DESC_RDG_ENABLE_8723D(pdesc, 1);
				SET_TX_DESC_HTC_8723D(pdesc, 1);
			}
		}
	}

	SET_TX_DESC_TX_BUFFER_SIZE_8723D(pdesc, (u16) buf_len);
	/* if (rtlpriv->dm.useramask) { */
	if (1) {
		SET_TX_DESC_RATE_ID_8723D(pdesc, ptcb_desc->ratr_index);
		SET_TX_DESC_MACID_8723D(pdesc, ptcb_desc->mac_id);
	} else {
		SET_TX_DESC_RATE_ID_8723D(pdesc, 0xC + ptcb_desc->ratr_index);
		SET_TX_DESC_MACID_8723D(pdesc, ptcb_desc->mac_id);
	}
	if (!ieee80211_is_data_qos(fc))  {
		SET_TX_DESC_HWSEQ_EN_8723D(pdesc, 1);
		SET_TX_DESC_HWSEQ_SEL_8723D(pdesc, 0);
	}
	SET_TX_DESC_MORE_FRAG_8723D(pdesc, (lastseg ? 0 : 1));
	if (is_multicast_ether_addr(ieee80211_get_DA(hdr)) ||
	    is_broadcast_ether_addr(ieee80211_get_DA(hdr))) {
		SET_TX_DESC_BMC_8723D(pdesc, 1);
	}

	RT_TRACE(rtlpriv, COMP_SEND, DBG_TRACE, "\n");
}

void rtl8723de_tx_fill_special_desc(struct ieee80211_hw *hw, u8 *pdesc,
				    u8 *pbd_desc, struct sk_buff *skb,
				    u8 hw_queue)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
	u8 fw_queue = QSLT_BEACON;
	u8 txdesc_len = TX_DESC_SIZE;
	dma_addr_t mapping;

	if (hw_queue != BEACON_QUEUE)
		return;

	mapping = pci_map_single(rtlpci->pdev, skb->data, skb->len,
				 PCI_DMA_TODEVICE);

	if (pci_dma_mapping_error(rtlpci->pdev, mapping)) {
		RT_TRACE(rtlpriv, COMP_SEND, DBG_DMESG, "DMA mapping error");
		return;
	}

	rtl8723de_pre_fill_tx_bd_desc(hw, pbd_desc, pdesc, hw_queue, skb,
				      mapping);

	CLEAR_PCI_TX_DESC_CONTENT(pdesc, txdesc_len);

	SET_TX_DESC_OFFSET_8723D(pdesc, txdesc_len);

	SET_TX_DESC_TX_RATE_8723D(pdesc, DESC92C_RATE1M);

	SET_TX_DESC_SEQ_8723D(pdesc, 0);

	SET_TX_DESC_QUEUE_SEL_8723D(pdesc, fw_queue);

	SET_TX_DESC_TX_BUFFER_SIZE_8723D(pdesc, (u16)(skb->len));

	SET_TX_DESC_RATE_ID_8723D(pdesc, 0);
	SET_TX_DESC_MACID_8723D(pdesc, 0);

	SET_TX_DESC_PKT_SIZE_8723D((u8 *)pdesc, (u16)(skb->len));

	SET_TX_DESC_USE_RATE_8723D(pdesc, 1);

	RT_PRINT_DATA(rtlpriv, COMP_CMD, DBG_LOUD, "Tx beacon content\n",
		      pdesc, txdesc_len);
}

void rtl8723de_set_desc(struct ieee80211_hw *hw, u8 *pdesc,
			bool istx, u8 desc_name, u8 *val)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	u8 q_idx = *val;
	bool dma64 = rtlpriv->cfg->mod_params->dma64;

	if (istx) {
		switch (desc_name) {
		case HW_DESC_OWN: {
			struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
			struct rtl8192_tx_ring *ring = &rtlpci->tx_ring[q_idx];
			u16 max_tx_desc = ring->entries;

			if (q_idx == BEACON_QUEUE) {
				/* in case of beacon, pdesc is BD desc. */
				u8 *pbd_desc = pdesc;

				ring->cur_tx_wp = 0;
				ring->cur_tx_rp = 0;
				SET_TX_BUFF_DESC_OWN(pbd_desc, 1);
				return;
			}

			/* make sure tx desc is available by caller */
			ring->cur_tx_wp = ((ring->cur_tx_wp + 1) % max_tx_desc);

			rtl_write_word(
				rtlpriv,
				get_desc_address_from_queue_index(
					q_idx),
				ring->cur_tx_wp);
		} break;
		}
	} else {
		switch (desc_name) {
		case HW_DESC_RX_PREPARE:
			SET_RX_BUFFER_DESC_LS(pdesc, 0);
			SET_RX_BUFFER_DESC_FS(pdesc, 0);
			SET_RX_BUFFER_DESC_TOTAL_LENGTH(pdesc, 0);

			SET_RX_BUFFER_DESC_DATA_LENGTH(
				pdesc, MAX_RECEIVE_BUFFER_SIZE + RX_DESC_SIZE);

			SET_RX_BUFFER_PHYSICAL_LOW(
				pdesc, (*(dma_addr_t *)val) & DMA_BIT_MASK(32));
			SET_RX_BUFFER_PHYSICAL_HIGH(
				pdesc, ((u64)(*(dma_addr_t *)val) >> 32),
				dma64);
			break;
		default:
			WARN_ONCE(true, "ERR rxdesc :%d not process\n",
				  desc_name);
			break;
		}
	}
}

u64 rtl8723de_get_desc(struct ieee80211_hw *hw,
		       u8 *pdesc, bool istx, u8 desc_name)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	u64 ret = 0;
	u8 *pbd_desc = pdesc;
	bool dma64 = rtlpriv->cfg->mod_params->dma64;

	if (istx) {
		switch (desc_name) {
		case HW_DESC_TXBUFF_ADDR:
			ret = GET_TXBUFFER_DESC_ADDR_LOW(pbd_desc, 1);
			ret |= (u64)GET_TXBUFFER_DESC_ADDR_HIGH(pbd_desc, 1,
								dma64) << 32;
			break;
		default:
			WARN_ONCE(true, "ERR txdesc :%d not process\n",
				  desc_name);
			break;
		}
	} else {
		switch (desc_name) {
		case HW_DESC_RXPKT_LEN:
			ret = GET_RX_DESC_PKT_LEN_8723D(pdesc);
			break;
		default:
			WARN_ONCE(true, "ERR rxdesc :%d not process\n",
				  desc_name);
			break;
		}
	}
	return ret;
}

bool rtl8723de_is_tx_desc_closed(struct ieee80211_hw *hw,
				 u8 hw_queue, u16 index)
{
	struct rtl_pci *rtlpci = rtl_pcidev(rtl_pcipriv(hw));
	struct rtl_priv *rtlpriv = rtl_priv(hw);
	bool ret = false;
	struct rtl8192_tx_ring *ring = &rtlpci->tx_ring[hw_queue];
	u16 cur_tx_rp, cur_tx_wp;
	u16 tmp16;

	/*
	 * design rule:
	 *     idx <= cur_tx_rp <= hw_rp <= cur_tx_wp = hw_wp
	 */

	if (index == ring->cur_tx_rp) {
		/* update only if sw_rp reach hw_rp */
		tmp16 = rtl_read_word(
			    rtlpriv,
			    get_desc_address_from_queue_index(hw_queue) + 2);

		cur_tx_rp = tmp16 & 0x01ff;
		cur_tx_wp = ring->cur_tx_wp;

		/* don't need to update ring->cur_tx_wp */
		ring->cur_tx_rp = cur_tx_rp;
	}

	if (index == ring->cur_tx_rp)
		ret = false;	/* no more */
	else
		ret = true;	/* more */

	if (hw_queue == BEACON_QUEUE)
		ret = true;

	if (rtlpriv->rtlhal.driver_is_goingto_unload ||
	    rtlpriv->psc.rfoff_reason > RF_CHANGE_BY_PS)
		ret = true;

	return ret;
}

void rtl8723de_tx_polling(struct ieee80211_hw *hw, u8 hw_queue)
{
	struct rtl_priv *rtlpriv = rtl_priv(hw);

	if (hw_queue == BEACON_QUEUE) {
		/* kick start */
		rtl_write_byte(
			rtlpriv, REG_RX_RXBD_NUM_8723D + 1,
			rtl_read_byte(rtlpriv, REG_RX_RXBD_NUM_8723D + 1) |
				BIT(4));
	}
}

u32 rtl8723de_rx_command_packet(struct ieee80211_hw *hw,
				const struct rtl_stats *status,
				struct sk_buff *skb)
{
	u32 result = 0;
	struct rtl_priv *rtlpriv = rtl_priv(hw);

	switch (status->packet_report_type) {
	case NORMAL_RX:
			result = 0;
			break;
	case C2H_PACKET:
			rtl8723de_c2h_packet_handler(hw, skb->data,
						     (u8)skb->len);
			result = 1;
			break;
	default:
			RT_TRACE(rtlpriv, COMP_RECV, DBG_TRACE,
				 "No this packet type!!\n");
			break;
	}

	return result;
}

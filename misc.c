/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include <string.h>

#include "misc.h"
#include "settings.h"
#include "driver/bk4819.h"

const uint8_t     fm_radio_countdown_500ms         =  2000 / 500;  // 2 seconds
const uint16_t    fm_play_countdown_scan_10ms      =   100 / 10;   // 100ms
const uint16_t    fm_play_countdown_noscan_10ms    =  1200 / 10;   // 1.2 seconds
const uint16_t    fm_restore_countdown_10ms        =  5000 / 10;   // 5 seconds

const uint8_t     vfo_state_resume_countdown_500ms =  2500 / 500;  // 2.5 seconds

const uint8_t     menu_timeout_500ms               =  90000 / 500;  // 90 seconds
const uint16_t    menu_timeout_long_500ms          = 120000 / 500;  // 2 minutes

const uint8_t     DTMF_RX_live_timeout_500ms       =  6000 / 500;  // 6 seconds live decoder on screen
#ifdef ENABLE_DTMF_CALLING
const uint8_t     DTMF_RX_timeout_500ms            = 10000 / 500;  // 10 seconds till we wipe the DTMF receiver
const uint8_t     DTMF_decode_ring_countdown_500ms = 15000 / 500;  // 15 seconds .. time we sound the ringing for
const uint8_t     DTMF_txstop_countdown_500ms      =  3000 / 500;  // 6 seconds
#endif

const uint8_t     key_input_timeout_500ms          =  8000 / 500;  // 8 seconds

const uint16_t    key_repeat_delay_10ms            =   400 / 10;   // 400ms
const uint16_t    key_repeat_10ms                  =    80 / 10;   // 80ms .. MUST be less than 'key_repeat_delay'
const uint16_t    key_debounce_10ms                =    20 / 10;   // 20ms

const uint8_t     scan_delay_10ms                  =   210 / 10;   // 210ms

const uint16_t    dual_watch_count_after_tx_10ms   =  3600 / 10;   // 3.6 sec after TX ends
const uint16_t    dual_watch_count_after_rx_10ms   =  1000 / 10;   // 1 sec after RX ends ?
const uint16_t    dual_watch_count_after_1_10ms    =  5000 / 10;   // 5 sec
const uint16_t    dual_watch_count_after_2_10ms    =  3600 / 10;   // 3.6 sec
const uint16_t    dual_watch_count_noaa_10ms       =    70 / 10;   // 70ms
#ifdef ENABLE_VOX
	const uint16_t dual_watch_count_after_vox_10ms  =   200 / 10;   // 200ms
#endif
const uint16_t    dual_watch_count_toggle_10ms     =   100 / 10;   // 100ms between VFO toggles

const uint16_t    scan_pause_delay_in_1_10ms       =  5000 / 10;   // 5 seconds
const uint16_t    scan_pause_delay_in_2_10ms       =   500 / 10;   // 500ms
const uint16_t    scan_pause_delay_in_3_10ms       =   200 / 10;   // 200ms
const uint16_t    scan_pause_delay_in_4_10ms       =   300 / 10;   // 300ms
const uint16_t    scan_pause_delay_in_5_10ms       =  1000 / 10;   // 1 sec
const uint16_t    scan_pause_delay_in_6_10ms       =   100 / 10;   // 100ms
const uint16_t    scan_pause_delay_in_7_10ms       =  3600 / 10;   // 3.6 seconds

const uint16_t    battery_save_count_10ms          = 10000 / 10;   // 10 seconds

const uint16_t    power_save1_10ms                 =   100 / 10;   // 100ms
const uint16_t    power_save2_10ms                 =   200 / 10;   // 200ms

#ifdef ENABLE_VOX
	const uint16_t    vox_stop_count_down_10ms         =  1000 / 10;   // 1 second
#endif

const uint16_t    NOAA_countdown_10ms              =  5000 / 10;   // 5 seconds
const uint16_t    NOAA_countdown_2_10ms            =   500 / 10;   // 500ms
const uint16_t    NOAA_countdown_3_10ms            =   200 / 10;   // 200ms

const uint32_t    gDefaultAesKey[4]                = {0x4AA5CC60, 0x0312CC5F, 0xFFD2DABB, 0x6BBA7F92};

const uint8_t     gMicGain_dB2[5]                  = {3, 8, 16, 24, 31};

bool              gSetting_350TX;
#ifdef ENABLE_DTMF_CALLING
bool              gSetting_KILLED;
#endif
bool              gSetting_200TX;
bool              gSetting_500TX;
bool              gSetting_350EN;
uint8_t           gSetting_F_LOCK;
bool              gSetting_ScrambleEnable;

#ifdef ENABLE_ARDF

bool              gSetting_ARDFEnable = ARDF_DEFAULT_ENABLE;
uint32_t          gARDFTime10ms = 0;
uint32_t          gARDFFoxDuration10ms = ARDF_DEFAULT_FOX_DURATION;  /* 60s * 100 ticks per second */
uint32_t          gARDFFoxDuration10ms_corr = ARDF_DEFAULT_FOX_DURATION + (ARDF_DEFAULT_FOX_DURATION * ARDF_CLOCK_CORR_TICKS_PER_MIN)/6000;
uint8_t           gARDFNumFoxes = ARDF_DEFAULT_NUM_FOXES;
uint8_t           gARDFActiveFox = 0;
uint8_t           gARDFGainRemember = ARDF_DEFAULT_GAIN_REMEMBER; /* remember gain on VFO 1 by default. */
uint8_t           gARDFCycleEndBeep_s = ARDF_CYCLE_END_BEEP_S_DEFAULT;
bool              gARDFPlayEndBeep = false;
unsigned int      gARDFRssiMax = 0; /* max rssi of last half second */
uint8_t           gARDFMemModeFreqToggleCnt_s = 0; /* toggle memory bank/frequency display every x s */
bool              gARDFRequestSaveEEPROM = true;
int16_t           gARDFClockCorrAddTicksPerMin = ARDF_CLOCK_CORR_TICKS_PER_MIN;
#ifdef ARDF_ENABLE_SHOW_DEBUG_DATA
int16_t	          gARDFdebug = 0;
#endif

uint8_t ardf_gain_index[2][ARDF_NUM_FOX_MAX];


// {0x03BE, -7},   //  0 .. 3 5 3 6 ..   0dB  -4dB  0dB  -3dB ..  -7dB original
#define ARDF_ORIG_GAIN_DB -7

t_ardf_gain_table ardf_gain_table[] =
{
	{0x0000, -90},         //   0 .. 0 0 0 0 .. -27dB -25dB -6dB -32dB .. -90dB
	{0x0020, -85},         //   1 .. 0 1 0 0 .. -27dB -20dB -6dB -32dB .. -85dB
	{0x0128, -80},         //   2 .. 1 1 1 0 .. -24dB -20dB -4dB -32dB .. -80dB
	{0x0060, -75},         //   3 .. 0 3 0 0 .. -27dB -10dB -6dB -32dB .. -75dB
	{0x0202, -70},         //   4 .. 2 0 0 2 .. -19dB -25dB -6dB -20dB .. -70dB
	{0x0081, -65},         //   5 .. 0 4 0 1 .. -27dB  -6dB -6dB -26dB .. -65dB
	{0x0053, -60},         //   6 .. 0 2 2 3 .. -27dB -16dB -2dB -15dB .. -60dB
	{0x00AA, -55},         //   7 .. 0 5 1 2 .. -27dB  -4dB -4dB -20dB .. -55dB
	{0x004E, -50},         //   8 .. 0 2 1 6 .. -27dB -16dB -4dB  -3dB .. -50dB
	{0x02E2, -45},         //   9 .. 2 7 0 2 .. -19dB   0dB -6dB -20dB .. -45dB
	{0x02B3, -40},         //  10 .. 2 5 2 3 .. -19dB  -4dB -2dB -15dB .. -40dB
	{0x0097, -35},         //  11 .. 0 4 2 7 .. -27dB  -6dB -2dB   0dB .. -35dB
	{0x01B7, -30},         //  12 .. 1 5 2 7 .. -24dB  -4dB -2dB   0dB .. -30dB
	{0x0392, -28},         //  13 .. 3 4 2 2 ..   0dB  -6dB -2dB -20dB .. -28dB
	{0x0336, -25},         //  14 .. 3 1 2 6 ..   0dB -20dB -2dB  -3dB .. -25dB
	{0x034F, -20},         //  15 .. 3 2 1 7 ..   0dB -16dB -4dB   0dB .. -20dB
	{0x039C, -15},         //  16 .. 3 4 3 4 ..   0dB  -6dB  0dB  -9dB .. -15dB
	{0x03A7, -10},         //  17 .. 3 5 0 7 ..   0dB  -4dB -6dB   0dB .. -10dB
	{0x03DE, -5},          //  18 .. 3 6 3 6 ..   0dB  -2dB  0dB  -3dB ..  -5dB
	{0x03FF, 0},           //  19 .. 3 7 3 7 ..   0dB   0dB  0dB   0dB ..   0dB
};


void ARDF_init(void)
{
	for ( uint8_t i; i<ARDF_NUM_FOX_MAX; i++ )
	{
		ardf_gain_index[0][i] = ARDF_GAIN_INDEX_DEFAULT;
		ardf_gain_index[1][i] = ARDF_GAIN_INDEX_DEFAULT;
	}
}


void ARDF_GainIncr(void)
{
	uint8_t vfo = gEeprom.RX_VFO;
	uint8_t activefox = gARDFActiveFox;
	
	if ( ARDF_ActVfoHasGainRemember(vfo) == false )
	{
		// do not remember fox gains on this vfo
		activefox = 0;
	}
	
        if ( ardf_gain_index[vfo][activefox] < (sizeof(ardf_gain_table)/sizeof(t_ardf_gain_table))-1 )
        {
                ardf_gain_index[vfo][activefox]++;
        }
}


void ARDF_GainDecr(void)
{
	uint8_t vfo = gEeprom.RX_VFO;
	uint8_t activefox = gARDFActiveFox;

        if ( ARDF_ActVfoHasGainRemember(vfo) == false )
        {
                // do not remember fox gains on this vfo
                activefox = 0;
        }


        if ( ardf_gain_index[vfo][activefox] > 0 )
        {
                ardf_gain_index[vfo][activefox]--;
        }
}


uint8_t ARDF_Get_GainIndex(uint8_t vfo)
{
	if ( ARDF_ActVfoHasGainRemember(vfo) == false )
	{
		// remember fox gains not on this vfo
		return ardf_gain_index[vfo][0];
	}
	else
	{
		return ardf_gain_index[vfo][gARDFActiveFox];
	}
}


bool ARDF_ActVfoHasGainRemember(uint8_t vfo)
{
        /* "OFF", 0
        "VFO A", 1
        "VFO B", 2
        "BOTH" 3 */
	
	if ( (vfo+1) & gARDFGainRemember )
	{
		return true;
	}
	else
	{
		return false;
	}
}


void ARDF_ActivateGainIndex(void)
{
	BK4819_WriteRegister( BK4819_REG_13, ardf_gain_table[ ARDF_Get_GainIndex(gEeprom.RX_VFO) ].reg_val );
	gARDFRssiMax = 0;
	gUpdateDisplay = true;
}


int32_t ARDF_GetRestTime_s(void)
{
	return (int32_t)(gARDFFoxDuration10ms - gARDFTime10ms * gARDFFoxDuration10ms/gARDFFoxDuration10ms_corr )/100;
}

int8_t ARDF_Get_GainDiff(void)
{
        return ARDF_ORIG_GAIN_DB - ardf_gain_table[ ARDF_Get_GainIndex(gEeprom.RX_VFO) ].gain_dB;
}

#endif

enum BacklightOnRxTx_t gSetting_backlight_on_tx_rx;

#ifdef ENABLE_AM_FIX
	bool          gSetting_AM_fix;
#endif

#ifdef ENABLE_AUDIO_BAR
	bool          gSetting_mic_bar;
#endif
bool              gSetting_live_DTMF_decoder;
uint8_t           gSetting_battery_text;

bool              gMonitor = false;           // true opens the squelch

uint32_t          gCustomAesKey[4];
bool              bHasCustomAesKey;
uint32_t          gChallenge[4];
uint8_t           gTryCount;

uint16_t          gEEPROM_RSSI_CALIB[7][4];

uint16_t          gEEPROM_1F8A;
uint16_t          gEEPROM_1F8C;

ChannelAttributes_t gMR_ChannelAttributes[FREQ_CHANNEL_LAST + 1];

volatile uint16_t gBatterySaveCountdown_10ms = battery_save_count_10ms;

volatile bool     gPowerSaveCountdownExpired;
volatile bool     gSchedulePowerSave;

volatile bool     gScheduleDualWatch = true;

volatile uint16_t gDualWatchCountdown_10ms;
bool              gDualWatchActive           = false;

volatile uint8_t  gSerialConfigCountDown_500ms;

volatile bool     gNextTimeslice_500ms;

volatile uint16_t gTxTimerCountdown_500ms;
volatile bool     gTxTimeoutReached;

volatile uint16_t gTailToneEliminationCountdown_10ms;

volatile uint8_t    gVFOStateResumeCountdown_500ms;

#ifdef ENABLE_NOAA
	volatile uint16_t gNOAA_Countdown_10ms;
#endif

bool              gEnableSpeaker;
uint8_t           gKeyInputCountdown = 0;
uint8_t           gKeyLockCountdown;
uint8_t           gRTTECountdown_10ms;
bool              bIsInLockScreen;
uint8_t           gUpdateStatus;
uint8_t           gFoundCTCSS;
uint8_t           gFoundCDCSS;
bool              gEndOfRxDetectedMaybe;

int16_t           gVFO_RSSI[2];
uint8_t           gVFO_RSSI_bar_level[2];

uint8_t           gReducedService;
uint8_t           gBatteryVoltageIndex;
bool     		  gCssBackgroundScan;

volatile bool     gScheduleScanListen = true;
volatile uint16_t gScanPauseDelayIn_10ms;

#if defined(ENABLE_ALARM) || defined(ENABLE_TX1750)
	AlarmState_t  gAlarmState;
#endif
uint16_t          gMenuCountdown;
bool              gPttWasReleased;
bool              gPttWasPressed;
uint8_t           gKeypadLocked;
bool              gFlagReconfigureVfos;
uint8_t           gVfoConfigureMode;
bool              gFlagResetVfos;
bool              gRequestSaveVFO;
uint8_t           gRequestSaveChannel;
bool              gRequestSaveSettings;
#ifdef ENABLE_FMRADIO
	bool          gRequestSaveFM;
#endif
bool              gFlagPrepareTX;

bool              gFlagAcceptSetting;
bool              gFlagRefreshSetting;

#ifdef ENABLE_FMRADIO
	bool          gFlagSaveFM;
#endif
bool              g_CDCSS_Lost;
uint8_t           gCDCSSCodeType;
bool              g_CTCSS_Lost;
bool              g_CxCSS_TAIL_Found;
#ifdef ENABLE_VOX
	bool          g_VOX_Lost;
	bool          gVOX_NoiseDetected;
	uint16_t      gVoxResumeCountdown;
	uint16_t      gVoxPauseCountdown;
#endif
bool              g_SquelchLost;

volatile uint16_t gFlashLightBlinkCounter;

bool              gFlagEndTransmission;
uint8_t           gNextMrChannel;
ReceptionMode_t   gRxReceptionMode;

bool              gRxVfoIsActive;
#ifdef ENABLE_ALARM
	uint8_t       gAlarmToneCounter;
	uint16_t      gAlarmRunningCounter;
#endif
bool              gKeyBeingHeld;
bool              gPttIsPressed;
uint8_t           gPttDebounceCounter;
uint8_t           gMenuListCount;
uint8_t           gBackup_CROSS_BAND_RX_TX;
uint8_t           gScanDelay_10ms;
uint8_t           gFSKWriteIndex;

#ifdef ENABLE_NOAA
	bool          gIsNoaaMode;
	uint8_t       gNoaaChannel;
#endif

bool              gUpdateDisplay;

bool              gF_LOCK = false;

uint8_t           gShowChPrefix;

volatile bool     gNextTimeslice;
volatile uint8_t  gFoundCDCSSCountdown_10ms;
volatile uint8_t  gFoundCTCSSCountdown_10ms;
#ifdef ENABLE_VOX
	volatile uint16_t gVoxStopCountdown_10ms;
#endif
volatile bool     gNextTimeslice40ms;
#ifdef ENABLE_NOAA
	volatile uint16_t gNOAACountdown_10ms = 0;
	volatile bool     gScheduleNOAA       = true;
#endif
volatile bool     gFlagTailToneEliminationComplete;
#ifdef ENABLE_FMRADIO
	volatile bool gScheduleFM;
#endif

volatile uint8_t  boot_counter_10ms;

uint8_t           gIsLocked = 0xFF;


inline void FUNCTION_NOP() { ; }


int32_t NUMBER_AddWithWraparound(int32_t Base, int32_t Add, int32_t LowerLimit, int32_t UpperLimit)
{
	Base += Add;

	if (Base == 0x7fffffff || Base < LowerLimit)
		return UpperLimit;

	if (Base > UpperLimit)
		return LowerLimit;

	return Base;
}

unsigned long StrToUL(const char * str)
{
	unsigned long ul = 0;
	for(uint8_t i = 0; i < strlen(str); i++){
		char c = str[i];
		if(c < '0' || c > '9')
			break;
		ul = ul * 10 + (uint8_t)(c-'0');
	}
	return ul;
}

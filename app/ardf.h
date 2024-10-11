/* Copyright 2023 Dennis Real
 * https://github.com/reald
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

#ifndef APP_ARDF_H
#define APP_ARDF_H

#ifdef ENABLE_ARDF

#include "driver/keyboard.h"



#define ARDF_NUM_FOX_MAX 10
#define ARDF_GAIN_INDEX_DEFAULT 15
#define ARDF_GAIN_INDEX_MIDDLE 7
#define ARDF_CYCLE_END_BEEP_S_DEFAULT 12 // warn with beep 12s before end of fox cycle
#define ARDF_CYCLE_END_BEEP_S_MAX 30

#define ARDF_MEM_MODE_FREQ_TOGGLE_S 3 // in frequency mode: toggle memory number and frequency every 3 s

#define ARDF_DEFAULT_ENABLE true
#define ARDF_DEFAULT_NUM_FOXES 5
#define ARDF_DEFAULT_FOX_DURATION 6000
#define ARDF_DEFAULT_GAIN_REMEMBER 1

#define ARDF_CLOCK_CORR_TICKS_PER_MIN (+00) // default clock correction

//#define ARDF_ENABLE_SHOW_DEBUG_DATA



typedef struct
{
    uint16_t reg_val;
    int8_t   gain_dB;
} __attribute__((packed)) t_ardf_gain_table;



extern t_ardf_gain_table     ardf_gain_table[];
extern uint8_t               ardf_gain_index[2][ARDF_NUM_FOX_MAX];

extern uint32_t              gARDFTime10ms;
extern uint32_t              gARDFFoxDuration10ms;
extern uint32_t              gARDFFoxDuration10ms_corr;
extern uint8_t               gARDFNumFoxes;
extern uint8_t               gARDFActiveFox;
extern uint8_t               gARDFGainRemember;
extern uint8_t               gARDFCycleEndBeep_s;
extern bool                  gARDFPlayEndBeep;
extern unsigned int          gARDFRssiMax;
extern uint8_t               gARDFMemModeFreqToggleCnt_s;
extern bool                  gARDFRequestSaveEEPROM;
extern int16_t               gARDFClockCorrAddTicksPerMin;
#ifdef ARDF_ENABLE_SHOW_DEBUG_DATA
extern int16_t               gARDFdebug;
extern int16_t               gARDFdebug2;
#endif
extern unsigned int          gARDFRssiMax;



extern void ARDF_10ms(void);
extern void ARDF_500ms(void);
extern void ARDF_init(void);
extern void ARDF_GainIncr(void);
extern void ARDF_GainDecr(void);
extern uint8_t ARDF_Get_GainIndex(uint8_t vfo);
extern bool ARDF_ActVfoHasGainRemember(uint8_t vfo);
extern void ARDF_ActivateGainIndex(void);
extern int32_t ARDF_GetRestTime_s(void);
extern int8_t ARDF_Get_GainDiff(void);


#endif

#endif

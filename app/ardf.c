/* Copyright 2024 Dennis Real
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

#ifdef ENABLE_ARDF

#include "app/ardf.h"
#include "driver/bk4819.h"
#include "driver/system.h"
#include "audio.h"
#include "misc.h"
#include "settings.h"
#include "ui/main.h"
#include "ui/ui.h"
#include "ui/ardf.h"



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
int16_t           gARDFdebug = 0;
#endif



/* void ARDF_ProcessKeys(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
   bool a = bKeyPressed;
   bool b = bKeyHeld;
   a = a;
   b = b;
   
   switch (Key) 
   {
      case KEY_0:
      case KEY_1:
      case KEY_2:
      case KEY_3:
      case KEY_4:
      case KEY_5:
      case KEY_6:
      case KEY_7:
      case KEY_8:
      case KEY_9:
         //AIRCOPY_Key_DIGITS(Key, bKeyPressed, bKeyHeld);
         break;
      case KEY_MENU:
         //AIRCOPY_Key_MENU(bKeyPressed, bKeyHeld);
         break;
      case KEY_EXIT:
         //AIRCOPY_Key_EXIT(bKeyPressed, bKeyHeld);
         break;
      default:
         break;
   }
} */



void ARDF_10ms(void)
{

   if ( gARDFTime10ms >= gARDFFoxDuration10ms_corr )
   {
      // new fox cycle
      gARDFTime10ms = 0;
      
      if ( (gARDFActiveFox + 1) >= gARDFNumFoxes )
      {
         gARDFActiveFox = 0;
      }
      else
      {
         gARDFActiveFox++;
      }

      if ( gSetting_ARDFEnable )
      {
         // recall last gain index if needed
         ARDF_ActivateGainIndex();
      }
      
      if ( gScreenToDisplay == DISPLAY_ARDF )
      {
         // update complete screen
         UI_DisplayARDF();
      }   

   }
   else if ( (gScreenToDisplay == DISPLAY_ARDF) && ( (gARDFTime10ms % 50) == 0) )
   {
      // update most important values ~2 times per second
      UI_DisplayARDF_Timer();
      UI_DisplayARDF_RSSI();

      gARDFRssiMax = BK4819_GetRSSI();

#ifdef ARDF_ENABLE_SHOW_DEBUG_DATA
      UI_DisplayARDF_Debug();
#elif defined(ENABLE_AGC_SHOW_DATA)
      UI_MAIN_PrintAGC(true);
#else
      center_line = CENTER_LINE_RSSI;
      DisplayRSSIBar(true);
#endif

   }
   else if ( (gScreenToDisplay == DISPLAY_ARDF) && ( (gARDFTime10ms % 5) == 0) )
   {
      // reduce call rate if i2c traffic is too high
      unsigned int rssi = BK4819_GetRSSI();
      if ( rssi > gARDFRssiMax )
      {
         gARDFRssiMax = rssi;
      }
   }

}



void ARDF_500ms(void)
{
   static uint8_t u8Secnd = 0;

   if ( gSetting_ARDFEnable && gScreenToDisplay==DISPLAY_MAIN )
   {
      // switch to ardf screen
      GUI_SelectNextDisplay(DISPLAY_ARDF);
   }
   else if ( !gSetting_ARDFEnable && gScreenToDisplay==DISPLAY_ARDF )
   {
      // ARDF is off now. switch back to main screen
      GUI_SelectNextDisplay(DISPLAY_MAIN);
   }


   u8Secnd++;
   
   if ( u8Secnd >= 2 )
   {

      // update status bar every second
      gUpdateStatus = 1;
      u8Secnd = 0;

      // counter for memory mode / frequency display toggle
      gARDFMemModeFreqToggleCnt_s++;

      if ( (gScreenToDisplay==DISPLAY_ARDF)
            && (gARDFMemModeFreqToggleCnt_s == ARDF_MEM_MODE_FREQ_TOGGLE_S) )
      {
         // screen update only really necessary in memory mode
         UI_DisplayARDF_FreqCh();
      }
      else if ( (gScreenToDisplay==DISPLAY_ARDF)
                 && (gARDFMemModeFreqToggleCnt_s >= (2 * ARDF_MEM_MODE_FREQ_TOGGLE_S)) )
      {
         gARDFMemModeFreqToggleCnt_s = 0;
         // screen update only really necessary in memory mode
         UI_DisplayARDF_FreqCh();
      }


      // generate fox cycle end signal

      if ( (gScreenToDisplay==DISPLAY_ARDF) 
           && (gARDFCycleEndBeep_s != 0)
           && (ARDF_GetRestTime_s() == gARDFCycleEndBeep_s) )
      {
         gARDFPlayEndBeep = true;
         AUDIO_PlayBeep( BEEP_880HZ_60MS_TRIPLE_BEEP );
         gARDFPlayEndBeep = false;
      }

   }

   if ( gARDFRequestSaveEEPROM != false )
   {
      // save ARDF settings to eeprom
      gARDFRequestSaveEEPROM = false;
      SETTINGS_SaveARDF();
   }

}



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

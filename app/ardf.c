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
#include "audio.h"
#include "misc.h"
#include "settings.h"
#include "ui/main.h"
#include "ui/ui.h"
#include "ui/ardf.h"



void ARDF_ProcessKeys(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
bool a = bKeyPressed;
bool b =bKeyHeld;
a=a;
b=b;
	switch (Key) {
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
}



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
   else if ( (gScreenToDisplay == DISPLAY_ARDF) )
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
         UI_DisplayARDF();
      }
      else if ( (gScreenToDisplay==DISPLAY_ARDF)
                 && (gARDFMemModeFreqToggleCnt_s >= (2 * ARDF_MEM_MODE_FREQ_TOGGLE_S)) )
      {
         gARDFMemModeFreqToggleCnt_s = 0;
         // screen update only really necessary in memory mode
         UI_DisplayARDF();
      }

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

#endif

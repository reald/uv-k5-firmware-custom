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

#include <string.h>

#include "app/ardf.h"
#include "driver/bk4819.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "ui/ardf.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/main.h"
#include "ui/ui.h"


void UI_DisplayARDF_Timer(void)
{
   char buffer[4];

   if ( gLowBattery && !gLowBatteryConfirmed )
      return;

   int32_t resttime = ARDF_GetRestTime_s();
   
   if ( resttime <= 99 )
   { 
      sprintf(buffer, "-%02u", resttime );
   }
   else
   {
      sprintf(buffer, "%3u", resttime );
   }
   
   UI_DisplayFrequency(buffer, 12, 0, false);
   
   // note: ST7565_BlitLine(0/1) for this screen update is called in UI_DisplayARDF_RSSI()
}



void UI_DisplayARDF_RSSI(void)
{
   char buffer[4];

   if ( gLowBattery && !gLowBatteryConfirmed )
      return;

   sprintf(buffer, "%03d", gARDFRssiMax);
   UI_DisplayFrequency(buffer, 89, 0, false);

   ST7565_BlitLine(0);
   ST7565_BlitLine(1); 
}



#ifdef ARDF_ENABLE_SHOW_DEBUG_DATA
void UI_DisplayARDF_Debug(void)
{
   char buffer[17];

   sprintf(buffer, "> %d - %d", gARDFdebug, gARDFdebug2);
   UI_PrintStringSmallNormal(buffer, 2, 0, 3);
   ST7565_BlitLine(3);
}
#endif



void UI_DisplayARDF_FreqCh(void)
{
   char buffer[16];
   uint8_t vfo = gEeprom.RX_VFO;

   if ( gLowBattery && !gLowBatteryConfirmed )
      return;

   if ( IS_FREQ_CHANNEL(gEeprom.ScreenChannel[vfo]) 
        || ( (gARDFMemModeFreqToggleCnt_s >= ARDF_MEM_MODE_FREQ_TOGGLE_S) && (gInputBoxIndex == 0) ) )
   {
      // frequency mode or (frequency is shown in memory mode and no input)
      
      if ( gInputBoxIndex == 0 )
      {
         uint32_t frequency = gRxVfo->freq_config_RX.Frequency;
         sprintf(buffer, "%3u.%05u", frequency / 100000, frequency % 100000);
      }
      else
      {
         const char * ascii = INPUTBOX_GetAscii();
         sprintf(buffer, "%.3s.%.3s", ascii, ascii + 3);
      }
      UI_PrintStringSmallBold(buffer, 64, 0, 2);

   }
   else if ( IS_MR_CHANNEL(gEeprom.ScreenChannel[vfo]) )
   {
      // memory mode

      if ( gInputBoxIndex == 0 )
      {
         sprintf(buffer, "M %03u", gEeprom.ScreenChannel[vfo] + 1);
      }
      else
      {
         sprintf(buffer, "M%.3s", INPUTBOX_GetAscii() );  // show the input text
      }
      UI_PrintStringSmallBold(buffer, 64, 0, 2); // 0->128: text centered, but pixel deletion problem
   
   }

   ST7565_BlitLine(2);
}



void UI_DisplayARDF(void)
{
   char buffer[16];
   uint8_t vfo = gEeprom.RX_VFO;
   
   UI_DisplayClear();

   if( gLowBattery && !gLowBatteryConfirmed )
   {
      UI_DisplayPopup("LOW BATTERY");
      ST7565_BlitFullScreen();
      return;
   }

   /* 1. big line */
   if ( gARDFNumFoxes > 0 )
   {
      uint8_t activefox = gARDFActiveFox + 1;
      if ( activefox >= 10 )
         activefox = 0;
      sprintf(buffer, "%d", activefox);
      UI_DisplayFrequency(buffer, 0, 0, false);
   
      UI_DisplayARDF_Timer();
   }

   sprintf(buffer, "%02d", ARDF_Get_GainIndex(vfo) );
   UI_DisplayFrequency(buffer, 57, 0, false);

   UI_DisplayARDF_RSSI();


   /* 2. small line: active vfo */
   VFO_Info_t *vfoInfo = &gEeprom.VfoInfo[vfo];
   ModulationMode_t mod = vfoInfo->Modulation;

   sprintf(buffer, "%c", 'A' + vfo );
   UI_PrintStringSmallBold(buffer, 0, 0, 2);

   UI_PrintStringSmallBold(gModulationStr[mod], 12, 36, 2); // modulation

   char *bw = "";
   switch ( vfoInfo->CHANNEL_BANDWIDTH )
   {
      case BANDWIDTH_NARROW:
   
         bw="N";
         break;
         
      case BANDWIDTH_U2K5:

         bw="N-";
         break;

      case BANDWIDTH_U1K7:
      
         bw="U-";
         break;

      default:

         bw="W";
         break;
   }
   UI_PrintStringSmallBold(bw, 42, 58, 2);

   UI_DisplayARDF_FreqCh();

   /* 3. middle line for debug or rssi bar */

#ifdef ARDF_ENABLE_SHOW_DEBUG_DATA
   UI_DisplayARDF_Debug();
#elif defined(ENABLE_AGC_SHOW_DATA)
   UI_MAIN_PrintAGC(true);
#else
   center_line = CENTER_LINE_RSSI;
   DisplayRSSIBar(true);
#endif

   /* 4. gain index history. show max 5 foxes. */

   if ( ARDF_ActVfoHasGainRemember(vfo) != false )
   {
      int foxliststart = 0;
   
      if ( gARDFNumFoxes > 5 )
      {
         foxliststart = (gARDFNumFoxes - 2 + gARDFActiveFox) % gARDFNumFoxes;
      }

      for ( int i=0; i < MIN(5, gARDFNumFoxes); i++ )
      {
         int idx = (foxliststart + i) % gARDFNumFoxes;
         sprintf(buffer, "%d", idx + 1 );

         const int lineofs = 16;
         
         if ( idx == gARDFActiveFox )
         {
            UI_PrintStringSmallBold(buffer, lineofs + i*16 + i*4, lineofs + (i+1)*16 + i*4, 4);
         }
         else
         {
            UI_PrintStringSmallNormal(buffer, lineofs + i*16 + i*4, lineofs + (i+1)*16 + i*4, 4);
         }
         sprintf(buffer, "%d", ardf_gain_index[vfo][idx] );
         UI_PrintStringSmallBold(buffer, lineofs + i*16 + i*4, lineofs + (i+1)*16 + i*4, 5);

      }

   }


   /* 5. small line: inactive vfo */
   vfoInfo = &gEeprom.VfoInfo[1-vfo];
   mod = vfoInfo->Modulation;

   sprintf(buffer, "%c", 'B' - vfo );
   UI_PrintStringSmallNormal(buffer, 0, 0, 6);

   UI_PrintStringSmallNormal(gModulationStr[mod], 12, 36, 6); // modulation

   switch ( vfoInfo->CHANNEL_BANDWIDTH )
   {
      case BANDWIDTH_NARROW:
   
         UI_PrintStringSmallNormal("N", 42, 58, 6);
         break;
         
      case BANDWIDTH_U2K5:

         UI_PrintStringSmallNormal("N-", 42, 58, 6);
         break;

      case BANDWIDTH_U1K7:
      
         UI_PrintStringSmallNormal("U-", 42, 58, 6);
         break;

      default:

         UI_PrintStringSmallNormal("W", 42, 58, 6);
         break;

   }

   uint32_t frequency = vfoInfo->freq_config_RX.Frequency;

   if ( IS_MR_CHANNEL(gEeprom.ScreenChannel[vfo]) )
   {
      // memory mode
      sprintf(buffer, "M%3u.%04u", frequency / 100000, (frequency % 100000)/10 );

   }
   else
   {
      // frequency mode
      sprintf(buffer, "%3u.%05u", frequency / 100000, frequency % 100000);
   }

   UI_PrintStringSmallNormal(buffer, 64, 0, 6);

   
   ST7565_BlitFullScreen();
}

#endif

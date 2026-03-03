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
#ifdef ENABLE_AM_FIX
   #include "am_fix.h"
#endif



static void Ui_DisplayARDF_Mod(VFO_Info_t *vfoInfo, uint8_t vfo, uint8_t line, bool bold)
{
   char buffer[16];

   ModulationMode_t mod = vfoInfo->Modulation;

   sprintf(buffer, "%c", 'A' + vfo );

   char *bw = NULL;
   switch ( vfoInfo->CHANNEL_BANDWIDTH )
   {
      case BANDWIDTH_NARROW:

         bw = "N";
         break;

      case BANDWIDTH_U2K5:

         bw = "N-";
         break;

      case BANDWIDTH_U1K7:

         bw = "U-";
         break;

      default:

         bw = "W";
         break;
   }

   if ( bold != false )
   {
      UI_PrintStringSmallBold(buffer, 0, 0, line);
      UI_PrintStringSmallBold(gModulationStr[mod], 12, 36, line); // modulation

      UI_PrintStringSmallBold(bw, 42, 58, line);
   }
   else
   {
      UI_PrintStringSmallNormal(buffer, 0, 0, line);
      UI_PrintStringSmallNormal(gModulationStr[mod], 12, 36, line); // modulation

      UI_PrintStringSmallNormal(bw, 42, 58, line);
   }
}



static uint16_t Ui_DisplayARDF_dBm2level(int16_t dBm)
{
    int16_t rssi_dBm =
        dBm
    //    + ( (gSetting_ARDFEnable != false) ? ARDF_Get_GainDiff() : 0 )
        + dBmCorrTable[gRxVfo->Band];

    rssi_dBm = -rssi_dBm;

    if(rssi_dBm > 141) rssi_dBm = 141;
    if(rssi_dBm < 53) rssi_dBm = 53;

    uint8_t s_level = 0;
    //uint8_t overS9dBm = 0;
    //uint8_t overS9Bars = 0;

    if(rssi_dBm >= 93)
    {
        s_level = map(rssi_dBm, 141, 93, 1, 9);
    }
    else
    {
        s_level = 9;
        //overS9dBm = map(rssi_dBm, 93, 53, 0, 40);
        //overS9Bars = map(overS9dBm, 0, 40, 0, 4);
    }

//#else
//    const int16_t s0_dBm   = -gEeprom.S0_LEVEL;                  // S0 .. base level
//
//    int s0_9 = gEeprom.S0_LEVEL - gEeprom.S9_LEVEL;
//    const uint8_t s_level = MIN(MAX((int32_t)(rssi_dBm - s0_dBm)*100 / (s0_9*100/9), 0), 9); // S0 - S9
//    //uint8_t overS9dBm = MIN(MAX(rssi_dBm + gEeprom.S9_LEVEL, 0), 99);
//    //uint8_t overS9Bars = MIN(overS9dBm/10, 4);
//#endif


   return MIN(s_level, 9);

}



void Ui_DisplayARDF_RSSIBar_Simple(void)
{
   const uint8_t line = 3;

   uint8_t level, level_max;
   uint8_t *p_line = gFrameBuffer[line];

   level = Ui_DisplayARDF_dBm2level( BK4819_GetRSSI_dBm() );
   level_max = Ui_DisplayARDF_dBm2level( (gARDFRssiMax / 2) - 160 );

   uint8_t barslice = 0xFF;

   for(uint8_t i = 0; i < 9; i++)
   {
      if ( i >= level )
      {
         barslice = 0x00;
      }

      for(uint8_t j = 0; j < 13; j++)
      {
         p_line[i * 14 + j] = barslice;
      }
   }

   p_line[(level_max-1) * 14 + 11] = 0xFF;
   p_line[(level_max-1) * 14 + 12] = 0xFF;

   ST7565_BlitLine(line);
}



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

   if ( gLowBattery && !gLowBatteryConfirmed && (gARDFDFSimpleMode==0) )
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
   uint8_t activefox = gARDFActiveFox;
   uint8_t line = (2 + 4*gARDFDFSimpleMode);

   if ( gLowBattery && !gLowBatteryConfirmed && (gARDFDFSimpleMode==0) )
      return;

   if ( ARDF_ActVfoHasGainRemember(vfo) == false )
   {
      // do not remember fox gains/mistuning on this vfo
      activefox = 0;
   }

   if ( ( IS_FREQ_CHANNEL(gEeprom.ScreenChannel[vfo]) && (ardf_mistune_active[vfo][activefox] == false) ) 
          || ( (gARDFMemModeFreqToggleCnt_s >= ARDF_MEM_MODE_FREQ_TOGGLE_S) && (gInputBoxIndex == 0) ) )
   {
      // frequency mode without mistuning
      // or (frequency is shown anyway (memory mode or mistuning mode) and no input)
      
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

      if ( gARDFDFSimpleMode==0 )
      {
         UI_PrintStringSmallBold(buffer, 64, 0, line);
      }
      else
      {
         UI_PrintStringSmallNormal(buffer, 64, 0, line);
      }

   }
   else if ( ardf_mistune_active[vfo][activefox] != false )
   {
      // frequency mistuning active
      if ( gInputBoxIndex == 0 )
      {
         sprintf(buffer, "%+d Hz", gARDFMistuneFreqRaw*ARDF_MISTUNE_RES_HZ);
      }
      else
      {
         if ( IS_FREQ_CHANNEL(gEeprom.ScreenChannel[vfo]) )
         {
            const char * ascii = INPUTBOX_GetAscii();
            sprintf(buffer, "%.3s.%.3s", ascii, ascii + 3);
         }
         else
         {
            sprintf(buffer, "M%.3s", INPUTBOX_GetAscii() );  // show the input text
         }
      }

      if ( gARDFDFSimpleMode==0 )
      {
         UI_PrintStringSmallBold(buffer, 64, 0, line);
      }
      else
      {
         UI_PrintStringSmallNormal(buffer, 64, 0, line);
      }

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
   
      if ( gARDFDFSimpleMode==0 )
      {
         UI_PrintStringSmallBold(buffer, 64, 0, line); // 0->128: text centered, but pixel deletion problem
      }
      else
      {
         UI_PrintStringSmallNormal(buffer, 64, 0, line); // 0->128: text centered, but pixel deletion problem
      }

   }

   ST7565_BlitLine(line);
}



void UI_DisplayARDF(void)
{
   char buffer[16];
   uint8_t vfo = gEeprom.RX_VFO;
   uint8_t xpos = 0;
   
   UI_DisplayClear();

   if ( gLowBattery && !gLowBatteryConfirmed && (gARDFDFSimpleMode==0) )
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

      xpos = 57;
   }

   if ( (ARDF_ActVfoHasGainRemember(vfo) != false) && ardf_mistune_active[vfo][gARDFActiveFox] != false )
   {
      // mistuning and gain remember active
      sprintf(buffer, "%d", ARDF_Get_GainIndex(vfo) - ardf_gain_index_steps_mistune[vfo][gARDFActiveFox] - 1 );
   }
   else if ( (ARDF_ActVfoHasGainRemember(vfo) == false) && ardf_mistune_active[vfo][0] != false )
   {
      // mistuning active and gain remember not active
      sprintf(buffer, "%d", ARDF_Get_GainIndex(vfo) - ardf_gain_index_steps_mistune[vfo][0] - 1 );
   }
   else
   {
      // show normal index
      sprintf(buffer, "%02d", ARDF_Get_GainIndex(vfo) );
   }

   UI_DisplayFrequency(buffer, xpos, 0, false);

   UI_DisplayARDF_RSSI();


   /* 2. small line: active vfo */
   Ui_DisplayARDF_Mod(&gEeprom.VfoInfo[vfo], vfo, (2 + 4*gARDFDFSimpleMode), (gARDFDFSimpleMode==0) );
   UI_DisplayARDF_FreqCh();

   /* 3. middle line for debug or rssi bar */

#ifdef ARDF_ENABLE_SHOW_DEBUG_DATA
   UI_DisplayARDF_Debug();
#elif defined(ENABLE_AGC_SHOW_DATA)
   UI_MAIN_PrintAGC(true);
#else
   center_line = CENTER_LINE_RSSI;

   if ( gARDFDFSimpleMode != false )
   {
      Ui_DisplayARDF_RSSIBar_Simple();
   }
   else if( !(gLowBattery && !gLowBatteryConfirmed) )
   {
      DisplayRSSIBar(true);
   }

#endif

   /* 4. gain index history. show max 5 foxes. */

   if ( (ARDF_ActVfoHasGainRemember(vfo) != false) && (gARDFDFSimpleMode == 0) )
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


         if ( ardf_mistune_active[vfo][idx] != false )
         {
            // mistuning active
            sprintf(buffer, "%d", ardf_gain_index[vfo][idx] - ardf_gain_index_steps_mistune[vfo][idx] - 1 );
         }
         else
         {
            // normal index
            sprintf(buffer, "%d", ardf_gain_index[vfo][idx] );
         }

         UI_PrintStringSmallBold(buffer, lineofs + i*16 + i*4, lineofs + (i+1)*16 + i*4, 5);

      }

   }


   /* 5. small line: inactive vfo (but not in DF simple) */
   if ( gARDFDFSimpleMode == 0 )
   {
      Ui_DisplayARDF_Mod(&gEeprom.VfoInfo[1-vfo], 1-vfo, 6, false);
   
      VFO_Info_t *vfoInfo = &gEeprom.VfoInfo[1-vfo]; // the inactive vfo
   uint32_t frequency = vfoInfo->freq_config_RX.Frequency;

   if ( IS_MR_CHANNEL(gEeprom.ScreenChannel[1-vfo]) ) // the inactive vfo
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
   }
   
   ST7565_BlitFullScreen();
}

#endif

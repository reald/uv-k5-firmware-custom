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



static void UI_DisplayARDF_Mod(VFO_Info_t *vfoInfo, uint8_t vfo, uint8_t line, bool bold)
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



static uint16_t UI_ARDF_dBm2level(int16_t rssi_dBm)
{
   uint8_t s_level = 0;

   if ( rssi_dBm < ARDF_S0_dBm ) // -141 dBm = S1 (IARU)
   {
      return 0;
   }

   if( rssi_dBm >= ARDF_S9_dBm ) // -93 dBm = S9
   {
      return 9;
   }

   s_level = map(rssi_dBm, ARDF_S0_dBm, ARDF_S9_dBm, 1, 9);

   return MIN(s_level, 9);

}



void UI_DisplayARDF_RSSIBar_Simple(uint8_t line)
{
   uint8_t level, level_max;
   uint8_t *p_line = gFrameBuffer[line];

   level = UI_ARDF_dBm2level(BK4819_GetRSSI_dBm());
   level_max = UI_ARDF_dBm2level((gARDFRssiMax / 2) - 160);

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

   if ( level_max > 0 )
   {
      p_line[(level_max-1) * 14 + 11] = 0xFF;
      p_line[(level_max-1) * 14 + 12] = 0xFF;
   }

   ST7565_BlitLine(line);
}



void UI_DisplayARDF_RSSIBar(bool updatenow)
{
   const uint8_t line = 3;
   const uint8_t xpos = 52;
   const uint8_t bars = 10;

   char buffer[16];

   uint8_t s_level = 0;
   uint8_t overS9dBm  = 0;
   uint8_t overS9Bars = 0;

   uint8_t *p_line = gFrameBuffer[line];

   // clear complete line
   memset(p_line, 0, LCD_WIDTH);

   int16_t rssi_dBm = BK4819_GetRSSI_dBm()
                      + ARDF_Get_GainDiff() // shows the signal strength before the attenuators/lnas, not attenuated amplitude
                      + dBmCorrTable[gRxVfo->Band];

   s_level = UI_ARDF_dBm2level(rssi_dBm);

   if ( rssi_dBm > ARDF_S9_dBm )
   {
      // Signal > S9 : compute over-S9
      overS9dBm  = map(rssi_dBm, ARDF_S9_dBm, ARDF_S9_dBm + 40, 0, 40);
      overS9Bars = map(overS9dBm, 0, 40, 0, 4);
   }

   if ( overS9Bars == 0 )
   {
      sprintf(buffer, "%4d S%d", rssi_dBm, s_level);
   }
   else
   {
      sprintf(buffer, "%3d +%02d", rssi_dBm, overS9dBm);
   }

   UI_PrintStringSmallNormal(buffer, 0, 0, line);


   uint8_t level = MIN(s_level + overS9Bars, bars);
   uint8_t add_space = 0;

   for ( uint8_t i = 0; i < level; i++ ) // start at S1
   {
      add_space = (i >= 5) ? 1 : 0;

      const char hollowBar[] =
      {
         0b01111111,
         0b01000001,
         0b01000001,
         0b01111111
      };

      if ( i < (bars-1) )
      {
         for ( uint8_t j = 0; j < 4; j++ )
         {
            p_line[xpos + i * 5 + j + add_space ] = 0x7f;
         }
      }
      else
      {
         memcpy(p_line + (xpos + i * 5 + add_space), &hollowBar, ARRAY_SIZE(hollowBar));
      }
   }

   sprintf(buffer, "%3d", gRssi0Max);
   UI_PrintStringSmallNormal(buffer, 103, 127, line);


   if ( updatenow != false )
   {
      ST7565_BlitLine(line);
   }
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



void UI_DisplayARDF_Distance(bool updatenow)
{
   const uint8_t start_col = 89;
   char buffer[8];
   uint8_t *p_line = gFrameBuffer[3];
   int16_t distance_idx = gARDFDistanceIdx;

   if ( gLowBattery && !gLowBatteryConfirmed && (gARDFDFSimpleMode==0) )
      return;

   if ( gARDFDFSimpleMode == 0 )
   {
      // only in ARDF mode, not in DF simple

      // erase screenbuffer because following string has variable length and does not overwrite everything
      p_line = gFrameBuffer[4];
      memset(&p_line[start_col], 0x00, LCD_WIDTH - start_col);

      p_line = gFrameBuffer[5];
      memset(&p_line[start_col], 0x00, LCD_WIDTH - start_col);

      if ( (gARDFRssi0At100m != 0) && (0 <= distance_idx) )
      {
         // meters only if active and stable value available
         sprintf(buffer, "%s", ardf_rssi2distance[distance_idx]);

         if ( strlen(buffer) == 3 )
         {
            UI_PrintString(buffer, start_col+3, 127, 4, 10); // 7 pixel font, 10 pixel distance by padding 3 empty columns behind character
      }
         else if ( strlen(buffer) == 4 )
         {
            UI_PrintString(buffer, start_col+3, 127, 4, 8); // 7 pixel font, 8 pixel distance by padding 1 empty columns behind character
   }

      }
   }

   if ( updatenow != false )
   {
      ST7565_BlitLine(4);
      ST7565_BlitLine(5);
   }

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
   const uint8_t start_col = 64;
   char buffer[16];
   uint8_t vfo = gEeprom.RX_VFO;
   uint8_t line = (2 + 4*gARDFDFSimpleMode);
   uint8_t *p_line = gFrameBuffer[line];
   t_ardf_gain_cheat_type activeGainCheatType = ARDF_ActiveGainCheatType(vfo);

   if ( gLowBattery && !gLowBatteryConfirmed && (gARDFDFSimpleMode==0) )
      return;

   // clear half line with frequency
   memset(p_line + start_col, 0, LCD_WIDTH - start_col);

   if ( ( IS_FREQ_CHANNEL(gEeprom.ScreenChannel[vfo]) && (activeGainCheatType == ARDF_NO_GAIN_CHEAT) )
          || ( (gARDFMemModeFreqToggleCnt_s >= ARDF_MEM_MODE_FREQ_TOGGLE_S) && (gInputBoxIndex == 0) ) )
   {
      // frequency mode without gain cheat
      // or (frequency is shown anyway (memory mode or gain cheat mode) and no input)

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
         UI_PrintStringSmallBold(buffer, start_col, 0, line);
      }
      else
      {
         UI_PrintStringSmallNormal(buffer, start_col, 0, line);
      }

   }
   else if ( activeGainCheatType != ARDF_NO_GAIN_CHEAT )
   {
      // gain cheat active

      if ( (gInputBoxIndex == 0) && (activeGainCheatType == ARDF_INT_LNA_OFF) )
      {
         sprintf(buffer, "LNA OFF" );
      }
      else if ( (gInputBoxIndex == 0) && (activeGainCheatType == ARDF_HARMONIC_2) )
      {
         sprintf(buffer, "2. HARM" );
      }
      else if ( (gInputBoxIndex == 0) && (activeGainCheatType == ARDF_HARMONIC_3) )
      {
         sprintf(buffer, "3. HARM" );
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
         UI_PrintStringSmallBold(buffer, start_col, 0, line);
      }
      else
      {
         UI_PrintStringSmallNormal(buffer, start_col, 0, line);
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
         UI_PrintStringSmallBold(buffer, start_col, 0, line);
      }
      else
      {
         UI_PrintStringSmallNormal(buffer, start_col, 0, line);
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

   if ( (ARDF_ActVfoHasGainRemember(vfo) != false) && (ARDF_ActiveGainCheatType(vfo) != ARDF_NO_GAIN_CHEAT) )
   {
      // gain cheat and gain remember active
      sprintf(buffer, "%d", -(int8_t)ardf_type_gain_cheat[vfo][gARDFActiveFox] );
   }
   else if ( (ARDF_ActVfoHasGainRemember(vfo) == false) && (ARDF_ActiveGainCheatType(vfo) != ARDF_NO_GAIN_CHEAT) )
   {
      // gain cheat active and gain remember not active
      sprintf(buffer, "%d", -(int8_t)ardf_type_gain_cheat[vfo][0] );
   }
   else
   {
      // show normal index
      sprintf(buffer, "%02d", ARDF_Get_GainIndex(vfo) );
   }

   UI_DisplayFrequency(buffer, xpos, 0, false);

   UI_DisplayARDF_RSSI();


   /* 2. small line: active vfo */

   UI_DisplayARDF_Mod(&gEeprom.VfoInfo[vfo], vfo, (2 + 4*gARDFDFSimpleMode), (gARDFDFSimpleMode==0) );
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
      UI_DisplayARDF_RSSIBar_Simple(3);
   }
   else if( !(gLowBattery && !gLowBatteryConfirmed) )
   {
      UI_DisplayARDF_RSSIBar(false);
   }

#endif

   /* 4a. gain index history. show max 5 foxes. */

   if ( (ARDF_ActVfoHasGainRemember(vfo) != false) && (gARDFDFSimpleMode == false) )
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

         const int xofs = 4;
         const int coldist = 4;
         const int charwidth = 7; // 6 + 1 space

         if ( idx == gARDFActiveFox )
         {
            UI_PrintStringSmallBold(buffer, xofs + i*(2*charwidth) + i*coldist, xofs + (i+1)*(2*charwidth) + i*coldist, 4);
         }
         else
         {
            UI_PrintStringSmallNormal(buffer, xofs + i*(2*charwidth) + i*coldist, xofs + (i+1)*(2*charwidth) + i*coldist, 4);
         }


         if ( ardf_type_gain_cheat[vfo][idx] != ARDF_NO_GAIN_CHEAT )
         {
            // gain cheat active
            sprintf(buffer, "%d", -(int8_t)ardf_type_gain_cheat[vfo][idx] );
         }
         else
         {
            // normal index
            sprintf(buffer, "%d", ardf_gain_index[vfo][idx] );
         }
         UI_PrintStringSmallBold(buffer, xofs + i*(2*charwidth) + i*coldist, xofs + (i+1)*(2*charwidth) + i*coldist, 5);

      }

   }

   /* 4b. distance prediction */

   UI_DisplayARDF_Distance(false);

   /* 5. small line: inactive vfo (but not in DF simple) */

   if ( gARDFDFSimpleMode == 0 )
   {
      uint32_t frequency = 0;

      UI_DisplayARDF_Mod(&gEeprom.VfoInfo[1-vfo], 1-vfo, 6, false);

      VFO_Info_t *vfoInfo = &gEeprom.VfoInfo[1-vfo]; // the inactive vfo

      if ( ARDF_ActiveGainCheatType(1-vfo) == ARDF_NO_GAIN_CHEAT )
      {
         frequency = vfoInfo->freq_config_RX.Frequency;
      }
      else
      {
         // gain cheat active but show base frequency
         frequency = gARDFGainCheatBaseFrequency[1-vfo]/10;
      }

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

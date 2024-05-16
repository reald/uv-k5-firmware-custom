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

#ifndef UI_ARDF_H
#define UI_ARDF_H

#ifdef ENABLE_ARDF
void UI_DisplayARDF_Timer(void);
void UI_DisplayARDF_RSSI(void);
void UI_DisplayARDF_Debug(void);
void UI_DisplayARDF(void);
#endif

#endif

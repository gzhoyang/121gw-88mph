/*****************************************************************************
 *  Copyright 2018 Thomas Watson                                             *
 *                                                                           *
 *  This file is a part of 88mph: https://github.com/tpwrules/121gw-88mph/   *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *****************************************************************************/

#include <stdint.h>

#include "hardware/lcd_tables.h"

#include "hardware/lcd_segments.h"

// table to map letters and numbers to segments
// each byte is in xGFEDCBA order
const uint8_t lcd_7seg_font[36] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, // 0-5
    0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, // 6-9, A-B
    0x39, 0x5e, 0x79, 0x71, 0x6f, 0x76, // C-H
    0x30, 0x0e, 0x75, 0x38, 0x55, 0x54, // I-N
    0x5c, 0x73, 0x67, 0x50, 0x6d, 0x78, // O-T
    0x3e, 0x1c, 0x1d, 0x64, 0x6e, 0x1b  // U-Z
};

// table to map 7 segment segments to the screen
// 0-4: sub screen, left to right
// 5-9: main screen, left to right
const uint8_t lcd_7seg_segments[10][7] = {
    {SEG_SS_10000_A, SEG_SS_10000_B, SEG_SS_10000_C, SEG_SS_10000_D,
        SEG_SS_10000_E, SEG_SS_10000_F, SEG_SS_10000_G},
    {SEG_SS_1000_A, SEG_SS_1000_B, SEG_SS_1000_C, SEG_SS_1000_D,
        SEG_SS_1000_E, SEG_SS_1000_F, SEG_SS_1000_G},
    {SEG_SS_100_A, SEG_SS_100_B, SEG_SS_100_C, SEG_SS_100_D,
        SEG_SS_100_E, SEG_SS_100_F, SEG_SS_100_G},
    {SEG_SS_10_A, SEG_SS_10_B, SEG_SS_10_C, SEG_SS_10_D,
        SEG_SS_10_E, SEG_SS_10_F, SEG_SS_10_G},
    {SEG_SS_1_A, SEG_SS_1_B, SEG_SS_1_C, SEG_SS_1_D,
        SEG_SS_1_E, SEG_SS_1_F, SEG_SS_1_G},

    {SEG_MS_10000_A, SEG_MS_10000_B, SEG_MS_10000_C, SEG_MS_10000_D,
        SEG_MS_10000_E, SEG_MS_10000_F, SEG_MS_10000_G},
    {SEG_MS_1000_A, SEG_MS_1000_B, SEG_MS_1000_C, SEG_MS_1000_D,
        SEG_MS_1000_E, SEG_MS_1000_F, SEG_MS_1000_G},
    {SEG_MS_100_A, SEG_MS_100_B, SEG_MS_100_C, SEG_MS_100_D,
        SEG_MS_100_E, SEG_MS_100_F, SEG_MS_100_G},
    {SEG_MS_10_A, SEG_MS_10_B, SEG_MS_10_C, SEG_MS_10_D,
        SEG_MS_10_E, SEG_MS_10_F, SEG_MS_10_G},
    {SEG_MS_1_A, SEG_MS_1_B, SEG_MS_1_C, SEG_MS_1_D,
        SEG_MS_1_E, SEG_MS_1_F, SEG_MS_1_G}
};

// table to map reading units to lcd segments
// 0 = subscreen, 1 = main screen
// order follows that defined in reading.h
const uint8_t lcd_unit_icons[2][11] = {
    // on subscreen
    {
        SEG_NONE, SEG_SS_AMPS, SEG_NONE, SEG_NONE,
        SEG_SS_HERTZ, SEG_SS_nS, SEG_SS_OHMS,
        SEG_SS_VOLTS, SEG_NONE, SEG_NONE,
        SEG_SS_dB
    },
    // and main screen
    {
        SEG_NONE, SEG_MS_AMPS, SEG_MS_DUTY_PERCENT, SEG_MS_FARADS,
        SEG_MS_HERTZ, SEG_MS_mSEC, SEG_MS_OHMS,
        SEG_MS_VOLTS, SEG_MS_DEG_C, SEG_MS_DEG_F,
        SEG_NONE
    }
};

// table to map exponents to their icons
// starts at nano, ends at mega
// 0 = subscreen, 1 = main screen
const uint8_t lcd_exponent_icons[2][6] = {
    // subscreen
    {SEG_NONE, SEG_NONE, SEG_SS_MILLI, SEG_NONE, SEG_SS_KILO, SEG_NONE},
    // main screen
    {SEG_MS_NANO, SEG_MS_MICRO, SEG_MS_MILLI,
        SEG_NONE, SEG_MS_KILO, SEG_MS_MEGA}
};

// table to map exponents to decimal points
// 0 = subscreen, 1 = main screen
const uint8_t lcd_decimal_points[2][5] = {
    // subscreen
    {SEG_SS_POINT_d0000, SEG_SS_POINT_d000, 
        SEG_SS_POINT_d00, SEG_SS_POINT_d0, SEG_NONE},
    // and main screen
    {SEG_MS_POINT_d0000, SEG_MS_POINT_d000,
        SEG_MS_POINT_d00, SEG_MS_POINT_d0, SEG_NONE}
};
/*******************************************************************************
 * Size: 13 px
 * Bpp: 1
 * Opts: --size 13 --bpp 1 --format lvgl --font E:\develop\hardware\weixue\tools\font\NotoSansCJKsc-Regular.otf -r 0x20-0x7E --symbols 上一页长按首页下一页操作双击刷新数据正在实时更新请求已发送设备设置网页配置开关行情新闻返回开启关闭沪深创 --no-kerning --lv-font-name lv_font_noto_sans_sc_13 -o C:\Users\yun\Documents\Codex\2026-08-17\https-docs-waveshare-net-esp32-s3\work\home-stock-layout\lv_font_noto_sans_sc_13.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_FONT_NOTO_SANS_SC_13
#define LV_FONT_NOTO_SANS_SC_13 1
#endif

#if LV_FONT_NOTO_SANS_SC_13

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfc, 0x80,

    /* U+0022 "\"" */
    0x99, 0x90,

    /* U+0023 "#" */
    0x49, 0x2f, 0xd4, 0x53, 0xf9, 0x24, 0x90,

    /* U+0024 "$" */
    0x23, 0xa3, 0x8, 0x30, 0xc1, 0xc, 0x5c, 0x40,

    /* U+0025 "%" */
    0x61, 0x12, 0x22, 0x48, 0x4a, 0x66, 0x52, 0x12,
    0x42, 0x48, 0x89, 0x20, 0xc0,

    /* U+0026 "&" */
    0x38, 0x48, 0x48, 0x50, 0x63, 0x92, 0x8e, 0x8e,
    0x7b,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x49, 0x49, 0x24, 0x91, 0x20,

    /* U+0029 ")" */
    0x49, 0x12, 0x49, 0x29, 0x20,

    /* U+002A "*" */
    0x4b, 0x65,

    /* U+002B "+" */
    0x10, 0x23, 0xf8, 0x81, 0x2, 0x0,

    /* U+002C "," */
    0xd8,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x8, 0x84, 0x22, 0x10, 0x88, 0x42, 0x31, 0x0,

    /* U+0030 "0" */
    0x79, 0x28, 0x61, 0x86, 0x18, 0x52, 0x78,

    /* U+0031 "1" */
    0x61, 0x8, 0x42, 0x10, 0x84, 0xf8,

    /* U+0032 "2" */
    0x72, 0x20, 0x82, 0x18, 0x42, 0x10, 0xfc,

    /* U+0033 "3" */
    0x39, 0x10, 0x43, 0x38, 0x30, 0x51, 0x78,

    /* U+0034 "4" */
    0x18, 0x62, 0x92, 0xcb, 0xf0, 0x82, 0x8,

    /* U+0035 "5" */
    0x7d, 0x4, 0x1e, 0xc, 0x10, 0x63, 0x78,

    /* U+0036 "6" */
    0x39, 0x8, 0x2e, 0xce, 0x18, 0x53, 0x38,

    /* U+0037 "7" */
    0xfc, 0x21, 0x84, 0x10, 0x82, 0x8, 0x20,

    /* U+0038 "8" */
    0x7a, 0x38, 0x61, 0x7a, 0x38, 0x71, 0x78,

    /* U+0039 "9" */
    0x72, 0x28, 0x63, 0x74, 0x10, 0x42, 0x70,

    /* U+003A ":" */
    0xc2,

    /* U+003B ";" */
    0xf0, 0xd, 0x80,

    /* U+003C "<" */
    0x0, 0x76, 0x30, 0x30, 0x30,

    /* U+003D "=" */
    0xfe, 0x0, 0x7, 0xf0,

    /* U+003E ">" */
    0x3, 0x81, 0x83, 0x33, 0x0,

    /* U+003F "?" */
    0xe9, 0x11, 0x24, 0x40, 0x40,

    /* U+0040 "@" */
    0xf, 0x6, 0x19, 0x81, 0x27, 0x98, 0x93, 0x24,
    0x64, 0x8c, 0x92, 0x8d, 0x88, 0x1, 0x84, 0xf,
    0x0,

    /* U+0041 "A" */
    0x18, 0x18, 0x2c, 0x24, 0x24, 0x7e, 0x42, 0x42,
    0x83,

    /* U+0042 "B" */
    0xf9, 0xa, 0x14, 0x6f, 0x90, 0xe0, 0xc3, 0xfc,

    /* U+0043 "C" */
    0x3c, 0x82, 0x4, 0x8, 0x10, 0x20, 0x21, 0x3c,

    /* U+0044 "D" */
    0xf9, 0xa, 0xc, 0x18, 0x30, 0x61, 0xc6, 0xf8,

    /* U+0045 "E" */
    0xfc, 0x21, 0xf, 0xc2, 0x10, 0xf8,

    /* U+0046 "F" */
    0xfc, 0x21, 0xf, 0xc2, 0x10, 0x80,

    /* U+0047 "G" */
    0x3c, 0x86, 0x4, 0x8, 0xf0, 0x60, 0xa1, 0x3c,

    /* U+0048 "H" */
    0x83, 0x6, 0xc, 0x1f, 0xf0, 0x60, 0xc1, 0x82,

    /* U+0049 "I" */
    0xff, 0x80,

    /* U+004A "J" */
    0x8, 0x42, 0x10, 0x84, 0x31, 0x70,

    /* U+004B "K" */
    0x85, 0x12, 0x45, 0xd, 0x1b, 0x22, 0x42, 0x86,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x42, 0x10, 0xf8,

    /* U+004D "M" */
    0xc3, 0xc3, 0xc3, 0xa5, 0xa5, 0x99, 0x99, 0x99,
    0x81,

    /* U+004E "N" */
    0x83, 0x86, 0x8d, 0x19, 0x31, 0x62, 0xc3, 0x82,

    /* U+004F "O" */
    0x3c, 0x42, 0x81, 0x81, 0x81, 0x81, 0x81, 0x42,
    0x3c,

    /* U+0050 "P" */
    0xfa, 0x18, 0x63, 0xfa, 0x8, 0x20, 0x80,

    /* U+0051 "Q" */
    0x3c, 0x42, 0xc3, 0x81, 0x81, 0x81, 0x81, 0x42,
    0x66, 0x1c, 0x8, 0x7,

    /* U+0052 "R" */
    0xfa, 0x18, 0x61, 0xfa, 0x48, 0xa2, 0x84,

    /* U+0053 "S" */
    0x7a, 0x8, 0x30, 0x38, 0x30, 0x61, 0x78,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,

    /* U+0055 "U" */
    0x83, 0x6, 0xc, 0x18, 0x30, 0x60, 0xe2, 0x7c,

    /* U+0056 "V" */
    0x82, 0x85, 0x1a, 0x22, 0x44, 0x8a, 0xc, 0x18,

    /* U+0057 "W" */
    0xc4, 0x28, 0xc5, 0x29, 0xa5, 0x26, 0xb4, 0x52,
    0x8c, 0x51, 0x8c, 0x31, 0x80,

    /* U+0058 "X" */
    0x46, 0xc8, 0xb1, 0xc1, 0x85, 0xb, 0x22, 0x42,

    /* U+0059 "Y" */
    0xc6, 0x89, 0xb1, 0x42, 0x82, 0x4, 0x8, 0x10,

    /* U+005A "Z" */
    0xfe, 0x8, 0x30, 0x41, 0x4, 0x18, 0x20, 0xfe,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x4e,

    /* U+005C "\\" */
    0x82, 0x10, 0x82, 0x10, 0x84, 0x10, 0x84, 0x10,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0x9e,

    /* U+005E "^" */
    0x21, 0x94, 0xa8, 0xc4,

    /* U+005F "_" */
    0xfe,

    /* U+0060 "`" */
    0x88, 0x80,

    /* U+0061 "a" */
    0x70, 0x42, 0xf8, 0xc5, 0xe0,

    /* U+0062 "b" */
    0x82, 0x8, 0x2e, 0xce, 0x18, 0x61, 0x8b, 0xe0,

    /* U+0063 "c" */
    0x7e, 0x21, 0x8, 0x61, 0xe0,

    /* U+0064 "d" */
    0x4, 0x10, 0x5f, 0xc6, 0x18, 0x61, 0xc5, 0xf0,

    /* U+0065 "e" */
    0x79, 0x38, 0x7f, 0x83, 0x3, 0x80,

    /* U+0066 "f" */
    0x34, 0x4e, 0x44, 0x44, 0x44,

    /* U+0067 "g" */
    0x7e, 0x28, 0xa2, 0x72, 0x7, 0xe1, 0x85, 0xe0,

    /* U+0068 "h" */
    0x82, 0x8, 0x2e, 0xc6, 0x18, 0x61, 0x86, 0x10,

    /* U+0069 "i" */
    0x9f, 0xc0,

    /* U+006A "j" */
    0x20, 0x12, 0x49, 0x24, 0x9c,

    /* U+006B "k" */
    0x82, 0x8, 0x22, 0x92, 0x8e, 0x34, 0x9a, 0x20,

    /* U+006C "l" */
    0xaa, 0xaa, 0xb0,

    /* U+006D "m" */
    0xb3, 0x66, 0x62, 0x31, 0x18, 0x8c, 0x46, 0x22,

    /* U+006E "n" */
    0xbb, 0x18, 0x61, 0x86, 0x18, 0x40,

    /* U+006F "o" */
    0x7b, 0x38, 0x61, 0x87, 0x37, 0x80,

    /* U+0070 "p" */
    0xbb, 0x38, 0x61, 0x86, 0x2f, 0xa0, 0x82, 0x0,

    /* U+0071 "q" */
    0x7f, 0x18, 0x61, 0x87, 0x17, 0xc1, 0x4, 0x10,

    /* U+0072 "r" */
    0xbc, 0x88, 0x88, 0x80,

    /* U+0073 "s" */
    0x74, 0x20, 0xe0, 0xc7, 0xc0,

    /* U+0074 "t" */
    0x44, 0xf4, 0x44, 0x44, 0x70,

    /* U+0075 "u" */
    0x86, 0x18, 0x61, 0x86, 0x37, 0x40,

    /* U+0076 "v" */
    0xc4, 0x89, 0x13, 0x42, 0x85, 0x4, 0x0,

    /* U+0077 "w" */
    0xc4, 0x53, 0x14, 0xa9, 0x2a, 0x32, 0x8c, 0xa3,
    0x10,

    /* U+0078 "x" */
    0x45, 0xa3, 0x8c, 0x39, 0x24, 0x40,

    /* U+0079 "y" */
    0xc4, 0x89, 0x11, 0x42, 0x85, 0x4, 0x8, 0x30,
    0xc0,

    /* U+007A "z" */
    0xf8, 0x84, 0x44, 0x23, 0xe0,

    /* U+007B "{" */
    0x69, 0x24, 0xa2, 0x49, 0x26,

    /* U+007C "|" */
    0xff, 0xfc,

    /* U+007D "}" */
    0xc9, 0x24, 0x8a, 0x49, 0x2c,

    /* U+007E "~" */
    0xe0, 0xe0,

    /* U+4E00 "一" */
    0xff, 0xf0,

    /* U+4E0A "上" */
    0x4, 0x0, 0x40, 0x4, 0x0, 0x40, 0x7, 0xe0,
    0x40, 0x4, 0x0, 0x40, 0x4, 0x0, 0x40, 0xff,
    0xf0,

    /* U+4E0B "下" */
    0xff, 0xf0, 0x40, 0x4, 0x0, 0x60, 0x5, 0x80,
    0x46, 0x4, 0x20, 0x40, 0x4, 0x0, 0x40, 0x4,
    0x0,

    /* U+4F5C "作" */
    0x12, 0x1, 0x20, 0x27, 0xf2, 0x50, 0x69, 0xa,
    0x9f, 0x21, 0x2, 0x10, 0x21, 0xf2, 0x10, 0x21,
    0x2, 0x10,

    /* U+5173 "关" */
    0x20, 0x86, 0x10, 0x44, 0x3f, 0xe0, 0x40, 0x8,
    0x3f, 0xf8, 0x30, 0xa, 0x3, 0x21, 0x83, 0x60,
    0x10,

    /* U+51FB "击" */
    0x4, 0x0, 0x81, 0xff, 0x2, 0x0, 0x41, 0xff,
    0xc1, 0x2, 0x22, 0x44, 0x48, 0x89, 0xff, 0x0,
    0x20,

    /* U+521B "创" */
    0x8, 0x8, 0x62, 0x45, 0x92, 0x64, 0x96, 0x14,
    0xbf, 0x24, 0x89, 0x24, 0x49, 0x26, 0x49, 0x8,
    0x48, 0xc2, 0x7c, 0xe0,

    /* U+5237 "刷" */
    0x0, 0x17, 0xe1, 0x42, 0x94, 0x29, 0x7e, 0x94,
    0x89, 0x7e, 0x96, 0xa9, 0x6a, 0x96, 0xa1, 0xae,
    0x18, 0x87,

    /* U+53CC "双" */
    0x7f, 0xf0, 0x28, 0x91, 0x44, 0xd2, 0x42, 0x92,
    0xc, 0x50, 0x63, 0x3, 0x18, 0x25, 0xc3, 0x19,
    0x10, 0x84,

    /* U+53D1 "发" */
    0x12, 0x40, 0x93, 0x9, 0x88, 0x7f, 0xe0, 0x40,
    0x7, 0xf0, 0x31, 0x2, 0xc8, 0x32, 0x83, 0x1c,
    0x3, 0x98, 0x30, 0x20,

    /* U+542F "启" */
    0x4, 0x1, 0x7, 0xfd, 0x1, 0x40, 0x5f, 0xf4,
    0x1, 0x7f, 0x50, 0x74, 0x19, 0xfe, 0x41,

    /* U+56DE "回" */
    0xff, 0xf0, 0x6, 0x0, 0xcf, 0x99, 0x13, 0x22,
    0x67, 0xcc, 0x1, 0x80, 0x3f, 0xfe, 0x0, 0x80,

    /* U+5728 "在" */
    0x8, 0x1, 0x3, 0xff, 0x88, 0x2, 0x10, 0xc2,
    0x1b, 0xfd, 0x8, 0x21, 0x4, 0x20, 0x84, 0x1f,
    0xf0,

    /* U+5907 "备" */
    0xc, 0x0, 0xff, 0xe, 0x30, 0x9b, 0x0, 0xf8,
    0x78, 0x1c, 0xff, 0x84, 0x44, 0x3f, 0xe1, 0x11,
    0x8, 0x88, 0x7f, 0xc0,

    /* U+5B9E "实" */
    0x4, 0x0, 0x83, 0xff, 0xc2, 0x19, 0x42, 0x8,
    0x19, 0x0, 0x20, 0xff, 0xe1, 0x40, 0x66, 0x30,
    0x30,

    /* U+5DF2 "已" */
    0xff, 0xc0, 0x4, 0x40, 0x44, 0x4, 0x7f, 0xc4,
    0x4, 0x40, 0x4, 0x0, 0x40, 0x14, 0x2, 0x3f,
    0xe0,

    /* U+5F00 "开" */
    0xff, 0xf1, 0x8, 0x10, 0x81, 0x8, 0x10, 0x8f,
    0xff, 0x10, 0x81, 0x8, 0x20, 0x86, 0x8, 0xc0,
    0x80,

    /* U+60C5 "情" */
    0x21, 0x2, 0xff, 0x31, 0x7, 0xff, 0x71, 0xa,
    0xef, 0x27, 0xe2, 0x42, 0x27, 0xe2, 0x7e, 0x24,
    0x22, 0x46,

    /* U+6309 "按" */
    0x21, 0x2, 0x8, 0x2f, 0xff, 0x81, 0x21, 0x2,
    0xff, 0x32, 0x6e, 0x44, 0x22, 0x42, 0x18, 0x23,
    0x6e, 0xc1,

    /* U+636E "据" */
    0x20, 0x2, 0x7f, 0x24, 0x1f, 0x7f, 0x24, 0x82,
    0x7e, 0x74, 0x8e, 0x48, 0x2b, 0xf2, 0xa1, 0x2a,
    0x1d, 0x3f,

    /* U+64CD "操" */
    0x20, 0x2, 0x3c, 0x24, 0x47, 0x3c, 0x2f, 0xf2,
    0x99, 0x3f, 0xf6, 0x10, 0x2f, 0xf2, 0x3c, 0x25,
    0x26, 0x91,

    /* U+6570 "数" */
    0x21, 0x6, 0x90, 0x69, 0xf, 0xfe, 0x6f, 0x4a,
    0x74, 0x21, 0x4f, 0x8c, 0x48, 0x86, 0x8c, 0x39,
    0x4c, 0x63,

    /* U+65B0 "新" */
    0x10, 0xff, 0xe1, 0x28, 0x25, 0xf, 0xfe, 0x24,
    0x84, 0x97, 0xf2, 0x52, 0x4a, 0xca, 0x49, 0x1a,
    0x20,

    /* U+65F6 "时" */
    0x0, 0x4f, 0x4, 0x90, 0x49, 0x7f, 0x90, 0x4f,
    0x44, 0x92, 0x49, 0x14, 0x90, 0x4f, 0x4, 0x80,
    0x40, 0x3c,

    /* U+66F4 "更" */
    0xff, 0xe0, 0x81, 0xff, 0x22, 0x27, 0xfc, 0x88,
    0x9f, 0xf1, 0x20, 0x38, 0x7, 0x83, 0xf, 0x80,

    /* U+6B63 "正" */
    0x7f, 0xf0, 0x20, 0x2, 0x2, 0x20, 0x23, 0xe2,
    0x20, 0x22, 0x2, 0x20, 0x22, 0xf, 0xff,

    /* U+6C42 "求" */
    0x5, 0x80, 0x8b, 0xff, 0x82, 0x4, 0x44, 0x4d,
    0x1, 0xc0, 0x68, 0x34, 0x98, 0x8a, 0x10, 0x8e,
    0x0,

    /* U+6CAA "沪" */
    0xc3, 0x4, 0x20, 0x3f, 0x84, 0x1c, 0x82, 0x90,
    0x43, 0xf9, 0x40, 0x48, 0xb, 0x2, 0x40, 0x0,
    0x0,

    /* U+6DF1 "深" */
    0x40, 0x6, 0xff, 0x8, 0x10, 0x24, 0xc4, 0x22,
    0x90, 0xf, 0xf2, 0x18, 0x23, 0x44, 0x52, 0x59,
    0x10, 0x10,

    /* U+7F51 "网" */
    0xff, 0xf0, 0x6, 0x22, 0xd5, 0x5b, 0x33, 0x22,
    0x66, 0x6d, 0x15, 0xe4, 0x30, 0x6, 0x3, 0x80,

    /* U+7F6E "置" */
    0x7f, 0xe4, 0x92, 0x36, 0xcf, 0xff, 0x4, 0x3,
    0xbc, 0x3f, 0xc3, 0xfc, 0x3f, 0xc2, 0x4, 0xff,
    0xf0,

    /* U+884C "行" */
    0x10, 0x2, 0x3f, 0xc0, 0x1, 0x0, 0x27, 0xf6,
    0x4, 0xa0, 0x42, 0x4, 0x20, 0x42, 0x4, 0x20,
    0x42, 0x1c,

    /* U+8BBE "设" */
    0x63, 0xc1, 0x24, 0x2, 0x4e, 0x47, 0x20, 0x2,
    0x7e, 0x24, 0x22, 0x24, 0x29, 0x83, 0x18, 0x6e,
    0x70,

    /* U+8BF7 "请" */
    0x41, 0x6, 0xff, 0x21, 0x0, 0x7e, 0x1, 0x6,
    0xef, 0x27, 0xe2, 0x42, 0x27, 0xe3, 0x7e, 0x24,
    0x20, 0x4e,

    /* U+8FD4 "返" */
    0x0, 0x22, 0x3f, 0x89, 0x0, 0x8, 0x0, 0x7e,
    0x72, 0x10, 0x97, 0x5, 0x98, 0x29, 0xe1, 0x51,
    0x9c, 0x1, 0x1f, 0xe0,

    /* U+9001 "送" */
    0x44, 0x22, 0x24, 0xf, 0xf0, 0x10, 0xe1, 0x2,
    0xff, 0x21, 0x82, 0x1c, 0x22, 0x62, 0xc2, 0x50,
    0x8, 0xff,

    /* U+914D "配" */
    0xfd, 0xe6, 0x7, 0xf0, 0xda, 0x1b, 0x5f, 0xac,
    0x63, 0x87, 0xf0, 0x86, 0x3f, 0xc6, 0x17, 0x80,

    /* U+957F "长" */
    0x10, 0x1, 0xc, 0x11, 0x81, 0x60, 0x10, 0xf,
    0xff, 0x13, 0x1, 0x10, 0x11, 0x81, 0xc, 0x1e,
    0x33, 0x0,

    /* U+95ED "闭" */
    0x0, 0x9, 0xfc, 0x0, 0xc1, 0x1b, 0xfb, 0xc,
    0x61, 0x8c, 0x51, 0xb2, 0x30, 0x46, 0x38, 0xc0,
    0x70,

    /* U+95FB "闻" */
    0x0, 0x9, 0xfc, 0x0, 0xdf, 0xd9, 0x13, 0x3e,
    0x64, 0x4c, 0xf9, 0x91, 0x37, 0xf6, 0x4, 0xc0,
    0x30,

    /* U+9875 "页" */
    0xff, 0xe0, 0x81, 0xff, 0x20, 0x24, 0x44, 0x88,
    0x91, 0x12, 0x22, 0xb, 0x6, 0x1b, 0x0, 0x80,

    /* U+9996 "首" */
    0x30, 0xc1, 0x8, 0xff, 0xf0, 0x40, 0x3f, 0xc2,
    0x4, 0x3f, 0xc2, 0x4, 0x3f, 0xc2, 0x4, 0x3f,
    0xc2, 0x4
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 47, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 67, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 99, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 5, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 12, .adv_w = 115, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 20, .adv_w = 192, .box_w = 11, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 33, .adv_w = 141, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 42, .adv_w = 58, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 43, .adv_w = 70, .box_w = 3, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 48, .adv_w = 70, .box_w = 3, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 53, .adv_w = 97, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 55, .adv_w = 115, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 61, .adv_w = 58, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 62, .adv_w = 72, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 63, .adv_w = 58, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 64, .adv_w = 82, .box_w = 5, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 72, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 79, .adv_w = 115, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 85, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 92, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 99, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 106, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 113, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 120, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 127, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 134, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 141, .adv_w = 58, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 58, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 145, .adv_w = 115, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 150, .adv_w = 115, .box_w = 7, .box_h = 4, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 154, .adv_w = 115, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 159, .adv_w = 99, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 164, .adv_w = 197, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 181, .adv_w = 126, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 190, .adv_w = 137, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 133, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 143, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 214, .adv_w = 123, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 220, .adv_w = 115, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 226, .adv_w = 143, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 234, .adv_w = 151, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 242, .adv_w = 61, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 244, .adv_w = 111, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 250, .adv_w = 134, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 258, .adv_w = 113, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 264, .adv_w = 169, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 273, .adv_w = 150, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 281, .adv_w = 154, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 290, .adv_w = 132, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 297, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 309, .adv_w = 132, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 316, .adv_w = 124, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 323, .adv_w = 125, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 331, .adv_w = 150, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 339, .adv_w = 120, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 347, .adv_w = 183, .box_w = 11, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 360, .adv_w = 119, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 368, .adv_w = 110, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 376, .adv_w = 125, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 384, .adv_w = 70, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 389, .adv_w = 82, .box_w = 5, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 397, .adv_w = 70, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 402, .adv_w = 115, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 406, .adv_w = 116, .box_w = 7, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 407, .adv_w = 126, .box_w = 3, .box_h = 3, .ofs_x = 2, .ofs_y = 8},
    {.bitmap_index = 409, .adv_w = 117, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 414, .adv_w = 129, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 422, .adv_w = 106, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 427, .adv_w = 129, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 435, .adv_w = 115, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 441, .adv_w = 68, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 446, .adv_w = 117, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 454, .adv_w = 126, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 462, .adv_w = 57, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 464, .adv_w = 57, .box_w = 3, .box_h = 13, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 469, .adv_w = 115, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 477, .adv_w = 59, .box_w = 2, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 480, .adv_w = 193, .box_w = 9, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 488, .adv_w = 127, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 494, .adv_w = 126, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 500, .adv_w = 129, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 508, .adv_w = 129, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 516, .adv_w = 81, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 520, .adv_w = 97, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 525, .adv_w = 78, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 530, .adv_w = 126, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 536, .adv_w = 108, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 543, .adv_w = 167, .box_w = 10, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 552, .adv_w = 104, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 558, .adv_w = 108, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 567, .adv_w = 99, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 572, .adv_w = 70, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 577, .adv_w = 56, .box_w = 1, .box_h = 14, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 579, .adv_w = 70, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 584, .adv_w = 115, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 586, .adv_w = 208, .box_w = 12, .box_h = 1, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 588, .adv_w = 208, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 605, .adv_w = 208, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 622, .adv_w = 208, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 640, .adv_w = 208, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 657, .adv_w = 208, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 674, .adv_w = 208, .box_w = 13, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 694, .adv_w = 208, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 712, .adv_w = 208, .box_w = 13, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 730, .adv_w = 208, .box_w = 13, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 750, .adv_w = 208, .box_w = 10, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 765, .adv_w = 208, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 781, .adv_w = 208, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 798, .adv_w = 208, .box_w = 13, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 818, .adv_w = 208, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 835, .adv_w = 208, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 852, .adv_w = 208, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 869, .adv_w = 208, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 887, .adv_w = 208, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 905, .adv_w = 208, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 923, .adv_w = 208, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 941, .adv_w = 208, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 959, .adv_w = 208, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 976, .adv_w = 208, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 994, .adv_w = 208, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1010, .adv_w = 208, .box_w = 12, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1025, .adv_w = 208, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1042, .adv_w = 208, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1059, .adv_w = 208, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1077, .adv_w = 208, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1093, .adv_w = 208, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1110, .adv_w = 208, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1128, .adv_w = 208, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1145, .adv_w = 208, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1163, .adv_w = 208, .box_w = 13, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1183, .adv_w = 208, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1201, .adv_w = 208, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1217, .adv_w = 208, .box_w = 12, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1235, .adv_w = 208, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1252, .adv_w = 208, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1269, .adv_w = 208, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1285, .adv_w = 208, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = -1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0xa, 0xb, 0x15c, 0x373, 0x3fb, 0x41b, 0x437,
    0x5cc, 0x5d1, 0x62f, 0x8de, 0x928, 0xb07, 0xd9e, 0xff2,
    0x1100, 0x12c5, 0x1509, 0x156e, 0x16cd, 0x1770, 0x17b0, 0x17f6,
    0x18f4, 0x1d63, 0x1e42, 0x1eaa, 0x1ff1, 0x3151, 0x316e, 0x3a4c,
    0x3dbe, 0x3df7, 0x41d4, 0x4201, 0x434d, 0x477f, 0x47ed, 0x47fb,
    0x4a75, 0x4b96
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 19968, .range_length = 19351, .glyph_id_start = 96,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 42, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t lv_font_noto_sans_sc_13 = {
#else
lv_font_t lv_font_noto_sans_sc_13 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 15,          /*The maximum line height required by the font*/
    .base_line = 4,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LV_FONT_NOTO_SANS_SC_13*/

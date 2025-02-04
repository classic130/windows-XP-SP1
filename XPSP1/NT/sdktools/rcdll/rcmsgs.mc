;/*++ BUILD Version: 0001    // Increment this if a change has global effects
;
;Copyright (c) 1991  Microsoft Corporation
;
;Module Name:
;
;    rcdll.h
;
;Abstract:
;
;    This file contains the message definitions for the Win32
;    resource compiler program.
;
;Author:
;
;    Floyd Rogers - extracted from getmsg.h
;
;Revision History:
;
;Notes:
;
;    This file is generated by the MC tool from the rcdll.mc file.
;
;--*/
;
;#ifndef _RCDLLMSG_
;#define _RCDLLMSG_
;
;

LanguageNames=(Japanese=1041:msg00411)

MessageId=312 SymbolicName=MSG_WARN
Language=English
warning
.
Language=Japanese
warning
.

MessageId=313 SymbolicName=MSG_ERROR
Language=English
error
.
Language=Japanese
error
.

MessageId=314 SymbolicName=MSG_FATAL
Language=English
fatal error
.
Language=Japanese
fatal error
.

MessageId=1000
Language=English
UNKNOWN FATAL ERROR
                From the Help menu choose the Technical Support command
or open the Technical Support help file for more information
.
Language=Japanese
UNKNOWN FATAL ERROR
                »Îß°Ä»°ËÞ½?Ü?Å?²?A??­?¾?³?¢
.

MessageId=1002
Language=English
out of heap space
.
Language=Japanese
out of heap space
.

MessageId=1003
Language=English
error count exceeds %1!d!; stopping compilation
.
Language=Japanese
´×°Ìª %1!d! ð´¦Üµ½; ºÝÊß²ÙÍf³êÜ·
.

MessageId=1004
Language=English
unexpected end of file found
.
Language=Japanese
svÈ EOF ª èÜ·
.

MessageId=1005
Language=English
file not found: %3
.
Language=Japanese
File not found: %3
.

MessageId=1007
Language=English
unrecognized option '%1!ws!' in '%2'
.
Language=Japanese
'%1!ws!' µÌß¼®ÝàÌ '%2' ÍLøÈµÌß¼®ÝÅÍ èÜ¹ñ
.

MessageId=1008
Language=English
no input file specified
.
Language=Japanese
ºÝÊß²Ù·éÌ§²Ùª^¦çêÄ¢Ü¹ñ
.

MessageId=1009
Language=English
compiler limit : macros too deeply nested '%1!ws!'
.
Language=Japanese
½ÌÏ¸Ûª¯ÉWJ³êÜµ½ '%1!ws!'
.

MessageId=1010
Language=English
no output file specified
.
Language=Japanese
no output file specified
.

MessageId=1011
Language=English
compiler limit : '%1!ws!': macro definition too big
.
Language=Japanese
ºÝÊß²×ÌÀE : '%1!ws!': Ï¸Ûè`ªå«·¬Ü·
.

MessageId=1012
Language=English
mismatched parenthesis : missing '%1!c!'
.
Language=Japanese
¶bºÌsêv : '%!c!' ª èÜ¹ñ
.

MessageId=1013
Language=English
%1 (%2!d!): error RC1013 : mismatched parentheses
.
Language=Japanese
%1 (%2!d!): error RC1013 : ¶EÌ¶bºÌªsêvÅ·
.

MessageId=1014
Language=English
too many include files : depth = %1!d!
.
Language=Japanese
%#include ÃÞ¨Ú¸Ã¨ÌÞÌÈ½Äª[·¬Ü· : È½Ä¶³ÝÄ = %1!d!
.

MessageId=1015
Language=English
cannot open include file '%1!ws!'.
.
Language=Japanese
cannot open include file '%1!ws!'.
.

MessageId=1016
Language=English
%#if[n]def expected an identifier
.
Language=Japanese
%#if[n]def ÉÍ¯ÊqªKvÅ·
.

MessageId=1017
Language=English
invalid integer constant expression
.
Language=Japanese
³øÈ®è®Å·
.

MessageId=1018
Language=English
unexpected '#elif'
.
Language=Japanese
svÈ '#elif' ª èÜ·
.

MessageId=1019
Language=English
unexpected '#else'
.
Language=Japanese
svÈ '#else'
.

MessageId=1020
Language=English
unexpected '#endif'
.
Language=Japanese
svÈ '#endif'
.

MessageId=1021
Language=English
invalid preprocessor command '%1!ws!'
.
Language=Japanese
'%1!ws!' Í³øÈÌßØÌßÛ¾b» ºÏÝÄÞÅ·
.

MessageId=1022
Language=English
expected '#endif'
.
Language=Japanese
'#endif' ªKvÅ·
.

MessageId=1023
Language=English
cannot open source file '%1'
.
Language=Japanese
¿°½ Ì§²Ù '%1' ªµ°ÌßÝÅ«Ü¹ñ
.

MessageId=1047
Language=English
too many %1!ws! options, '%2!ws!'
.
Language=Japanese
%1!ws! Ì×¸Þª½·¬Ü·, '%2!ws!'
.

MessageId=1048
Language=English
unknown option '%1!wc!' in '%2!ws!'
.
Language=Japanese
'%1!wc!' µÌß¼®Ý ('%2!ws!' ) ªs³Å·
.

MessageId=1049
Language=English
invalid numerical argument '%1!ws!'
.
Language=Japanese
ø '%1!ws!' Ìlªs³Å·
.

MessageId=1052
Language=English
compiler limit : #if or #ifdef blocks nested too deeply
.
Language=Japanese
ºÝÊß²×ÌÀE : #if Ü½Í #ifdef ÌÞÛb¸ÌÈ½Äª[·¬Ü·
.

MessageId=1060
Language=English
out of far heap space
.
Language=Japanese
far Ë°Ìß½Íß°½ªs«Å·
.

MessageId=1067
Language=English
compiler limit : identifier overflowed internal buffer
.
Language=Japanese
compiler limit : identifier overflowed internal buffer
.

MessageId=1101
Language=English
RC : fatal error RC1101 : no resource binary filename specified.
.
Language=Japanese
RC : fatal error RC1101 : Ø¿°½ÌÊÞ²ÅØÌ§²Ù¼ªwè³êÄ¢Ü¹ñ.
.

MessageId=1102
Language=English
RC : fatal error RC1102 : internal error : too many arguments to RCPP
.
Language=Japanese
RC : fatal error RC1102 : ÀsÂ\Ì§²Ù¼ªwè³êÄ¢Ü¹ñ.
.

MessageId=1103
Language=English
;-red RC : fatal error RC1103 : Invalid switch, %1
RC : fatal error RC1103 : invalid option, %1
.
Language=Japanese
RC : fatal error RC1103 : ½²bÁ %1 Í³øÅ·
.

MessageId=1104
Language=English
RC : fatal error RC1104 : invalid option, %1: too many /u options
.
Language=Japanese
RC : fatal error RC1104 : Invalid switch, %1: too many -U switches
.

MessageId=1105
Language=English
RC : fatal error RC1105 : invalid option, %1: too many /d options
.
Language=Japanese
RC : fatal error RC1105 : Invalid switch, %1: too many -D switches
.

MessageId=1106
Language=English
RC : fatal error RC1106 : invalid option: %1
.
Language=Japanese
RC : fatal error RC1106 : ½²bÁ %1 Í³øÅ·.
.

MessageId=1107
Language=English
RC : fatal error RC1107 : invalid usage; use RC /? for Help
.
Language=Japanese
RC : fatal error RC1107 : \¶ª³µ­ èÜ¹ñ.  \¶ðmF·éÉÍ rc -? ÆüÍµÄ­¾³¢
.

MessageId=1109
Language=English
RC : fatal error RC1109 : error creating %1
.
Language=Japanese
RC : fatal error RC1109 : %1 Ìì¬É´×°ª­¶µÜµ½
.

MessageId=1110
Language=English
RC : fatal error RC1110 : could not open %1
.
Language=Japanese
RC : fatal error RC1110 : %1 ªµ°ÌßÝÅ«Ü¹ñ
.

MessageId=1116
Language=English
RC : fatal error RC1116 : RC terminating after preprocessor errors
.
Language=Japanese
RC : fatal error RC1116 : RC ÍÌßØÌßÛ¾b»´×°ÌãÉI¹µÜ·
.

MessageId=1120
Language=English
RC : fatal error RC1120 : out of memory, needed %1!d! bytes
.
Language=Japanese
RC : fatal error RC1120 : ÒÓØs«Å·.  Æ %1!d! ÊÞ²ÄKvÅ·
.

MessageId=1121
Language=English
RC : fatal error RC1121 : I/O error reading file
.
Language=Japanese
RC : fatal error RC1121 : Ì§²ÙÇÝoµÌ I/O ´×°
.

MessageId=1122
Language=English
RC : fatal error RC1122 : I/O error writing file
.
Language=Japanese
RC : fatal error RC1122 : Ì§²Ù«ÝÌ I/O ´×°
.

MessageId=1123
Language=English
RC : fatal error RC1123 : I/O error seeking in file
.
Language=Japanese
RC : fatal error RC1123 : Ì§²ÙõÌ I/O ´×°
.

MessageId=1201
Language=English
RC : fatal error RC1201 : invalid option; missing include path after /i
.
Language=Japanese
RC : fatal error RC1201 : Invalid switch - missing include path after -I.
.

MessageId=1202
Language=English
RC : fatal error RC1202 : invalid option - missing default language ID after /l
.
Language=Japanese
RC : fatal error RC1202 : Invalid switch - missing default language ID after -L.
.

MessageId=1203
Language=English
RC : fatal error RC1203 : invalid hexadecimal default language ID specified
.
Language=Japanese
RC : fatal error RC1203 : Invalid hexidecimal default language Id specified.
.

MessageId=1204
Language=English
RC : fatal error RC1204 : invalid switch; missing code page after /c
.
Language=Japanese
RC : fatal error RC1204 : Invalid switch - missing CodePage after -C.
.

MessageId=1205
Language=English
RC : fatal error RC1205 : invalid code page
.
Language=Japanese
RC : fatal error RC1205 : Invalid Code page.
.

MessageId=1206
Language=English
RC : fatal error RC1206 : code page specified on command line not in registry
.
Language=Japanese
RC : fatal error RC1206 : Specified Code page at cmd line does not exist in registry.
.

MessageId=1207
Language=English
RC : fatal error RC1207 : invalid default code page
.
Language=Japanese
RC : fatal error RC1207 : Default Code page is invalid.
.

MessageId=1208
Language=English
RC : fatal error RC1208 : input file has .RES extension
.
Language=Japanese
RC : fatal error RC1208 : Input file has .RES suffix
.

MessageId=1209
Language=English
RC : fatal error RC1209 : invalid option; missing resource base number after /br
.
Language=Japanese
RC : fatal error RC1209 : invalid option; missing resource base number after /br
.

MessageId=1210
Language=English
RC : fatal error RC1210 : resource base number must be from 1 to 32767
.
Language=Japanese
RC : fatal error RC1210 : resource base number must be from 1 to 32767
.

MessageId=1211
Language=English
RC : fatal error RC1211 : invalid option - missing argument after /z
.
Language=Japanese
RC : fatal error RC1211 : Invalid switch - missing argument after -Z.
.

MessageId=1212
Language=English
RC : fatal error RC1212 : invalid option - /z argument missing substitute font name
.
Language=Japanese
RC : fatal error RC1212 : Invalid switch - -Z argument missing substitute font name.
.

MessageId=1213
Language=English
RC : fatal error RC1213 : invalid option - /z argument specified too many font names
.
Language=Japanese
RC : fatal error RC1213 : Invalid switch - -Z argument specified too many font names.
.

MessageId=2001
Language=English
newline in constant
.
Language=Japanese
èÉüs¶ª èÜ·
.

MessageId=2003
Language=English
expected 'defined id'
.
Language=Japanese
'defined'ÌãÉA¯ÊqªKvÅ·
.

MessageId=2004
Language=English
expected 'defined(id)'
.
Language=Japanese
'defined'É±­¶¶bº(()ÌãÉA¯ÊqªKvÅ·
.

MessageId=2005
Language=English
%#line expected a line number, found '%1!ws!'
.
Language=Japanese
%#line ÉÍsÔªKvÅ· ('%1!ws!')
.

MessageId=2006
Language=English
%#include expected a file name, found '%1!ws!'
.
Language=Japanese
%#include ÉÍÌ§²Ù¼ªKvÅ· ('%1!ws!')
.

MessageId=2007
Language=English
%#define syntax
.
Language=Japanese
#defineÌãÉ¯ÊqªKvÅ·
.

MessageId=2008
Language=English
'%1!wc!' : unexpected in macro definition
.
Language=Japanese
'%1!wc!' : Ï¸Ûè`ÅÍsvÅ·
.

MessageId=2009
Language=English
reuse of macro formal '%1!ws!'
.
Language=Japanese
Ï¸ÛÌ¼ø '%1!ws!' ðÄgpµÄ¢Ü·
.

MessageId=2010
Language=English
'%1!wc!' : unexpected in formal parameter list
.
Language=Japanese
'%1!wc!' : ¼øÌØ½ÄÉÍsvÅ·
.

MessageId=2012
Language=English
missing name following '<'
.
Language=Japanese
'<' ÌãÌ¼Oª èÜ¹ñ
.

MessageId=2013
Language=English
missing '>'
.
Language=Japanese
'>' ª èÜ¹ñ
.

MessageId=2014
Language=English
preprocessor command must start as first non-whitespace
.
Language=Japanese
ÌßØÌßÛ¾b» ºÏÝÄÞÌæªÍó¶ÈOÅÈ¯êÎÈèÜ¹ñ
.

MessageId=2015
Language=English
too many chars in constant
.
Language=Japanese
èÌ¶ª½·¬Ü·
.

MessageId=2016
Language=English
no closing single quote
.
Language=Japanese
nßÌ ' ÉÎ·é ' ª èÜ¹ñ
.

MessageId=2017
Language=English
illegal escape sequence
.
Language=Japanese
³øÈ´½¹°Ìß¼°¹Ý½Å·
.

MessageId=2018
Language=English
unknown character '0x%1!x!'
.
Language=Japanese
'0x%1!x!' Í³øÈ¶Å·
.

MessageId=2019
Language=English
expected preprocessor command, found '%1!wc!'
.
Language=Japanese
'%1!wc!' ÉÍÌßØÌßÛ¾b» ºÏÝÄÞªKvÅ·
.

MessageId=2020
Language=English
illegal digit %1!wc! for base %2!d!
.
Language=Japanese
illegal digit %1!wc! for base %2!d!
.

MessageId=2021
Language=English
expected exponent value, not '%1!wc!'
.
Language=Japanese
'%1!wc!' ÈOÌwlªKvÅ·
.

MessageId=2022
Language=English
'%1!d!' : too big for char
.
Language=Japanese
'%1!ld!' : lªå«·¬Ä char ^ÅÍµ¦Ü¹ñ
.

MessageId=2101
Language=English
%1 (%2!d!): error RC2101 : invalid directive in preprocessed RC file
.
Language=Japanese
%1 (%2!d!): error RC2101 : ÌßØÌßÛ¾½Ì RC Ì§²ÙÉÍ³øÈÃÞ¨Ú¸Ã¨ÌÞÅ·
.

MessageId=2102
Language=English
%1 (%2!d!): error RC2102 : string literal too long
.
Language=Japanese
%1 (%2!d!): error RC2102 : ØÃ×Ù¶ñª··¬Ü·
.

MessageId=2103
Language=English
%1 (%2!d!): error RC2103 : unexpected end of file in string literal
.
Language=Japanese
%1 (%2!d!): error RC2103 : ØÃ×Ù¶ñÌÉsvÈ EOF ª èÜ·
.

MessageId=2104
Language=English
%1 (%2!d!): error RC2104 : undefined keyword or key name: %3!ws!
.
Language=Japanese
%1 (%2!d!): error RC2104 : ¢è`Ì·°Ü°ÄÞÜ½Í·°¼: %3!ws!
.

MessageId=2105
Language=English
%1 (%2!d!): error RC2105 : BEGIN expected in string table
.
Language=Japanese
%1 (%2!d!): error RC2105 : ½ÄØÝ¸ÞÃ°ÌÞÙÉÍ BEGIN ªKvÅ·
.

MessageId=2106
Language=English
%1 (%2!d!): error RC2106 : BEGIN expected in accelerator table
.
Language=Japanese
%1 (%2!d!): error RC2106 : ±¸¾×Ú°ÀÃ°ÌÞÙÉÍ BEGIN ªKvÅ·
.

MessageId=2107
Language=English
%1 (%2!d!): error RC2107 : expected numeric command value
.
Language=Japanese
%1 (%2!d!): error RC2107 : ºÏÝÄÞlªKvÅ·
.

MessageId=2108
Language=English
%1 (%2!d!): error RC2108 : mismatched parentheses
.
Language=Japanese
%1 (%2!d!): error RC2108 : ¶bºÌ`ªÁÄ¢Ü¹ñ
.

MessageId=2109
Language=English
%1 (%2!d!): error RC2108 : expected numerical dialog constant
.
Language=Japanese
%1 (%2!d!): error RC2108 : Expected Numerical Dialog constant
.

MessageId=2110
Language=English
%1 (%2!d!): error RC2110 : expected numerical string id
.
Language=Japanese
%1 (%2!d!): error RC2110 : expected numerical string id
.

MessageId=2111
Language=English
%1 (%2!d!): error RC2111 : invalid control type
.
Language=Japanese
%1 (%2!d!): error RC2111 : ºÝÄÛ°ÙÀ²ÌßÍ³øÅ·
.

MessageId=2112
Language=English
%1 (%2!d!): error RC2112 : BEGIN expected in dialog
.
Language=Japanese
%1 (%2!d!): error RC2112 : DIALOG ¶É BEGIN ªKvÅ·
.

MessageId=2113
Language=English
%1 (%2!d!): error RC2113 : END expected in dialog
.
Language=Japanese
%1 (%2!d!): error RC2113 : DIALOG ¶É END ªKvÅ·
.

MessageId=2114
Language=English
%1 (%2!d!): error RC2114 : expected control class name
.
Language=Japanese
%1 (%2!d!): error RC2114 : ºÝÄÛ°ÙÌ¸×½¼ªKvÅ·
.

MessageId=2115
Language=English
%1 (%2!d!): error RC2115 : text string or ordinal expected in control
.
Language=Japanese
%1 (%2!d!): error RC2115 : CONTROL ¶ÉÃ·½Ä¶ñ©lªKvÅ·
.

MessageId=2116
Language=English
%1 (%2!d!): error RC2116 : expecting number for ID
.
Language=Japanese
%1 (%2!d!): error RC2116 : ID ÉÔªKvÅ·
.

MessageId=2117
Language=English
%1 (%2!d!): error RC2117 : expected numeric point size
.
Language=Japanese
%1 (%2!d!): error RC2117 : Îß²ÝÄªKvÅ·
.

MessageId=2118
Language=English
%1 (%2!d!): error RC2118 : expected font face name
.
Language=Japanese
%1 (%2!d!): error RC2118 : Ì«ÝÄÌÌª²½¼ªKvÅ·
.

MessageId=2119
Language=English
%1 (%2!d!): error RC2119 : expecting quoted string in dialog title
.
Language=Japanese
%1 (%2!d!): error RC2119 : ÀÞ²±Û¸ÞÌÀ²ÄÙÍ "" ÅÍñÅ­¾³¢
.

MessageId=2120
Language=English
%1 (%2!d!): error RC2120 : expecting quoted string in dialog class
.
Language=Japanese
%1 (%2!d!): error RC2120 : ÀÞ²±Û¸ÞÌ¸×½Í "" ÅÍñÅ­¾³¢
.

MessageId=2121
Language=English
%1 (%2!d!): error RC2121 : BEGIN expected in menu
.
Language=Japanese
%1 (%2!d!): error RC2121 : MENU ¶ÉÍ BEGIN ªKvÅ·
.

MessageId=2122
Language=English
%1 (%2!d!): error RC2122 : unknown menu subtype
.
Language=Japanese
%1 (%2!d!): error RC2122 : ÒÆ­°Ì»ÌÞÀ²ÌßÍ³øÅ·
.

MessageId=2123
Language=English
%1 (%2!d!): error RC2123 : END expected in menu
.
Language=Japanese
%1 (%2!d!): error RC2123 : MENU ¶ÉÍ END ªKvÅ·
.

MessageId=2124
Language=English
%1 (%2!d!): error RC2124 : empty menus not allowed
.
Language=Japanese
%1 (%2!d!): error RC2124 : óÌÒÆ­°Íè`Å«Ü¹ñ
.

MessageId=2125
Language=English
%1 (%2!d!): error RC2125 : expected ID value for menu item
.
Language=Japanese
%1 (%2!d!): error RC2125 : MENUITEM ¶ÉÍ ID lªKvÅ·
.

MessageId=2126
Language=English
%1 (%2!d!): error RC2126 : expected menu string
.
Language=Japanese
%1 (%2!d!): error RC2126 : MENU ¶ÉÍ¶ñªKvÅ·
.

MessageId=2127
Language=English
%1 (%2!d!): error RC2127 : version WORDs separated by commas expected
.
Language=Japanese
%1 (%2!d!): error RC2127 : ÊÞ°¼Þ®ÝÌ WORD Í¶ÝÏÅæØçÈ¯êÎÈèÜ¹ñ
.

MessageId=2128
Language=English
%1 (%2!d!): error RC2128 : DWORD expected
.
Language=Japanese
%1 (%2!d!): error RC2128 : DWORD ªKvÅ·
.

MessageId=2129
Language=English
%1 (%2!d!): error RC2129 : BEGIN expected in VERSIONINFO resource
.
Language=Japanese
%1 (%2!d!): error RC2129 : VERSIONINFO Ø¿°½ÉÍ BEGIN ªKvÅ·
.

MessageId=2130
Language=English
%#line expected a string containing the file name, found '%1!ws!'
.
Language=Japanese
'%1!ws!' Ì #line ÉÍÌ§²Ù¼ðÜÞ¶ñªKvÅ·
.

MessageId=2131
Language=English
%1 (%2!d!): error RC2131 : expecting quoted string for key
.
Language=Japanese
%1 (%2!d!): error RC2131 : ±¸¾×Ú°À·°Ìè`ÅÍ¶ñð "" ÅÍñÅ­¾³¢
.

MessageId=2132
Language=English
%1 (%2!d!): error RC2132 : expected VALUE, BLOCK, or END keyword
.
Language=Japanese
%1 (%2!d!): error RC2132 : VALUE, BLOCK, END Ì¤¿Ì¢¸ê©Ì·°Ü°ÄÞªKvÅ·.
.

MessageId=2133
Language=English
%1 (%2!d!): error RC2133 : unexpected value in value data
.
Language=Japanese
%1 (%2!d!): error RC2133 : value ÌÃÞ°ÀlÍ³øÅ·
.

MessageId=2134
Language=English
%1 (%2!d!): error RC2134 : BEGIN expected in RCDATA
.
Language=Japanese
%1 (%2!d!): error RC2134 : RCDATA ¶ÉÍ BEGIN ªKvÅ·
.

MessageId=2135
Language=English
%1 (%2!d!): error RC2135 : file not found: %3
.
Language=Japanese
%1 (%2!d!): error RC2135 : File not found: %3
.

MessageId=2136
Language=English
%1 (%2!d!): error RC2136 : missing '=' in EXSTYLE=<flags>
.
Language=Japanese
%1 (%2!d!): error RC2136 : Missing '=' in EXSTYLE=<flags>
.

MessageId=2137
Language=English
empty character constant
.
Language=Japanese
¶èªóÅ·
.

MessageId=2138
Language=English
unmatched close comment '/*'
.
Language=Japanese
unmatched close comment '/*'
.

MessageId=2139
Language=English
%1 (%2!d!): error RC2139 : VERSION not a number
.
Language=Japanese
%1 (%2!d!): error RC2139 : VERSION not a number
.

MessageId=2140
Language=English
%1 (%2!d!): error RC2140 : CHARACTERISTICS not a number
.
Language=Japanese
%1 (%2!d!): error RC2140 : CHARACTERISTICS not a number
.

MessageId=2141
Language=English
%1 (%2!d!): error RC2141 : invalid type
.
Language=Japanese
%1 (%2!d!): error RC2141 : Invalid Type
.

MessageId=2142
Language=English
ellipsis requires three periods
.
Language=Japanese
ËßØµÄÞÍ 3 ÂKvÅ·
.

MessageId=2143
Language=English
%1 (%2!d!): error RC2143 : font names must be ordinals
.
Language=Japanese
%1 (%2!d!): error RC2143 : Font names must be ordinals
.

MessageId=2144
Language=English
%1 (%2!d!): error RC2144 : PRIMARY LANGUAGE ID not a number
.
Language=Japanese
%1 (%2!d!): error RC2144 : PRIMARY LANGUAGE ID not a number
.

MessageId=2145
Language=English
%1 (%2!d!): error RC2145 : PRIMARY LANGUAGE ID too large
.
Language=Japanese
%1 (%2!d!): error RC2145 : PRIMARY LANGUAGE ID too large
.

MessageId=2146
Language=English
%1 (%2!d!): error RC2146 : missing comma in LANGUAGE statement
.
Language=Japanese
%1 (%2!d!): error RC2146 : Missing COMMA in LANGUAGE Statement
.

MessageId=2147
Language=English
%1 (%2!d!): error RC2147 : SUBLANGUAGE ID not a number
.
Language=Japanese
%1 (%2!d!): error RC2147 : SUBLANGUAGE ID not a number
.

MessageId=2148
Language=English
%1 (%2!d!): error RC2148 : SUBLANGUAGE ID too large
.
Language=Japanese
%1 (%2!d!): error RC2148 : SUBLANGUAGE ID too large
.

MessageId=2149
Language=English
%1 (%2!d!): error RC2148 : expected numeric constant in string table
.
Language=Japanese
%1 (%2!d!): error RC2148 : Expected numeric constant in string table
.

MessageId=2150
Language=English
%1 (%2!d!): error RC2150 : expected string in STRINGTABLE
.
Language=Japanese
%1 (%2!d!): error RC2150 : Expected String in STRINGTABLE
.

MessageId=2151
Language=English
%1 (%2!d!): error RC2151 : cannot reuse string constants, %3!d!(0x%3!X!) - "%4!ws!" already defined. Cannot add %3!d!(0x%3!X!) - "%5!ws!".
.
Language=Japanese
%1 (%2!d!): error RC2151 : cannot reuse string constants, %3!d!(0x%3!X!) - "%4!ws!" already defined. Cannot add %3!d!(0x%3!X!) - "%5!ws!".
.

MessageId=2152
Language=English
%1 (%2!d!): error RC2152 : invalid control character
.
Language=Japanese
%1 (%2!d!): error RC2152 : Invalid control character
.

MessageId=2153
Language=English
hex constants must have at least 1 hex digit
.
Language=Japanese
 16 iÌèÍÅá 1 ¯½Ì 16 iÌlð½È¯êÎÈèÜ¹ñ
.

MessageId=2154
Language=English
%1 (%2!d!): error RC2154 : control character out of range [^A - ^Z]
.
Language=Japanese
%1 (%2!d!): error RC2154 : Control Character out of range [^A - ^Z]
.

MessageId=2155
Language=English
%1 (%2!d!): error RC2155 : invalid accelerator
.
Language=Japanese
%1 (%2!d!): error RC2155 : Invalid Accelerator
.

MessageId=2156
Language=English
%1 (%2!d!): error RC2156 : expected string or constant accelerator command
.
Language=Japanese
%1 (%2!d!): error RC2156 : Expected String or Constant Accelerator command
.

MessageId=2157
Language=English
%1 (%2!d!): error RC2157 : expected comma in accelerator table
.
Language=Japanese
%1 (%2!d!): error RC2157 : Expected Comma in Accelerator Table
.

MessageId=2159
Language=English
%1 (%2!d!): error RC2159 : invalid accelerator type [ASCII or VIRTKEY]
.
Language=Japanese
%1 (%2!d!): error RC2159 : Invalid Accelerator Type [ASCII or VIRTKEY]
.

MessageId=2160
Language=English
%## cannot occur at the beginning of a macro definition
.
Language=Japanese
Ï¸Ûè`ÌÅÉ ## ð­±ÆÍÅ«Ü¹ñ
.

MessageId=2161
Language=English
%## cannot occur at the end of a macro definition
.
Language=Japanese
Ï¸Ûè`ÌÅãÉ ## ð­±ÆÍÅ«Ü¹ñ
.

MessageId=2162
Language=English
expected macro formal parameter
.
Language=Japanese
Ï¸ÛÉ¼øªKvÅ·
.

MessageId=2163
Language=English
%1 (%2!d!): error RC2163 : accelerator type required [ASCII or VIRTKEY]
.
Language=Japanese
%1 (%2!d!): error RC2163 : Accelerator Type required [ASCII or VIRTKEY]
.

MessageId=2164
Language=English
%1 (%2!d!): error RC2164 : unexpected value in RCDATA
.
Language=Japanese
%1 (%2!d!): error RC2164 : Unexpected value in RCData
.

MessageId=2165
Language=English
%1 (%2!d!): error RC2165 : string not found in DLGINCLUDE statement
.
Language=Japanese
%1 (%2!d!): error RC2165 : String not found in DLGINCLUDE statement
.

MessageId=2166
Language=English
%1 (%2!d!): error RC2166 : numeric value expected at %3!wc!
.
Language=Japanese
%1 (%2!d!): error RC2166 : lÍ '%3!wc!' ÅÈ¯êÎÈèÜ¹ñ
.

MessageId=2167
Language=English
%1 (%2!d!): error RC2167 : unrecognized VERSIONINFO field;  BEGIN or comma expected
.
Language=Japanese
%1 (%2!d!): error RC2167 : VERSIONINFO Ì¨°ÙÄÞÍ³øÅ·;  BEGIN Ü½Í¶ÝÏ (,) ªKvÅ·
.

MessageId=2168
Language=English
%1 (%2!d!): error RC2168 : resource too large
.
Language=Japanese
%1 (%2!d!): error RC2168 : Ø¿°½ªå«·¬Ü·
.

MessageId=2169
Language=English
%1 (%2!d!): error RC2169 : resource file %3!ws! is not in 2.03 format
.
Language=Japanese
%1 (%2!d!): error RC2169 : Ø¿°½Ì§²Ù %3!ws! Í 2.03 Ì«°ÏbÄÅÍ èÜ¹ñ.
.

MessageId=2170
Language=English
%1 (%2!d!): error RC2170 : bitmap file %3!ws! is not in 3.00 format
.
Language=Japanese
%1 (%2!d!): error RC2170 : ËÞbÄÏbÌßÌ§²Ù %3!ws! Í 3.00 Ì«°ÏbÄÅÍ èÜ¹ñ.
.

MessageId=2171
Language=English
%1 (%2!d!): error RC2171 : unknown DIB header format
.
Language=Japanese
%1 (%2!d!): error RC2171 : DIB ÍbÀÞ°Ì§²ÙÍ³øÅ·
.

MessageId=2173
Language=English
%1 (%2!d!): error RC2173 : file %3!ws! is not a valid animated cursor/icon RIFF file
.
Language=Japanese
%1 (%2!d!): error RC2173 : file %3!ws! is not a valid animated cursor/icon RIFF file
.

MessageId=2174
Language=English
%1 (%2!d!): error RC2174 : bitmap file %3!ws! is not in 2.03 format
.
Language=Japanese
%1 (%2!d!): error RC2174 : ËÞbÄÏbÌßÌ§²Ù %3!ws! Í 2.03 Ì«°ÏbÄÅÍ èÜ¹ñ.
.

MessageId=2175
Language=English
%1 (%2!d!): error RC2175 : resource file %3!ws! is not in 3.00 format
.
Language=Japanese
%1 (%2!d!): error RC2175 : Ø¿°½Ì§²Ù %3!ws! Í 3.00 Ì«°ÏbÄÅÍ èÜ¹ñ.
.

MessageId=2176
Language=English
%1 (%2!d!): error RC2176 : old DIB in %3!ws!; pass it through SDKPAINT
.
Language=Japanese
%1 (%2!d!): error RC2176 : %3!ws! Ì DIB ÍÃ¢ÊÞ°¼Þ®ÝÌàÌÅ·.  SDKPAINT ðgÁÄnµÄ­¾³¢.
.

MessageId=2177
Language=English
constant too big
.
Language=Japanese
constant too big
.

MessageId=2180
Language=English
RC : fatal error RC2180 : unable to open temporary file
.
Language=Japanese
RC : fatal error RC2180 : ÃÝÎß×Ø Ì§²Ùðµ°ÌßÝÅ«Ü¹ñ
.

MessageId=2181
Language=English
%1 (%2!d!): error RC2181 : duplicate font ID %3!d!
.
Language=Japanese
%1 (%2!d!): error RC2181 : Duplicate Font ID %3!d!
.

MessageId=2182
Language=English
%1 (%2!d!): warning RC2182 : duplicate dialog control ID %3!d!
.
Language=Japanese
%1 (%2!d!): warning RC2182 : Duplicate dialog control ID %3!d!
.

MessageId=2189
Language=English
%#error : %1!ws!
.
Language=Japanese
%#error ÃÞ¨Ú¸Ã¨ÌÞª èÜµ½: %1!ws! ÍLqÂ\ÈÃ·½ÄÅ·
.

MessageId=2235
Language=English
%1 (%2!d!): error RC2235 : too many arguments supplied
.
Language=Japanese
%1 (%2!d!): error RC2235 : Too many arguments supplied
.

MessageId=2236
Language=English
%1 (%2!d!): error RC2236 : required parameter missing
.
Language=Japanese
%1 (%2!d!): error RC2236 : Required parameter missing
.

MessageId=2237
Language=English
%1 (%2!d!): error RC2237 : numeric value expected at %3!ws!
.
Language=Japanese
%1 (%2!d!): error RC2237 : Numeric value expected at %3!ws!
.

MessageId=2238
Language=English
%1 (%2!d!): error RC2238 : string expected at %3!ws!
.
Language=Japanese
%1 (%2!d!): error RC2238 : String expected at %3!ws!
.

MessageId=2239
Language=English
%1 (%2!d!): error RC2239 : BEGIN expected at %3!ws!
.
Language=Japanese
%1 (%2!d!): error RC2239 : BEGIN expected at %3!ws!
.

MessageId=2240
Language=English
%1 (%2!d!): error RC2240 : END expected at %3!ws!
.
Language=Japanese
%1 (%2!d!): error RC2240 : END expected at %3!ws!
.

MessageId=2241
Language=English
%1 (%2!d!): error RC2241 : mismatched parentheses
.
Language=Japanese
%1 (%2!d!): error RC2241 : Mismatched parentheses
.

MessageId=2242
Language=English
%1 (%2!d!): error RC2242 : invalid numeric expression at %3!ws!
.
Language=Japanese
%1 (%2!d!): error RC2242 : Invalid numeric expression at %3!ws!
.

MessageId=2243
Language=English
%1 (%2!d!): error RC2243 : invalid string expression at %3!ws!
.
Language=Japanese
%1 (%2!d!): error RC2243 : Invalid string expression at %3!ws!
.

MessageId=2244
Language=English
%1 (%2!d!): error RC2244 : can't have a separator with a popup menu
.
Language=Japanese
%1 (%2!d!): error RC2244 : Can't have a separator with a popup menu
.

MessageId=2245
Language=English
%1 (%2!d!): error RC2245 : invalid submenu type at %3!ws!
.
Language=Japanese
%1 (%2!d!): error RC2245 : Invalid submenu type at %3!ws!
.

MessageId=2246
Language=English
%1 (%2!d!): error RC2246 : empty menus not allowed
.
Language=Japanese
%1 (%2!d!): error RC2246 : Empty menus not allowed
.

MessageId=2247
Language=English
%1 (%2!d!): error RC2247 : SYMBOL name too long
.
Language=Japanese
%1 (%2!d!): error RC2247 : SYMBOL name too long
.

MessageId=2248
Language=English
%1 (%2!d!): error RC2248 : SYMBOL definition did not terminate with delimiter
.
Language=Japanese
%1 (%2!d!): error RC2248 : SYMBOL definition did not terminate with delimiter
.

MessageId=2249
Language=English
%1 (%2!d!): error RC2249 : SYMBOL definition missing quote
.
Language=Japanese
%1 (%2!d!): error RC2249 : SYMBOL definition missing quote
.

MessageId=2250
Language=English
%1 (%2!d!): error RC2250 : expected numerical toolbar constant
.
Language=Japanese
%1 (%2!d!): error RC2250 : expected numerical toolbar constant
.

MessageId=2251
Language=English
%1 (%2!d!): error RC2251 : BEGIN expected in toolbar
.
Language=Japanese
%1 (%2!d!): error RC2251 : BEGIN expected in toolbar
.

MessageId=2252
Language=English
%1 (%2!d!): error RC2252 : END expected in toolbar
.
Language=Japanese
%1 (%2!d!): error RC2252 : END expected in toolbar
.

MessageId=2253
Language=English
%1 (%2!d!): error RC2253 : unknown toolbar item type
.
Language=Japanese
%1 (%2!d!): error RC2253 : unknown toolbar item type
.

MessageId=2254
Language=English
%1 (%2!d!): error RC2254 : empty toolbars not allowed
.
Language=Japanese
%1 (%2!d!): error RC2254 : empty toolbars not allowed
.

MessageId=2255
Language=English
%1 (%2!d!): error RC2255 : named STRINGTABLEs are not allowed
.
Language=Japanese
%1 (%2!d!): error RC2255 : named STRINGTABLEs are not allowed
.

MessageId=4000
Language=English
UNKNOWN WARNING
                From the Help menu choose the Technical Support command
or open the Technical Support help file for more information
.
Language=Japanese
UNKNOWN WARNING
                »Îß°Ä»°ËÞ½?Ü?Å?²?A??­?¾?³?¢
.

MessageId=4002
Language=English
too many actual parameters for macro '%1!ws!'
.
Language=Japanese
'%1!ws!' Ï¸ÛÌÀøª½·¬Ü·
.

MessageId=4003
Language=English
not enough actual parameters for macro '%1!ws!'
.
Language=Japanese
'%1!ws!' Ï¸ÛÌÀøª«èÜ¹ñ
.

MessageId=4004
Language=English
missing close parenthesis after 'defined'
.
Language=Japanese
'defined' ÌãÌ "")"" ª èÜ¹ñ
.

MessageId=4005
Language=English
'%1!ws!' : redefinition
.
Language=Japanese
'%1!ws!' : Äè`³êÄ¢Ü·
.

MessageId=4006
Language=English
%#undef expected an identifier
.
Language=Japanese
%#undef ÉÍ¯ÊqªKvÅ·
.

MessageId=4009
Language=English
string too big, trailing chars truncated
.
Language=Japanese
¶ñªå«·¬Ü·. ]Á½¶ÍØèÌÄçêÜ·
.

MessageId=4011
Language=English
identifier truncated to '%1!ws!'
.
Language=Japanese
'%1!ws!' : ¯ÊqÍØèÌÄçêÄ '%d' ¶ÉÈèÜµ½
.

MessageId=4012
Language=English
float constant in a cross compilation
.
Language=Japanese
®¬_èª¸Û½ ºÝÊß²Ù³êÜµ½
.

MessageId=4067
Language=English
unexpected characters following '%1!ws!' directive; newline expected
.
Language=Japanese
'%1!ws!' ÃÞ¨Ú¸Ã¨ÌÞÈ~Ì¶ñÍsvÅ· - üs¶ªKvÅ·
.

MessageId=4079
Language=English
unexpected token '%1!ws!'
.
Language=Japanese
'%1!ws!' ÍsvÈÄ°¸ÝÅ·
.

MessageId=4093
Language=English
unescaped newline in character constant in inactive code
.
Language=Japanese
ÌßØÌßÛ¾b» ÃÞ¨Ú¸Ã¨ÌÞÌè®ª 0 Æ]¿³êA»ÌÌº°ÄÞð³øÉµÜµ½
.

MessageId=4111
Language=English
unexpected token 'string'
.
Language=Japanese
'string' Í ³øÈÄ°¸ÝÅ·
.

MessageId=4117
Language=English
macro name '%1!ws!' is reserved, %2 ignored
.
Language=Japanese
Ï¸Û¼ '%1!ws!' Í\ñ³êÄ¢Ü·, %2 Í³³êÜ·
.

MessageId=4203
Language=English
%1 (%2!d!): warning RC4203 : SHIFT or CONTROL used without VIRTKEY
.
Language=Japanese
%1 (%2!d!): warning RC4203 : SHIFT Ü½Í CONTROL ª VIRTKEY ÈµÅgíêÄ¢Ü·
.

MessageId=4204
Language=English
%1 (%2!d!): warning RC4204 : ASCII character not equivalent to virtual key code
.
Language=Japanese
%1 (%2!d!): warning RC4204 : ±Ì ASCII ¶ÉÎµÄ¢é¼z·°º°ÄÞÍ èÜ¹ñ
.

MessageId=4205
Language=English
%1 (%2!d!): warning RC4205 : string literal longer than 256; stored anyway
.
Language=Japanese
%1 (%2!d!): warning RC4205 : String Literal longer than 256 - stored anyway
.

MessageId=4206
Language=English
%1 (%2!d!): warning RC4206 : name string too long; truncated at 256
.
Language=Japanese
%1 (%2!d!): warning RC4206 : Name String too long - truncated at 256
.

MessageId=4207
Language=English
%1 (%2!d!): warning RC4206 : type string too long; truncated at 256
.
Language=Japanese
%1 (%2!d!): warning RC4206 : Type String too long - truncated at 256
.

MessageId=4208
Language=English
%1 (%2!d!): warning RC4206 : title string too long; truncated at 256
.
Language=Japanese
%1 (%2!d!): warning RC4206 : Title String too long - truncated at 256
.

MessageId=4210
Language=English
Missing left parenthesis in code_page #pragma
.
Language=Japanese
Missing left parenthesis in code_page #pragma
.

MessageId=4211
Language=English
Missing right parenthesis in code_page #pragma
.
Language=Japanese
Missing right parenthesis in code_page #pragma
.

MessageId=4212
Language=English
Codepage not integer:  %0
.
Language=Japanese
Codepage not integer:  %0
.

MessageId=4213
Language=English
Codepage 1200 (Unicode) not allowed:  ignored
.
Language=Japanese
Codepage 1200 (Unicode) not allowed:  ignored
.

MessageId=4214
Language=English
Codepage not valid:  ignored
.
Language=Japanese
Codepage not valid:  ignored
.

MessageId=4413
Language=English
Cannot determine file type for '%1' : assuming 8-bit ASCII
.
Language=Japanese
'%1' ÌÌ§²ÙÀ²ÌßªÁèÅ«Ü¹ñ : 8 ËÞbÄÌ ASCII Æ¼èµÜ·
.

MessageId=4506
Language=English
%1 (%2!d!): warning RC4506 : scaling cursor %3!ws! down to 16 by 16
.
Language=Japanese
%1 (%2!d!): warning RC4506 : scaling cursor %3!ws! down to 16 by 16
.

MessageId=4507
Language=English
%1 (%2!d!): warning RC4507 : Macintosh cursor %3!ws! must be 16 by 16 and monochrome; ignored
.
Language=Japanese
%1 (%2!d!): warning RC4507 : Macintosh cursor %3!ws! must be 16 by 16 and monochrome; ignored
.

MessageId=4508
Language=English
%1 (%2!d!): warning RC4508 : Ignoring icon %3!ws! with invalid Macintosh size or color
.
Language=Japanese
%1 (%2!d!): warning RC4508 : Ignoring icon %3!ws! with invalid Macintosh size or color
.

MessageId=4509
Language=English
%1 (%2!d!): warning RC4509 : Resource types '%3' and '%4' were both mapped to '%5'
.
Language=Japanese
%1 (%2!d!): warning RC4509 : Resource types '%3' and '%4' were both mapped to '%5'
.

MessageId=4510
Language=English
%1 (%2!d!): warning RC4510 : Hardcoded font '%3!ws!' changed to '%4!ws!'
.
Language=Japanese
%1 (%2!d!): warning RC4510 : Hardcoded font '%3!ws!' changed to '%4!ws!'
.

MessageId=10001
Language=English
Microsoft (R) Windows (R) Resource Compiler, Version %1 - Build %2!d!
.
Language=Japanese
Microsoft (R) Windows (R) Resource Compiler Version %1.%2!02d!J
.

MessageId=10002
Language=English
Copyright (C) Microsoft Corp. 1985-2001. All rights reserved.
.
Language=Japanese
Copyright (C) Microsoft Corp. 1985-2001. All rights reserved.
.

MessageId=10102
Language=English
Creating %1
.
Language=Japanese
%1 ðì¬µÄ¢Ü·
.

MessageId=10105
Language=English
Copying segment %1!d! (%2!u! bytes)
.
Language=Japanese
¾¸ÞÒÝÄ %1!d! (%2!lu! ÊÞ²Ä) ðºËß°µÄ¢Ü·
.

MessageId=10106
Language=English
Resources will be aligned on %1!d! byte boundaries
.
Language=Japanese
Ø¿°½Í %d ÊÞ²Ä²ÆÌ«EÅ®ñµÜ·
.

MessageId=10107
Language=English
Writing resource
.
Language=Japanese
Ø¿°½ð«ñÅ¢Ü·
.

MessageId=20001
Language=English

Usage:  rc [options] .RC input file
Switches:
   /r    Emit .RES file (optional)
   /v    Verbose (print progress messages)
   /d    Define a symbol
   /u	 Undefine a symbol
   /fo   Rename .RES file
   /l    Default language ID in hex
   /i    Add a path for INCLUDE searches
   /x    Ignore INCLUDE environment variable
   /c	 Define a code page used by NLS conversion
   /w	 Warn on Invalid codepage in .rc (default is an error)
   /y	 Don't warn if there are duplicate control ID's
   /n    Append null's to all strings in the string tables.
Flags may be either upper or lower case

.
Language=Japanese

Usage:  rc [switches] .RC input file
Switches:
   /r    Emit .RES file (optional)
   /v    Verbose (print progress messages)
   /d    Define a symbol
   /u	 Undefine a symbol
   /fo   Rename .RES file
   /l    Default language ID in hex
   /i    Add a path for INCLUDE searches
   /x    Ignore INCLUDE environment variable
   /c    Define a CodePage used by NLS Conversion
   /w	 Warn on Invalid codepage in .rc (default is an error)
   /y	 Don't warn if there are duplicate control ID's
   /n    Append null's to all strings in the string tables.
Flags may be either upper or lower case

.

MessageId=20101
Language=English
RC terminated by user
.
Language=Japanese
RC ÍÕ°»Þ°ÌìÉæèI¹µÜµ½
.

MessageId=22103
Language=English
RC : fatal error RC2203 : unable to initialize symbol information
.
Language=Japanese
RC : fatal error RC2203 : ¼ÝÎÞÙîñðÛ¶Å«Ü¹ñ
.

MessageId=22104
Language=English
RC : fatal error RC2204 : unable to save symbol information
.
Language=Japanese
RC : fatal error RC2204 : ¼ÝÎÞÙîñðÛ¶Å«Ü¹ñ"},
.

;
;#endif _RCDLLMSG_
;

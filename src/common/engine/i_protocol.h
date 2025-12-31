/*
** i_protocol.h
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1998-2016 Marisa Heit
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Code written prior to 2026 is also licensed under:
**
** SPDX-License-Identifier: BSD-3-Clause
**
**---------------------------------------------------------------------------
**
*/

#ifndef __I_PROTOCOL_H__
#define __I_PROTOCOL_H__

#include "tarray.h"
#include "zstring.h"

uint8_t UncheckedReadInt8(uint8_t** stream);
int16_t UncheckedReadInt16(uint8_t** stream);
int32_t UncheckedReadInt32(uint8_t** stream);
int64_t UncheckedReadInt64(uint8_t** stream);
float UncheckedReadFloat(uint8_t** stream);
double UncheckedReadDouble(uint8_t** stream);
char* UncheckedReadString(uint8_t** stream);
const char* UncheckedReadStringConst(uint8_t** stream);
void UncheckedWriteInt8(uint8_t val, uint8_t** stream);
void UncheckedWriteInt16(int16_t val, uint8_t** stream);
void UncheckedWriteInt32(int32_t val, uint8_t** stream);
void UncheckedWriteInt64(int64_t val, uint8_t** stream);
void UncheckedWriteFloat(float val, uint8_t** stream);
void UncheckedWriteDouble(double val, uint8_t** stream);
void UncheckedWriteString(const char* string, uint8_t** stream);

void AdvanceStream(TArrayView<uint8_t>& stream, size_t bytes);

uint8_t ReadInt8(TArrayView<uint8_t>& stream);
int16_t ReadInt16(TArrayView<uint8_t>& stream);
int32_t ReadInt32(TArrayView<uint8_t>& stream);
int64_t ReadInt64(TArrayView<uint8_t>& stream);
float ReadFloat(TArrayView<uint8_t>& stream);
double ReadDouble(TArrayView<uint8_t>& stream);
char* ReadString(TArrayView<uint8_t>& stream);
const char* ReadStringConst(TArrayView<uint8_t>& stream);
void ReadBytes(TArrayView<uint8_t>& dst, TArrayView<uint8_t>& stream);
void WriteInt8(uint8_t val, TArrayView<uint8_t>& stream);
void WriteInt16(int16_t val, TArrayView<uint8_t>& stream);
void WriteInt32(int32_t val, TArrayView<uint8_t>& stream);
void WriteInt64(int64_t val, TArrayView<uint8_t>& stream);
void WriteFloat(float val, TArrayView<uint8_t>& stream);
void WriteDouble(double val, TArrayView<uint8_t>& stream);
void WriteString(const char* string, TArrayView<uint8_t>& stream);
void WriteBytes(const TArrayView<uint8_t>& source, TArrayView<uint8_t>& stream);
void WriteFString(FString& string, TArrayView<uint8_t>& stream);

#endif //__I_PROTOCOL_H__

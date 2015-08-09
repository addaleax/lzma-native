/**
 * lzma-native - Node.js bindings for liblzma
 * Copyright (C) 2014 Hauke Henningsen <sqrt@entless.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/
 
#include "liblzma-node.hpp"
#include <node_buffer.h>
#include <cstring>

namespace lzma {

lzma_vli FilterByName(Local<Value> v) {
	Nan::Utf8String cmpto(v);
	if (cmpto.length() == 0)
		return LZMA_VLI_UNKNOWN;
	
	struct searchEntry {
		const char* str;
		lzma_vli value;
	};
	
	static const struct searchEntry search[] = {
		{ "LZMA_FILTER_X86", LZMA_FILTER_X86 },
		{ "LZMA_FILTER_POWERPC", LZMA_FILTER_POWERPC },
		{ "LZMA_FILTER_IA64", LZMA_FILTER_IA64 },
		{ "LZMA_FILTER_ARM", LZMA_FILTER_ARM },
		{ "LZMA_FILTER_ARMTHUMB", LZMA_FILTER_ARMTHUMB },
		{ "LZMA_FILTER_SPARC", LZMA_FILTER_SPARC },
		{ "LZMA_FILTER_DELTA", LZMA_FILTER_DELTA },
		{ "LZMA_FILTERS_MAX", LZMA_FILTERS_MAX },
		{ "LZMA_FILTER_LZMA1", LZMA_FILTER_LZMA1 },
		{ "LZMA_FILTER_LZMA2", LZMA_FILTER_LZMA2 },
		{ "LZMA_VLI_UNKNOWN", LZMA_VLI_UNKNOWN }
	};
	
	for (const struct searchEntry* p = search; ; ++p) 
		if (p->value == LZMA_VLI_UNKNOWN || std::strcmp(*cmpto, p->str) == 0)
			return p->value;
}

const char* lzmaStrError(lzma_ret rv) {
	switch (rv) {
		case LZMA_OK:                return "LZMA_OK";
		case LZMA_STREAM_END:        return "LZMA_STREAM_END";
		case LZMA_NO_CHECK:          return "LZMA_NO_CHECK";
		case LZMA_UNSUPPORTED_CHECK: return "LZMA_UNSUPPORTED_CHECK";
		case LZMA_GET_CHECK:         return "LZMA_GET_CHECK";
		case LZMA_MEM_ERROR:         return "LZMA_MEM_ERROR";
		case LZMA_MEMLIMIT_ERROR:    return "LZMA_MEMLIMIT_ERROR";
		case LZMA_FORMAT_ERROR:      return "LZMA_FORMAT_ERROR";
		case LZMA_OPTIONS_ERROR:     return "LZMA_OPTIONS_ERROR";
		case LZMA_DATA_ERROR:        return "LZMA_DATA_ERROR";
		case LZMA_PROG_ERROR:        return "LZMA_PROG_ERROR";
		case LZMA_BUF_ERROR:         return "LZMA_BUF_ERROR";
		default:                     return "LZMA_UNKNOWN_ERROR";
	}
}

Local<Value> lzmaRetError(lzma_ret rv) {
	return Nan::Error(lzmaStrError(rv));
}

Local<Value> lzmaRet(lzma_ret rv) {
	if (rv != LZMA_OK && rv != LZMA_STREAM_END)
		Nan::ThrowError(lzmaRetError(rv));
	
	return Nan::New<Integer>(rv);
}

bool readBufferFromObj(Local<Value> buf_, std::vector<uint8_t>& data) {
	if (buf_.IsEmpty() || !node::Buffer::HasInstance(buf_)) {
		Nan::ThrowTypeError("Exptected Buffer as input");
		return false;
	}
	
	Local<Object> buf = Local<Object>::Cast(buf_);
	size_t len = node::Buffer::Length(buf);
	const uint8_t* ptr = reinterpret_cast<const uint8_t*>(len > 0 ? node::Buffer::Data(buf) : "");
	
	data = std::vector<uint8_t>(ptr, ptr + len);
	
	return true;
}

lzma_options_lzma parseOptionsLZMA (Local<Object> obj) {
	Nan::HandleScope();
	lzma_options_lzma r;
	
	if (obj.IsEmpty() || obj->IsUndefined())
		obj = Nan::New<Object>();
	
	r.dict_size = GetIntegerProperty(obj, "dictSize", LZMA_DICT_SIZE_DEFAULT);
	r.lp = GetIntegerProperty(obj, "lp", LZMA_LP_DEFAULT);
	r.lc = GetIntegerProperty(obj, "lc", LZMA_LC_DEFAULT);
	r.pb = GetIntegerProperty(obj, "pb", LZMA_PB_DEFAULT);
	r.mode = (lzma_mode)GetIntegerProperty(obj, "mode", (uint64_t)LZMA_MODE_FAST);
	r.nice_len = GetIntegerProperty(obj, "niceLen", 64);
	r.mf = (lzma_match_finder)GetIntegerProperty(obj, "mf", (uint64_t)LZMA_MF_HC4);
	r.depth = GetIntegerProperty(obj, "depth", 0);
	uint64_t preset_ = GetIntegerProperty(obj, "preset", UINT64_MAX);
	
	r.preset_dict = NULL;
	
	if (preset_ != UINT64_MAX) 
		lzma_lzma_preset(&r, preset_);
	
	return r;
}

Local<Value> Uint64ToNumberMaxNull(uint64_t in) {
	if (in == UINT64_MAX)
		return Nan::Null();
	else
		return Nan::New<Number>(in);
}

Local<Value> Uint64ToNumber0Null(uint64_t in) {
	if (in == 0)
		return Nan::Null();
	else
		return Nan::New<Number>(in);
}

uint64_t NumberToUint64ClampNullMax(Local<Value> in) {
	if (in->IsNull() || in->IsUndefined())
		return UINT64_MAX;
	
	Local<Number> n = Local<Number>::Cast(in);
	if (n.IsEmpty() && !in.IsEmpty()) {
		Nan::ThrowTypeError("Number required");
		return UINT64_MAX;
	}
	
	Local<Integer> integer = Local<Integer>::Cast(n);
	if (!integer.IsEmpty())
		return integer->Value();
	
	return n->Value();
}

}

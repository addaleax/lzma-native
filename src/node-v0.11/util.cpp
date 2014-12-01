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

lzma_vli FilterByName(Handle<Value> v) {
	String::Utf8Value cmpto(v);
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

int32_t lzmaRet(Isolate* isolate, lzma_ret rv) {
	if (rv != LZMA_OK && rv != LZMA_STREAM_END)
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, lzmaStrError(rv))));
	
	return rv;
}

bool readBufferFromObj(Isolate* isolate, Handle<Value> value, const uint8_t*& ptr, size_t& len) {
	ptr = NULL;
	len = 0;
	
	Local<Object> buf = Local<Object>::New(isolate, Handle<Object>::Cast(value));
	if (buf.IsEmpty() || !node::Buffer::HasInstance(buf)) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Exptected Buffer as input")));
		return false;
	}
	
	len = node::Buffer::Length(buf);
	ptr = reinterpret_cast<const uint8_t*>(len > 0 ? node::Buffer::Data(buf) : "");
	
	return true;
}

lzma_options_lzma parseOptionsLZMA (Isolate* isolate, Handle<Object> obj) {
	HandleScope scope(isolate);
	lzma_options_lzma r;
	
	if (obj.IsEmpty() || obj->IsUndefined())
		obj = Object::New(isolate);
	
	Local<Value> dict_size = obj->Get(String::NewFromUtf8(isolate, "dictSize"));
	Local<Value> lp = obj->Get(String::NewFromUtf8(isolate, "lp"));
	Local<Value> lc = obj->Get(String::NewFromUtf8(isolate, "lc"));
	Local<Value> pb = obj->Get(String::NewFromUtf8(isolate, "pb"));
	Local<Value> mode = obj->Get(String::NewFromUtf8(isolate, "mode"));
	Local<Value> nice_len = obj->Get(String::NewFromUtf8(isolate, "niceLen"));
	Local<Value> mf = obj->Get(String::NewFromUtf8(isolate, "mf"));
	Local<Value> depth = obj->Get(String::NewFromUtf8(isolate, "depth"));
	Local<Value> preset = obj->Get(String::NewFromUtf8(isolate, "preset"));
	r.dict_size = dict_size.IsEmpty() || dict_size->IsUndefined() ? LZMA_DICT_SIZE_DEFAULT : Local<Integer>::Cast(dict_size)->Value();
	r.lc = lc.IsEmpty() || lc->IsUndefined() ? LZMA_LC_DEFAULT : Local<Integer>::Cast(lc)->Value();
	r.lp = lp.IsEmpty() || lp->IsUndefined() ? LZMA_LP_DEFAULT : Local<Integer>::Cast(lp)->Value();
	r.pb = pb.IsEmpty() || pb->IsUndefined() ? LZMA_PB_DEFAULT : Local<Integer>::Cast(pb)->Value();
	r.mode = mode.IsEmpty() || mode->IsUndefined() ? LZMA_MODE_FAST : (lzma_mode)Local<Integer>::Cast(mode)->Value();
	r.nice_len = nice_len.IsEmpty() || nice_len->IsUndefined() ? 64 : Local<Integer>::Cast(nice_len)->Value();
	r.mf = mf.IsEmpty() || mf->IsUndefined() ? LZMA_MF_HC4 : (lzma_match_finder)Local<Integer>::Cast(mf)->Value();
	r.depth = mf.IsEmpty() || depth->IsUndefined() ? 0 : Local<Integer>::Cast(depth)->Value();
	r.preset_dict = NULL;
	
	if (!preset.IsEmpty() && !preset->IsUndefined()) 
		lzma_lzma_preset(&r, Local<Integer>::Cast(preset)->Value());
	
	return r;
}

Handle<Number> Uint64ToNumber(Isolate* isolate, uint64_t in) {
	if (in < UINT32_MAX)
		return Integer::NewFromUnsigned(isolate, in);
	else
		return Number::New(isolate, double(in));
}

Handle<Value> Uint64ToNumberMaxNull(Isolate* isolate, uint64_t in) {
	if (in == UINT64_MAX)
		return Null(isolate);
	else
		return Uint64ToNumber(isolate, in);
}

Handle<Value> Uint64ToNumber0Null(Isolate* isolate, uint64_t in) {
	if (in == 0)
		return Null(isolate);
	else
		return Uint64ToNumber(isolate, in);
}

uint64_t NumberToUint64ClampNullMax(Isolate* isolate, Handle<Value> in) {
	if (in->IsNull() || in->IsUndefined())
		return UINT64_MAX;
	
	Handle<Number> n = Handle<Number>::Cast(in);
	if (n.IsEmpty() && !in.IsEmpty()) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Number required")));
		return UINT64_MAX;
	}
	
	Handle<Integer> integer = Handle<Integer>::Cast(n);
	if (!integer.IsEmpty())
		return integer->Value();
	
	return n->Value();
}

}

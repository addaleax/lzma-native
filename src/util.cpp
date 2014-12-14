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
	NanUtf8String cmpto(v);
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

Handle<Value> lzmaRetError(lzma_ret rv) {
	return Exception::Error(NanNew<String>(lzmaStrError(rv)));
}

Handle<Value> lzmaRet(lzma_ret rv) {
	if (rv != LZMA_OK && rv != LZMA_STREAM_END)
		NanThrowError(lzmaRetError(rv));
	
	return NanNew<Integer>(rv);
}

bool readBufferFromObj(Handle<Value> buf_, std::vector<uint8_t>& data) {
	if (buf_.IsEmpty() || !node::Buffer::HasInstance(buf_)) {
		NanThrowTypeError("Exptected Buffer as input");
		return false;
	}
	
	Handle<Object> buf = Handle<Object>::Cast(buf_);
	size_t len = node::Buffer::Length(buf);
	const uint8_t* ptr = reinterpret_cast<const uint8_t*>(len > 0 ? node::Buffer::Data(buf) : "");
	
	data = std::vector<uint8_t>(ptr, ptr + len);
	
	return true;
}

lzma_options_lzma parseOptionsLZMA (Handle<Object> obj) {
	NanScope();
	lzma_options_lzma r;
	
	if (obj.IsEmpty() || obj->IsUndefined())
		obj = NanNew<Object>();
	
	Local<Value> dict_size = obj->Get(NanNew<String>("dictSize"));
	Local<Value> lp = obj->Get(NanNew<String>("lp"));
	Local<Value> lc = obj->Get(NanNew<String>("lc"));
	Local<Value> pb = obj->Get(NanNew<String>("pb"));
	Local<Value> mode = obj->Get(NanNew<String>("mode"));
	Local<Value> nice_len = obj->Get(NanNew<String>("niceLen"));
	Local<Value> mf = obj->Get(NanNew<String>("mf"));
	Local<Value> depth = obj->Get(NanNew<String>("depth"));
	Local<Value> preset = obj->Get(NanNew<String>("preset"));
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

Handle<Value> Uint64ToNumberMaxNull(uint64_t in) {
	if (in == UINT64_MAX)
		return NanNull();
	else
		return NanNew<Number>(in);
}

Handle<Value> Uint64ToNumber0Null(uint64_t in) {
	if (in == 0)
		return NanNull();
	else
		return NanNew<Number>(in);
}

uint64_t NumberToUint64ClampNullMax(Handle<Value> in) {
	if (in->IsNull() || in->IsUndefined())
		return UINT64_MAX;
	
	Handle<Number> n = Handle<Number>::Cast(in);
	if (n.IsEmpty() && !in.IsEmpty()) {
		NanThrowTypeError("Number required");
		return UINT64_MAX;
	}
	
	Handle<Integer> integer = Handle<Integer>::Cast(n);
	if (!integer.IsEmpty())
		return integer->Value();
	
	return n->Value();
}

}

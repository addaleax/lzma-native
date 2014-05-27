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
	String::AsciiValue cmpto(v);
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

Handle<Value> lzmaRet(lzma_ret rv) {
	if (rv != LZMA_OK && rv != LZMA_STREAM_END)
		ThrowException(Exception::Error(String::New(lzmaStrError(rv))));
	
	return Integer::New(rv);
}

bool readBufferFromObj(Handle<Value> value, const uint8_t*& ptr, size_t& len) {
	node::Buffer* buf = NULL;
	ptr = NULL;
	len = 0;
	
	Local<Object> bufarg = Local<Object>::New(Handle<Object>::Cast(value));
	if (bufarg.IsEmpty() || bufarg->IsUndefined() || bufarg->IsNull()) {
	expected_buffer:
		ThrowException(Exception::TypeError(String::New("Exptected Buffer as input")));
		return false;
	}
	
	Local<Object> parent = Local<Object>::Cast(bufarg->Get(String::NewSymbol("parent")));
	if (parent.IsEmpty() || parent->IsUndefined()) {
		// SlowBuffer
		buf = node::ObjectWrap::Unwrap<node::Buffer>(bufarg);
		
		if (!buf) 
			goto expected_buffer;
		
		if (node::Buffer::Length(buf) == 0) {
		empty_buffer:
			ptr = reinterpret_cast<const uint8_t*>("");
			return true;
		}
			
		ptr = reinterpret_cast<const uint8_t*>(node::Buffer::Data(buf));
		len = node::Buffer::Length(buf);
	} else {
		// Node.js userland Buffer
		buf = node::ObjectWrap::Unwrap<node::Buffer>(parent);
		if (!buf)
			goto expected_buffer;
		
		size_t jslen  = Local<Integer>::Cast(bufarg->Get(String::NewSymbol("length")))->Value();
		size_t offset = Local<Integer>::Cast(bufarg->Get(String::NewSymbol("offset")))->Value();
		
		size_t buflen = node::Buffer::Length(buf);
		if (jslen == 0)
			goto empty_buffer;
		
		if (jslen + offset > buflen) {
			ThrowException(Exception::TypeError(String::New("invalid buffer")));
			return false;
		}
		
		ptr = reinterpret_cast<const uint8_t*>(node::Buffer::Data(buf)) + offset;
		len = jslen;
	}
	
	return true;
}

lzma_options_lzma parseOptionsLZMA (Handle<Object> obj) {
	HandleScope scope;
	lzma_options_lzma r;
	
	if (obj.IsEmpty() || obj->IsUndefined())
		obj = Object::New();
	
	Local<Value> dict_size = obj->Get(String::NewSymbol("dictSize"));
	Local<Value> lp = obj->Get(String::NewSymbol("lp"));
	Local<Value> lc = obj->Get(String::NewSymbol("lc"));
	Local<Value> pb = obj->Get(String::NewSymbol("pb"));
	Local<Value> mode = obj->Get(String::NewSymbol("mode"));
	Local<Value> nice_len = obj->Get(String::NewSymbol("niceLen"));
	Local<Value> mf = obj->Get(String::NewSymbol("mf"));
	Local<Value> depth = obj->Get(String::NewSymbol("depth"));
	Local<Value> preset = obj->Get(String::NewSymbol("preset"));
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

}

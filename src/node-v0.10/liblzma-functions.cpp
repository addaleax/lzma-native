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

namespace lzma {

Handle<Value> lzmaVersionNumber(const Arguments& args) {
	HandleScope scope; 
	return scope.Close(Integer::NewFromUnsigned(lzma_version_number()));
}

Handle<Value> lzmaVersionString(const Arguments& args) {
	HandleScope scope;
	return scope.Close(String::New(lzma_version_string()));
}

Handle<Value> lzmaCheckIsSupported(const Arguments& args) {
	HandleScope scope;
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	return scope.Close(Boolean::New(lzma_check_is_supported((lzma_check) arg->Value())));
}

Handle<Value> lzmaCheckSize(const Arguments& args) {
	HandleScope scope;
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	return scope.Close(Integer::NewFromUnsigned(lzma_check_size((lzma_check) arg->Value())));
}

Handle<Value> lzmaFilterEncoderIsSupported(const Arguments& args) {
	HandleScope scope;
	uint64_t arg = FilterByName(args[0]);
	
	return scope.Close(Boolean::New(lzma_filter_encoder_is_supported(arg)));
}

Handle<Value> lzmaFilterDecoderIsSupported(const Arguments& args) {
	HandleScope scope;
	uint64_t arg = FilterByName(args[0]);
	
	return scope.Close(Boolean::New(lzma_filter_decoder_is_supported(arg)));
}

Handle<Value> lzmaMfIsSupported(const Arguments& args) {
	HandleScope scope;
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	return scope.Close(Boolean::New(lzma_mf_is_supported((lzma_match_finder) arg->Value())));
}

Handle<Value> lzmaModeIsSupported(const Arguments& args) {
	HandleScope scope;
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	return scope.Close(Boolean::New(lzma_mode_is_supported((lzma_mode) arg->Value())));
}

Handle<Value> lzmaEasyEncoderMemusage(const Arguments& args) {
	HandleScope scope;
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	return scope.Close(Uint64ToNumberMaxNull(lzma_easy_encoder_memusage(arg->Value())));
}

Handle<Value> lzmaEasyDecoderMemusage(const Arguments& args) {
	HandleScope scope;
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	return scope.Close(Uint64ToNumberMaxNull(lzma_easy_decoder_memusage(arg->Value())));
}

Handle<Value> lzmaCRC32(const Arguments& args) {
	HandleScope scope;
	Local<Integer> arg = Local<Integer>::Cast(args[1]);
	
	if (arg.IsEmpty() || args[1]->IsUndefined())
		arg = Integer::New(0);
	
	std::vector<uint8_t> data;
	
	if (!readBufferFromObj(args[0], data)) {
		ThrowException(Exception::TypeError(String::New("CRC32 expects Buffer as input")));
		return scope.Close(Undefined());
	}
	
	return scope.Close(Integer::NewFromUnsigned(lzma_crc32(data.data(), data.size(), arg->Value())));
}

Handle<Value> lzmaRawEncoderMemusage(const Arguments& args) {
	HandleScope scope;
	Local<Array> arg = Local<Array>::Cast(args[0]);
	if (arg.IsEmpty() || args[0]->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("rawEncoderMemusage requires filter array as arguments")));
		return scope.Close(Undefined());
	}
	
	const FilterArray fa(arg);
	if (!fa.ok()) 
		return scope.Close(Undefined());
	
	return scope.Close(Uint64ToNumberMaxNull(lzma_raw_encoder_memusage(fa.array())));
}

Handle<Value> lzmaRawDecoderMemusage(const Arguments& args) {
	HandleScope scope;
	Local<Array> arg = Local<Array>::Cast(args[0]);
	if (arg.IsEmpty() || args[0]->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("rawdecoderMemusage requires filter array as arguments")));
		return scope.Close(Undefined());
	}
	
	const FilterArray fa(arg);
	if (!fa.ok()) 
		return scope.Close(Undefined());
	
	return scope.Close(Uint64ToNumberMaxNull(lzma_raw_decoder_memusage(fa.array())));
}

}

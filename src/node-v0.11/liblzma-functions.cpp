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

void lzmaVersionNumber(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	args.GetReturnValue().Set(Integer::NewFromUnsigned(isolate, lzma_version_number()));
	return;
}

void lzmaVersionString(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	args.GetReturnValue().Set(String::NewFromUtf8(isolate, lzma_version_string()));
	return;
}

void lzmaCheckIsSupported(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	args.GetReturnValue().Set(Boolean::New(isolate, lzma_check_is_supported((lzma_check) arg->Value())));
	return;
}

void lzmaCheckSize(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	args.GetReturnValue().Set(Integer::NewFromUnsigned(isolate, lzma_check_size((lzma_check) arg->Value())));
	return;
}

void lzmaFilterEncoderIsSupported(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	uint64_t arg = FilterByName(args[0]);
	
	args.GetReturnValue().Set(Boolean::New(isolate, lzma_filter_encoder_is_supported(arg)));
	return;
}

void lzmaFilterDecoderIsSupported(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	uint64_t arg = FilterByName(args[0]);
	
	args.GetReturnValue().Set(Boolean::New(isolate, lzma_filter_decoder_is_supported(arg)));
	return;
}

void lzmaMfIsSupported(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	args.GetReturnValue().Set(Boolean::New(isolate, lzma_mf_is_supported((lzma_match_finder) arg->Value())));
	return;
}

void lzmaModeIsSupported(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	args.GetReturnValue().Set(Boolean::New(isolate, lzma_mode_is_supported((lzma_mode) arg->Value())));
	return;
}

void lzmaEasyEncoderMemusage(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	args.GetReturnValue().Set(Uint64ToNumberMaxNull(isolate, lzma_easy_encoder_memusage(arg->Value())));
	return;
}

void lzmaEasyDecoderMemusage(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	args.GetReturnValue().Set(Uint64ToNumberMaxNull(isolate, lzma_easy_decoder_memusage(arg->Value())));
	return;
}

void lzmaCRC32(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	Local<Integer> arg = Local<Integer>::Cast(args[1]);
	
	if (arg.IsEmpty() || args[1]->IsUndefined())
		arg = Integer::New(isolate, 0);
	
	const uint8_t* data;
	size_t datalen;
	
	if (!readBufferFromObj(isolate, args[0], data, datalen)) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "CRC32 expects Buffer as input")));
		args.GetReturnValue().SetUndefined();
		return;
	}
	
	args.GetReturnValue().Set(Integer::NewFromUnsigned(isolate, lzma_crc32(data, datalen, arg->Value())));
	return;
}

void lzmaRawEncoderMemusage(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	Local<Array> arg = Local<Array>::Cast(args[0]);
	if (arg.IsEmpty() || args[0]->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "rawEncoderMemusage requires filter array as arguments")));
		args.GetReturnValue().SetUndefined();
		return;
	}
	
	const FilterArray fa(arg);
	if (!fa.ok())
		args.GetReturnValue().SetUndefined();
	else
		args.GetReturnValue().Set(Uint64ToNumberMaxNull(isolate, lzma_raw_encoder_memusage(fa.array())));
}

void lzmaRawDecoderMemusage(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	Local<Array> arg = Local<Array>::Cast(args[0]);
	if (arg.IsEmpty() || args[0]->IsUndefined()) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "rawdecoderMemusage requires filter array as arguments")));
		args.GetReturnValue().SetUndefined();
		return;
	}
	
	const FilterArray fa(arg);
	if (!fa.ok())
		args.GetReturnValue().SetUndefined();
	else
		args.GetReturnValue().Set(Uint64ToNumberMaxNull(isolate, lzma_raw_decoder_memusage(fa.array())));
}

}

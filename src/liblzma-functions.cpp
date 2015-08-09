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

NAN_METHOD(lzmaVersionNumber) {
	NanScope(); 
	NanReturnValue(NanNew<Uint32>(lzma_version_number()));
}

NAN_METHOD(lzmaVersionString) {
	NanScope();
	NanReturnValue(NanNew<String>(lzma_version_string()));
}

NAN_METHOD(lzmaCheckIsSupported) {
	NanScope();
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	NanReturnValue(NanNew<Boolean>(lzma_check_is_supported((lzma_check) arg->Value())));
}

NAN_METHOD(lzmaCheckSize) {
	NanScope();
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	NanReturnValue(NanNew<Uint32>(lzma_check_size((lzma_check) arg->Value())));
}

NAN_METHOD(lzmaFilterEncoderIsSupported) {
	NanScope();
	uint64_t arg = FilterByName(args[0]);
	
	NanReturnValue(NanNew<Boolean>(lzma_filter_encoder_is_supported(arg)));
}

NAN_METHOD(lzmaFilterDecoderIsSupported) {
	NanScope();
	uint64_t arg = FilterByName(args[0]);
	
	NanReturnValue(NanNew<Boolean>(lzma_filter_decoder_is_supported(arg)));
}

NAN_METHOD(lzmaMfIsSupported) {
	NanScope();
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	NanReturnValue(NanNew<Boolean>(lzma_mf_is_supported((lzma_match_finder) arg->Value())));
}

NAN_METHOD(lzmaModeIsSupported) {
	NanScope();
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	NanReturnValue(NanNew<Boolean>(lzma_mode_is_supported((lzma_mode) arg->Value())));
}

NAN_METHOD(lzmaEasyEncoderMemusage) {
	NanScope();
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	NanReturnValue(Uint64ToNumberMaxNull(lzma_easy_encoder_memusage(arg->Value())));
}

NAN_METHOD(lzmaEasyDecoderMemusage) {
	NanScope();
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	NanReturnValue(Uint64ToNumberMaxNull(lzma_easy_decoder_memusage(arg->Value())));
}

NAN_METHOD(lzmaCRC32) {
	NanScope();
	Local<Integer> arg = Local<Integer>::Cast(args[1]);
	
	if (arg.IsEmpty() || args[1]->IsUndefined())
		arg = NanNew<Integer>(0);
	
	std::vector<uint8_t> data;
	
	if (!readBufferFromObj(args[0], data)) {
		NanThrowTypeError("CRC32 expects Buffer as input");
		NanReturnUndefined();
	}
	
	NanReturnValue(NanNew<Uint32>(lzma_crc32(data.data(), data.size(), arg->Value())));
}

NAN_METHOD(lzmaRawEncoderMemusage) {
	NanScope();
	Local<Array> arg = Local<Array>::Cast(args[0]);
	
	const FilterArray filters(arg);
	if (!filters.ok()) {
		NanThrowTypeError("rawEncoderMemusage requires filter array as arguments");
		NanReturnUndefined();
	}
	
	NanReturnValue(Uint64ToNumberMaxNull(lzma_raw_encoder_memusage(filters.array())));
}

NAN_METHOD(lzmaRawDecoderMemusage) {
	NanScope();
	Local<Array> arg = Local<Array>::Cast(args[0]);
	
	const FilterArray filters(arg);
	if (!filters.ok()) {
		NanThrowTypeError("rawEncoderMemusage requires filter array as arguments");
		NanReturnUndefined();
	}
	
	NanReturnValue(Uint64ToNumberMaxNull(lzma_raw_decoder_memusage(filters.array())));
}

}

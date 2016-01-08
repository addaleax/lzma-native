/**
 * lzma-native - Node.js bindings for liblzma
 * Copyright (C) 2014-2015 Anna Henningsen <sqrt@entless.org>
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
  info.GetReturnValue().Set(lzma_version_number());
}

NAN_METHOD(lzmaVersionString) {
  info.GetReturnValue().Set(Nan::New<String>(lzma_version_string()).ToLocalChecked());
}

NAN_METHOD(lzmaCheckIsSupported) {
  Local<Integer> arg = Local<Integer>::Cast(info[0]);
  
  info.GetReturnValue().Set((bool)lzma_check_is_supported((lzma_check) arg->Value()));
}

NAN_METHOD(lzmaCheckSize) {
  Local<Integer> arg = Local<Integer>::Cast(info[0]);
  
  info.GetReturnValue().Set(Nan::New<Number>(lzma_check_size((lzma_check) arg->Value())));
}

NAN_METHOD(lzmaFilterEncoderIsSupported) {
  uint64_t arg = FilterByName(info[0]);
  
  info.GetReturnValue().Set((bool)lzma_filter_encoder_is_supported(arg));
}

NAN_METHOD(lzmaFilterDecoderIsSupported) {
  uint64_t arg = FilterByName(info[0]);
  
  info.GetReturnValue().Set((bool)lzma_filter_decoder_is_supported(arg));
}

NAN_METHOD(lzmaMfIsSupported) {
  Local<Integer> arg = Local<Integer>::Cast(info[0]);
  
  info.GetReturnValue().Set((bool)lzma_mf_is_supported((lzma_match_finder) arg->Value()));
}

NAN_METHOD(lzmaModeIsSupported) {
  Local<Integer> arg = Local<Integer>::Cast(info[0]);
  
  info.GetReturnValue().Set((bool)lzma_mode_is_supported((lzma_mode) arg->Value()));
}

NAN_METHOD(lzmaEasyEncoderMemusage) {
  Local<Integer> arg = Local<Integer>::Cast(info[0]);
  
  info.GetReturnValue().Set(Uint64ToNumberMaxNull(lzma_easy_encoder_memusage(arg->Value())));
}

NAN_METHOD(lzmaEasyDecoderMemusage) {
  Local<Integer> arg = Local<Integer>::Cast(info[0]);
  
  info.GetReturnValue().Set(Uint64ToNumberMaxNull(lzma_easy_decoder_memusage(arg->Value())));
}

NAN_METHOD(lzmaCRC32) {
  Local<Integer> arg = Local<Integer>::Cast(info[1]);
  
  if (arg.IsEmpty() || info[1]->IsUndefined())
    arg = Nan::New<Integer>(0);
  
  std::vector<uint8_t> data;
  
  if (!readBufferFromObj(info[0], data)) {
    Nan::ThrowTypeError("CRC32 expects Buffer as input");
    info.GetReturnValue().SetUndefined();
    return;
  }
  
  info.GetReturnValue().Set(Nan::New<Number>(lzma_crc32(data.data(), data.size(), arg->Value())));
}

NAN_METHOD(lzmaRawEncoderMemusage) {
  Local<Array> arg = Local<Array>::Cast(info[0]);
  
  const FilterArray filters(arg);
  if (!filters.ok()) {
    Nan::ThrowTypeError("rawEncoderMemusage requires filter array as arguments");
    info.GetReturnValue().SetUndefined();
    return;
  }
  
  info.GetReturnValue().Set(Uint64ToNumberMaxNull(lzma_raw_encoder_memusage(filters.array())));
}

NAN_METHOD(lzmaRawDecoderMemusage) {
  Local<Array> arg = Local<Array>::Cast(info[0]);
  
  const FilterArray filters(arg);
  if (!filters.ok()) {
    Nan::ThrowTypeError("rawDecoderMemusage requires filter array as arguments");
    info.GetReturnValue().SetUndefined();
    return;
  }
  
  info.GetReturnValue().Set(Uint64ToNumberMaxNull(lzma_raw_decoder_memusage(filters.array())));
}

}

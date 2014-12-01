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

void moduleInit(Handle<Object> exports, Handle<Object> module) {
	Isolate* isolate = Isolate::GetCurrent();
	LZMAStream::Init(exports, module);
	
	exports->Set(String::NewFromUtf8(isolate, "versionNumber"),            FunctionTemplate::New(isolate, lzmaVersionNumber)->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "versionString"),            FunctionTemplate::New(isolate, lzmaVersionString)->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "checkIsSupported"),         FunctionTemplate::New(isolate, lzmaCheckIsSupported)->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "checkSize"),                FunctionTemplate::New(isolate, lzmaCheckSize)->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "crc32_"),                   FunctionTemplate::New(isolate, lzmaCRC32)->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "filterEncoderIsSupported"), FunctionTemplate::New(isolate, lzmaFilterEncoderIsSupported)->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "filterDecoderIsSupported"), FunctionTemplate::New(isolate, lzmaFilterDecoderIsSupported)->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "rawEncoderMemusage"),       FunctionTemplate::New(isolate, lzmaRawEncoderMemusage)->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "rawDecoderMemusage"),       FunctionTemplate::New(isolate, lzmaRawDecoderMemusage)->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "mfIsSupported"),            FunctionTemplate::New(isolate, lzmaMfIsSupported)->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "modeIsSupported"),          FunctionTemplate::New(isolate, lzmaModeIsSupported)->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "easyEncoderMemusage"),      FunctionTemplate::New(isolate, lzmaEasyEncoderMemusage)->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "easyDecoderMemusage"),      FunctionTemplate::New(isolate, lzmaEasyDecoderMemusage)->GetFunction());
	
	// enum lzma_ret
	exports->Set(String::NewFromUtf8(isolate, "OK"),                Integer::NewFromUnsigned(isolate, LZMA_OK));
	exports->Set(String::NewFromUtf8(isolate, "STREAM_END"),        Integer::NewFromUnsigned(isolate, LZMA_STREAM_END));
	exports->Set(String::NewFromUtf8(isolate, "NO_CHECK"),          Integer::NewFromUnsigned(isolate, LZMA_NO_CHECK));
	exports->Set(String::NewFromUtf8(isolate, "UNSUPPORTED_CHECK"), Integer::NewFromUnsigned(isolate, LZMA_UNSUPPORTED_CHECK));
	exports->Set(String::NewFromUtf8(isolate, "GET_CHECK"),         Integer::NewFromUnsigned(isolate, LZMA_GET_CHECK));
	exports->Set(String::NewFromUtf8(isolate, "MEM_ERROR"),         Integer::NewFromUnsigned(isolate, LZMA_MEM_ERROR));
	exports->Set(String::NewFromUtf8(isolate, "MEMLIMIT_ERROR"),    Integer::NewFromUnsigned(isolate, LZMA_MEMLIMIT_ERROR));
	exports->Set(String::NewFromUtf8(isolate, "FORMAT_ERROR"),      Integer::NewFromUnsigned(isolate, LZMA_FORMAT_ERROR));
	exports->Set(String::NewFromUtf8(isolate, "OPTIONS_ERROR"),     Integer::NewFromUnsigned(isolate, LZMA_OPTIONS_ERROR));
	exports->Set(String::NewFromUtf8(isolate, "DATA_ERROR"),        Integer::NewFromUnsigned(isolate, LZMA_DATA_ERROR));
	exports->Set(String::NewFromUtf8(isolate, "BUF_ERROR"),         Integer::NewFromUnsigned(isolate, LZMA_BUF_ERROR));
	exports->Set(String::NewFromUtf8(isolate, "PROG_ERROR"),        Integer::NewFromUnsigned(isolate, LZMA_PROG_ERROR));
	
	// enum lzma_action
	exports->Set(String::NewFromUtf8(isolate, "RUN"),        Integer::NewFromUnsigned(isolate, LZMA_RUN));
	exports->Set(String::NewFromUtf8(isolate, "SYNC_FLUSH"), Integer::NewFromUnsigned(isolate, LZMA_SYNC_FLUSH));
	exports->Set(String::NewFromUtf8(isolate, "FULL_FLUSH"), Integer::NewFromUnsigned(isolate, LZMA_FULL_FLUSH));
	exports->Set(String::NewFromUtf8(isolate, "FINISH"),     Integer::NewFromUnsigned(isolate, LZMA_FINISH));
	
	// enum lzma_check
	exports->Set(String::NewFromUtf8(isolate, "CHECK_NONE"),   Integer::NewFromUnsigned(isolate, LZMA_CHECK_NONE));
	exports->Set(String::NewFromUtf8(isolate, "CHECK_CRC32"),  Integer::NewFromUnsigned(isolate, LZMA_CHECK_CRC32));
	exports->Set(String::NewFromUtf8(isolate, "CHECK_CRC64"),  Integer::NewFromUnsigned(isolate, LZMA_CHECK_CRC64));
	exports->Set(String::NewFromUtf8(isolate, "CHECK_SHA256"), Integer::NewFromUnsigned(isolate, LZMA_CHECK_SHA256));
	
	// lzma_match_finder
	exports->Set(String::NewFromUtf8(isolate, "MF_HC3"), Integer::NewFromUnsigned(isolate, LZMA_MF_HC3));
	exports->Set(String::NewFromUtf8(isolate, "MF_HC4"), Integer::NewFromUnsigned(isolate, LZMA_MF_HC4));
	exports->Set(String::NewFromUtf8(isolate, "MF_BT2"), Integer::NewFromUnsigned(isolate, LZMA_MF_BT2));
	exports->Set(String::NewFromUtf8(isolate, "MF_BT3"), Integer::NewFromUnsigned(isolate, LZMA_MF_BT3));
	exports->Set(String::NewFromUtf8(isolate, "MF_BT4"), Integer::NewFromUnsigned(isolate, LZMA_MF_BT4));
	
	// lzma_mode
	exports->Set(String::NewFromUtf8(isolate, "MODE_FAST"),   Integer::NewFromUnsigned(isolate, LZMA_MODE_FAST));
	exports->Set(String::NewFromUtf8(isolate, "MODE_NORMAL"), Integer::NewFromUnsigned(isolate, LZMA_MODE_NORMAL));
	
	// defines
	exports->Set(String::NewFromUtf8(isolate, "FILTER_X86"),               String::NewFromUtf8(isolate, "LZMA_FILTER_X86"));
	exports->Set(String::NewFromUtf8(isolate, "FILTER_POWERPC"),           String::NewFromUtf8(isolate, "LZMA_FILTER_POWERPC"));
	exports->Set(String::NewFromUtf8(isolate, "FILTER_IA64"),              String::NewFromUtf8(isolate, "LZMA_FILTER_IA64"));
	exports->Set(String::NewFromUtf8(isolate, "FILTER_ARM"),               String::NewFromUtf8(isolate, "LZMA_FILTER_ARM"));
	exports->Set(String::NewFromUtf8(isolate, "FILTER_ARMTHUMB"),          String::NewFromUtf8(isolate, "LZMA_FILTER_ARMTHUMB"));
	exports->Set(String::NewFromUtf8(isolate, "FILTER_SPARC"),             String::NewFromUtf8(isolate, "LZMA_FILTER_SPARC"));
	exports->Set(String::NewFromUtf8(isolate, "FILTER_DELTA"),             String::NewFromUtf8(isolate, "LZMA_FILTER_DELTA"));
	exports->Set(String::NewFromUtf8(isolate, "FILTERS_MAX"),              String::NewFromUtf8(isolate, "LZMA_FILTERS_MAX"));
	exports->Set(String::NewFromUtf8(isolate, "FILTER_LZMA1"),             String::NewFromUtf8(isolate, "LZMA_FILTER_LZMA1"));
	exports->Set(String::NewFromUtf8(isolate, "FILTER_LZMA2"),             String::NewFromUtf8(isolate, "LZMA_FILTER_LZMA2"));
	exports->Set(String::NewFromUtf8(isolate, "VLI_UNKNOWN"),              String::NewFromUtf8(isolate, "LZMA_VLI_UNKNOWN"));
	
	exports->Set(String::NewFromUtf8(isolate, "VLI_BYTES_MAX"),            Integer::NewFromUnsigned(isolate, LZMA_VLI_BYTES_MAX));
	exports->Set(String::NewFromUtf8(isolate, "CHECK_ID_MAX"),             Integer::NewFromUnsigned(isolate, LZMA_CHECK_ID_MAX));
	exports->Set(String::NewFromUtf8(isolate, "CHECK_SIZE_MAX"),           Integer::NewFromUnsigned(isolate, LZMA_CHECK_SIZE_MAX));
	exports->Set(String::NewFromUtf8(isolate, "PRESET_DEFAULT"),           Integer::NewFromUnsigned(isolate, LZMA_PRESET_DEFAULT));
	exports->Set(String::NewFromUtf8(isolate, "PRESET_LEVEL_MASK"),        Integer::NewFromUnsigned(isolate, LZMA_PRESET_LEVEL_MASK));
	exports->Set(String::NewFromUtf8(isolate, "PRESET_EXTREME"),           Integer::NewFromUnsigned(isolate, LZMA_PRESET_EXTREME));
	exports->Set(String::NewFromUtf8(isolate, "TELL_NO_CHECK"),            Integer::NewFromUnsigned(isolate, LZMA_TELL_NO_CHECK));
	exports->Set(String::NewFromUtf8(isolate, "TELL_UNSUPPORTED_CHECK"),   Integer::NewFromUnsigned(isolate, LZMA_TELL_UNSUPPORTED_CHECK));
	exports->Set(String::NewFromUtf8(isolate, "TELL_ANY_CHECK"),           Integer::NewFromUnsigned(isolate, LZMA_TELL_ANY_CHECK));
	exports->Set(String::NewFromUtf8(isolate, "CONCATENATED"),             Integer::NewFromUnsigned(isolate, LZMA_CONCATENATED));
	exports->Set(String::NewFromUtf8(isolate, "STREAM_HEADER_SIZE"),       Integer::NewFromUnsigned(isolate, LZMA_STREAM_HEADER_SIZE));
	exports->Set(String::NewFromUtf8(isolate, "VERSION_MAJOR"),            Integer::NewFromUnsigned(isolate, LZMA_VERSION_MAJOR));
	exports->Set(String::NewFromUtf8(isolate, "VERSION_MINOR"),            Integer::NewFromUnsigned(isolate, LZMA_VERSION_MINOR));
	exports->Set(String::NewFromUtf8(isolate, "VERSION_PATCH"),            Integer::NewFromUnsigned(isolate, LZMA_VERSION_PATCH));
	exports->Set(String::NewFromUtf8(isolate, "VERSION_STABILITY"),        Integer::NewFromUnsigned(isolate, LZMA_VERSION_STABILITY));
	exports->Set(String::NewFromUtf8(isolate, "VERSION_STABILITY_ALPHA"),  Integer::NewFromUnsigned(isolate, LZMA_VERSION_STABILITY_ALPHA));
	exports->Set(String::NewFromUtf8(isolate, "VERSION_STABILITY_BETA"),   Integer::NewFromUnsigned(isolate, LZMA_VERSION_STABILITY_BETA));
	exports->Set(String::NewFromUtf8(isolate, "VERSION_STABILITY_STABLE"), Integer::NewFromUnsigned(isolate, LZMA_VERSION_STABILITY_STABLE));
	exports->Set(String::NewFromUtf8(isolate, "VERSION"),                  Integer::NewFromUnsigned(isolate, LZMA_VERSION));
	exports->Set(String::NewFromUtf8(isolate, "VERSION_STRING"),           String::NewFromUtf8(isolate, LZMA_VERSION_STRING));
	
	exports->Set(String::NewFromUtf8(isolate, "asyncCodeAvailable"),       Boolean::New(isolate, true));
}

}

NODE_MODULE(lzma_native, lzma::moduleInit)


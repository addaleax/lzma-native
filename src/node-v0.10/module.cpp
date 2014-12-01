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
	LZMAStream::Init(exports, module);
	
	exports->Set(String::NewSymbol("versionNumber"),            FunctionTemplate::New(lzmaVersionNumber)->GetFunction());
	exports->Set(String::NewSymbol("versionString"),            FunctionTemplate::New(lzmaVersionString)->GetFunction());
	exports->Set(String::NewSymbol("checkIsSupported"),         FunctionTemplate::New(lzmaCheckIsSupported)->GetFunction());
	exports->Set(String::NewSymbol("checkSize"),                FunctionTemplate::New(lzmaCheckSize)->GetFunction());
	exports->Set(String::NewSymbol("crc32_"),                    FunctionTemplate::New(lzmaCRC32)->GetFunction());
	exports->Set(String::NewSymbol("filterEncoderIsSupported"), FunctionTemplate::New(lzmaFilterEncoderIsSupported)->GetFunction());
	exports->Set(String::NewSymbol("filterDecoderIsSupported"), FunctionTemplate::New(lzmaFilterDecoderIsSupported)->GetFunction());
	exports->Set(String::NewSymbol("rawEncoderMemusage"),       FunctionTemplate::New(lzmaRawEncoderMemusage)->GetFunction());
	exports->Set(String::NewSymbol("rawDecoderMemusage"),       FunctionTemplate::New(lzmaRawDecoderMemusage)->GetFunction());
	exports->Set(String::NewSymbol("mfIsSupported"),            FunctionTemplate::New(lzmaMfIsSupported)->GetFunction());
	exports->Set(String::NewSymbol("modeIsSupported"),          FunctionTemplate::New(lzmaModeIsSupported)->GetFunction());
	exports->Set(String::NewSymbol("easyEncoderMemusage"),      FunctionTemplate::New(lzmaEasyEncoderMemusage)->GetFunction());
	exports->Set(String::NewSymbol("easyDecoderMemusage"),      FunctionTemplate::New(lzmaEasyDecoderMemusage)->GetFunction());
	
	// enum lzma_ret
	exports->Set(String::NewSymbol("OK"),                Integer::NewFromUnsigned(LZMA_OK));
	exports->Set(String::NewSymbol("STREAM_END"),        Integer::NewFromUnsigned(LZMA_STREAM_END));
	exports->Set(String::NewSymbol("NO_CHECK"),          Integer::NewFromUnsigned(LZMA_NO_CHECK));
	exports->Set(String::NewSymbol("UNSUPPORTED_CHECK"), Integer::NewFromUnsigned(LZMA_UNSUPPORTED_CHECK));
	exports->Set(String::NewSymbol("GET_CHECK"),         Integer::NewFromUnsigned(LZMA_GET_CHECK));
	exports->Set(String::NewSymbol("MEM_ERROR"),         Integer::NewFromUnsigned(LZMA_MEM_ERROR));
	exports->Set(String::NewSymbol("MEMLIMIT_ERROR"),    Integer::NewFromUnsigned(LZMA_MEMLIMIT_ERROR));
	exports->Set(String::NewSymbol("FORMAT_ERROR"),      Integer::NewFromUnsigned(LZMA_FORMAT_ERROR));
	exports->Set(String::NewSymbol("OPTIONS_ERROR"),     Integer::NewFromUnsigned(LZMA_OPTIONS_ERROR));
	exports->Set(String::NewSymbol("DATA_ERROR"),        Integer::NewFromUnsigned(LZMA_DATA_ERROR));
	exports->Set(String::NewSymbol("BUF_ERROR"),         Integer::NewFromUnsigned(LZMA_BUF_ERROR));
	exports->Set(String::NewSymbol("PROG_ERROR"),        Integer::NewFromUnsigned(LZMA_PROG_ERROR));
	
	// enum lzma_action
	exports->Set(String::NewSymbol("RUN"),        Integer::NewFromUnsigned(LZMA_RUN));
	exports->Set(String::NewSymbol("SYNC_FLUSH"), Integer::NewFromUnsigned(LZMA_SYNC_FLUSH));
	exports->Set(String::NewSymbol("FULL_FLUSH"), Integer::NewFromUnsigned(LZMA_FULL_FLUSH));
	exports->Set(String::NewSymbol("FINISH"),     Integer::NewFromUnsigned(LZMA_FINISH));
	
	// enum lzma_check
	exports->Set(String::NewSymbol("CHECK_NONE"),   Integer::NewFromUnsigned(LZMA_CHECK_NONE));
	exports->Set(String::NewSymbol("CHECK_CRC32"),  Integer::NewFromUnsigned(LZMA_CHECK_CRC32));
	exports->Set(String::NewSymbol("CHECK_CRC64"),  Integer::NewFromUnsigned(LZMA_CHECK_CRC64));
	exports->Set(String::NewSymbol("CHECK_SHA256"), Integer::NewFromUnsigned(LZMA_CHECK_SHA256));
	
	// lzma_match_finder
	exports->Set(String::NewSymbol("MF_HC3"), Integer::NewFromUnsigned(LZMA_MF_HC3));
	exports->Set(String::NewSymbol("MF_HC4"), Integer::NewFromUnsigned(LZMA_MF_HC4));
	exports->Set(String::NewSymbol("MF_BT2"), Integer::NewFromUnsigned(LZMA_MF_BT2));
	exports->Set(String::NewSymbol("MF_BT3"), Integer::NewFromUnsigned(LZMA_MF_BT3));
	exports->Set(String::NewSymbol("MF_BT4"), Integer::NewFromUnsigned(LZMA_MF_BT4));
	
	// lzma_mode
	exports->Set(String::NewSymbol("MODE_FAST"),   Integer::NewFromUnsigned(LZMA_MODE_FAST));
	exports->Set(String::NewSymbol("MODE_NORMAL"), Integer::NewFromUnsigned(LZMA_MODE_NORMAL));
	
	// defines
	exports->Set(String::NewSymbol("FILTER_X86"),               String::New("LZMA_FILTER_X86"));
	exports->Set(String::NewSymbol("FILTER_POWERPC"),           String::New("LZMA_FILTER_POWERPC"));
	exports->Set(String::NewSymbol("FILTER_IA64"),              String::New("LZMA_FILTER_IA64"));
	exports->Set(String::NewSymbol("FILTER_ARM"),               String::New("LZMA_FILTER_ARM"));
	exports->Set(String::NewSymbol("FILTER_ARMTHUMB"),          String::New("LZMA_FILTER_ARMTHUMB"));
	exports->Set(String::NewSymbol("FILTER_SPARC"),             String::New("LZMA_FILTER_SPARC"));
	exports->Set(String::NewSymbol("FILTER_DELTA"),             String::New("LZMA_FILTER_DELTA"));
	exports->Set(String::NewSymbol("FILTERS_MAX"),              String::New("LZMA_FILTERS_MAX"));
	exports->Set(String::NewSymbol("FILTER_LZMA1"),             String::New("LZMA_FILTER_LZMA1"));
	exports->Set(String::NewSymbol("FILTER_LZMA2"),             String::New("LZMA_FILTER_LZMA2"));
	exports->Set(String::NewSymbol("VLI_UNKNOWN"),              String::New("LZMA_VLI_UNKNOWN"));
	
	exports->Set(String::NewSymbol("VLI_BYTES_MAX"),            Integer::NewFromUnsigned(LZMA_VLI_BYTES_MAX));
	exports->Set(String::NewSymbol("CHECK_ID_MAX"),             Integer::NewFromUnsigned(LZMA_CHECK_ID_MAX));
	exports->Set(String::NewSymbol("CHECK_SIZE_MAX"),           Integer::NewFromUnsigned(LZMA_CHECK_SIZE_MAX));
	exports->Set(String::NewSymbol("PRESET_DEFAULT"),           Integer::NewFromUnsigned(LZMA_PRESET_DEFAULT));
	exports->Set(String::NewSymbol("PRESET_LEVEL_MASK"),        Integer::NewFromUnsigned(LZMA_PRESET_LEVEL_MASK));
	exports->Set(String::NewSymbol("PRESET_EXTREME"),           Integer::NewFromUnsigned(LZMA_PRESET_EXTREME));
	exports->Set(String::NewSymbol("TELL_NO_CHECK"),            Integer::NewFromUnsigned(LZMA_TELL_NO_CHECK));
	exports->Set(String::NewSymbol("TELL_UNSUPPORTED_CHECK"),   Integer::NewFromUnsigned(LZMA_TELL_UNSUPPORTED_CHECK));
	exports->Set(String::NewSymbol("TELL_ANY_CHECK"),           Integer::NewFromUnsigned(LZMA_TELL_ANY_CHECK));
	exports->Set(String::NewSymbol("CONCATENATED"),             Integer::NewFromUnsigned(LZMA_CONCATENATED));
	exports->Set(String::NewSymbol("STREAM_HEADER_SIZE"),       Integer::NewFromUnsigned(LZMA_STREAM_HEADER_SIZE));
	exports->Set(String::NewSymbol("VERSION_MAJOR"),            Integer::NewFromUnsigned(LZMA_VERSION_MAJOR));
	exports->Set(String::NewSymbol("VERSION_MINOR"),            Integer::NewFromUnsigned(LZMA_VERSION_MINOR));
	exports->Set(String::NewSymbol("VERSION_PATCH"),            Integer::NewFromUnsigned(LZMA_VERSION_PATCH));
	exports->Set(String::NewSymbol("VERSION_STABILITY"),        Integer::NewFromUnsigned(LZMA_VERSION_STABILITY));
	exports->Set(String::NewSymbol("VERSION_STABILITY_ALPHA"),  Integer::NewFromUnsigned(LZMA_VERSION_STABILITY_ALPHA));
	exports->Set(String::NewSymbol("VERSION_STABILITY_BETA"),   Integer::NewFromUnsigned(LZMA_VERSION_STABILITY_BETA));
	exports->Set(String::NewSymbol("VERSION_STABILITY_STABLE"), Integer::NewFromUnsigned(LZMA_VERSION_STABILITY_STABLE));
	exports->Set(String::NewSymbol("VERSION"),                  Integer::NewFromUnsigned(LZMA_VERSION));
	exports->Set(String::NewSymbol("VERSION_STRING"),           String::New(LZMA_VERSION_STRING));
	
	exports->Set(String::NewSymbol("asyncCodeAvailable"),       Boolean::New(true));
}

}

NODE_MODULE(lzma_native, lzma::moduleInit)


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
	
	exports->Set(NanNew<String>("versionNumber"),            NanNew<FunctionTemplate>(lzmaVersionNumber)->GetFunction());
	exports->Set(NanNew<String>("versionString"),            NanNew<FunctionTemplate>(lzmaVersionString)->GetFunction());
	exports->Set(NanNew<String>("checkIsSupported"),         NanNew<FunctionTemplate>(lzmaCheckIsSupported)->GetFunction());
	exports->Set(NanNew<String>("checkSize"),                NanNew<FunctionTemplate>(lzmaCheckSize)->GetFunction());
	exports->Set(NanNew<String>("crc32_"),                    NanNew<FunctionTemplate>(lzmaCRC32)->GetFunction());
	exports->Set(NanNew<String>("filterEncoderIsSupported"), NanNew<FunctionTemplate>(lzmaFilterEncoderIsSupported)->GetFunction());
	exports->Set(NanNew<String>("filterDecoderIsSupported"), NanNew<FunctionTemplate>(lzmaFilterDecoderIsSupported)->GetFunction());
	exports->Set(NanNew<String>("rawEncoderMemusage"),       NanNew<FunctionTemplate>(lzmaRawEncoderMemusage)->GetFunction());
	exports->Set(NanNew<String>("rawDecoderMemusage"),       NanNew<FunctionTemplate>(lzmaRawDecoderMemusage)->GetFunction());
	exports->Set(NanNew<String>("mfIsSupported"),            NanNew<FunctionTemplate>(lzmaMfIsSupported)->GetFunction());
	exports->Set(NanNew<String>("modeIsSupported"),          NanNew<FunctionTemplate>(lzmaModeIsSupported)->GetFunction());
	exports->Set(NanNew<String>("easyEncoderMemusage"),      NanNew<FunctionTemplate>(lzmaEasyEncoderMemusage)->GetFunction());
	exports->Set(NanNew<String>("easyDecoderMemusage"),      NanNew<FunctionTemplate>(lzmaEasyDecoderMemusage)->GetFunction());
	
	// enum lzma_ret
	exports->Set(NanNew<String>("OK"),                NanNew<Number>(LZMA_OK));
	exports->Set(NanNew<String>("STREAM_END"),        NanNew<Number>(LZMA_STREAM_END));
	exports->Set(NanNew<String>("NO_CHECK"),          NanNew<Number>(LZMA_NO_CHECK));
	exports->Set(NanNew<String>("UNSUPPORTED_CHECK"), NanNew<Number>(LZMA_UNSUPPORTED_CHECK));
	exports->Set(NanNew<String>("GET_CHECK"),         NanNew<Number>(LZMA_GET_CHECK));
	exports->Set(NanNew<String>("MEM_ERROR"),         NanNew<Number>(LZMA_MEM_ERROR));
	exports->Set(NanNew<String>("MEMLIMIT_ERROR"),    NanNew<Number>(LZMA_MEMLIMIT_ERROR));
	exports->Set(NanNew<String>("FORMAT_ERROR"),      NanNew<Number>(LZMA_FORMAT_ERROR));
	exports->Set(NanNew<String>("OPTIONS_ERROR"),     NanNew<Number>(LZMA_OPTIONS_ERROR));
	exports->Set(NanNew<String>("DATA_ERROR"),        NanNew<Number>(LZMA_DATA_ERROR));
	exports->Set(NanNew<String>("BUF_ERROR"),         NanNew<Number>(LZMA_BUF_ERROR));
	exports->Set(NanNew<String>("PROG_ERROR"),        NanNew<Number>(LZMA_PROG_ERROR));
	
	// enum lzma_action
	exports->Set(NanNew<String>("RUN"),        NanNew<Number>(LZMA_RUN));
	exports->Set(NanNew<String>("SYNC_FLUSH"), NanNew<Number>(LZMA_SYNC_FLUSH));
	exports->Set(NanNew<String>("FULL_FLUSH"), NanNew<Number>(LZMA_FULL_FLUSH));
	exports->Set(NanNew<String>("FINISH"),     NanNew<Number>(LZMA_FINISH));
	
	// enum lzma_check
	exports->Set(NanNew<String>("CHECK_NONE"),   NanNew<Number>(LZMA_CHECK_NONE));
	exports->Set(NanNew<String>("CHECK_CRC32"),  NanNew<Number>(LZMA_CHECK_CRC32));
	exports->Set(NanNew<String>("CHECK_CRC64"),  NanNew<Number>(LZMA_CHECK_CRC64));
	exports->Set(NanNew<String>("CHECK_SHA256"), NanNew<Number>(LZMA_CHECK_SHA256));
	
	// lzma_match_finder
	exports->Set(NanNew<String>("MF_HC3"), NanNew<Number>(LZMA_MF_HC3));
	exports->Set(NanNew<String>("MF_HC4"), NanNew<Number>(LZMA_MF_HC4));
	exports->Set(NanNew<String>("MF_BT2"), NanNew<Number>(LZMA_MF_BT2));
	exports->Set(NanNew<String>("MF_BT3"), NanNew<Number>(LZMA_MF_BT3));
	exports->Set(NanNew<String>("MF_BT4"), NanNew<Number>(LZMA_MF_BT4));
	
	// lzma_mode
	exports->Set(NanNew<String>("MODE_FAST"),   NanNew<Number>(LZMA_MODE_FAST));
	exports->Set(NanNew<String>("MODE_NORMAL"), NanNew<Number>(LZMA_MODE_NORMAL));
	
	// defines
	exports->Set(NanNew<String>("FILTER_X86"),               NanNew<String>("LZMA_FILTER_X86"));
	exports->Set(NanNew<String>("FILTER_POWERPC"),           NanNew<String>("LZMA_FILTER_POWERPC"));
	exports->Set(NanNew<String>("FILTER_IA64"),              NanNew<String>("LZMA_FILTER_IA64"));
	exports->Set(NanNew<String>("FILTER_ARM"),               NanNew<String>("LZMA_FILTER_ARM"));
	exports->Set(NanNew<String>("FILTER_ARMTHUMB"),          NanNew<String>("LZMA_FILTER_ARMTHUMB"));
	exports->Set(NanNew<String>("FILTER_SPARC"),             NanNew<String>("LZMA_FILTER_SPARC"));
	exports->Set(NanNew<String>("FILTER_DELTA"),             NanNew<String>("LZMA_FILTER_DELTA"));
	exports->Set(NanNew<String>("FILTERS_MAX"),              NanNew<String>("LZMA_FILTERS_MAX"));
	exports->Set(NanNew<String>("FILTER_LZMA1"),             NanNew<String>("LZMA_FILTER_LZMA1"));
	exports->Set(NanNew<String>("FILTER_LZMA2"),             NanNew<String>("LZMA_FILTER_LZMA2"));
	exports->Set(NanNew<String>("VLI_UNKNOWN"),              NanNew<String>("LZMA_VLI_UNKNOWN"));
	
	exports->Set(NanNew<String>("VLI_BYTES_MAX"),            NanNew<Number>(LZMA_VLI_BYTES_MAX));
	exports->Set(NanNew<String>("CHECK_ID_MAX"),             NanNew<Number>(LZMA_CHECK_ID_MAX));
	exports->Set(NanNew<String>("CHECK_SIZE_MAX"),           NanNew<Number>(LZMA_CHECK_SIZE_MAX));
	exports->Set(NanNew<String>("PRESET_DEFAULT"),           NanNew<Number>(LZMA_PRESET_DEFAULT));
	exports->Set(NanNew<String>("PRESET_LEVEL_MASK"),        NanNew<Number>(LZMA_PRESET_LEVEL_MASK));
	exports->Set(NanNew<String>("PRESET_EXTREME"),           NanNew<Number>(LZMA_PRESET_EXTREME));
	exports->Set(NanNew<String>("TELL_NO_CHECK"),            NanNew<Number>(LZMA_TELL_NO_CHECK));
	exports->Set(NanNew<String>("TELL_UNSUPPORTED_CHECK"),   NanNew<Number>(LZMA_TELL_UNSUPPORTED_CHECK));
	exports->Set(NanNew<String>("TELL_ANY_CHECK"),           NanNew<Number>(LZMA_TELL_ANY_CHECK));
	exports->Set(NanNew<String>("CONCATENATED"),             NanNew<Number>(LZMA_CONCATENATED));
	exports->Set(NanNew<String>("STREAM_HEADER_SIZE"),       NanNew<Number>(LZMA_STREAM_HEADER_SIZE));
	exports->Set(NanNew<String>("VERSION_MAJOR"),            NanNew<Number>(LZMA_VERSION_MAJOR));
	exports->Set(NanNew<String>("VERSION_MINOR"),            NanNew<Number>(LZMA_VERSION_MINOR));
	exports->Set(NanNew<String>("VERSION_PATCH"),            NanNew<Number>(LZMA_VERSION_PATCH));
	exports->Set(NanNew<String>("VERSION_STABILITY"),        NanNew<Number>(LZMA_VERSION_STABILITY));
	exports->Set(NanNew<String>("VERSION_STABILITY_ALPHA"),  NanNew<Number>(LZMA_VERSION_STABILITY_ALPHA));
	exports->Set(NanNew<String>("VERSION_STABILITY_BETA"),   NanNew<Number>(LZMA_VERSION_STABILITY_BETA));
	exports->Set(NanNew<String>("VERSION_STABILITY_STABLE"), NanNew<Number>(LZMA_VERSION_STABILITY_STABLE));
	exports->Set(NanNew<String>("VERSION"),                  NanNew<Number>(LZMA_VERSION));
	exports->Set(NanNew<String>("VERSION_STRING"),           NanNew<String>(LZMA_VERSION_STRING));
	
	exports->Set(NanNew<String>("asyncCodeAvailable"),       NanNew<Boolean>(true));
}

}

NODE_MODULE(lzma_native, lzma::moduleInit)


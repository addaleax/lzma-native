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

#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif

#ifndef LIBLZMA_NODE_HPP
#define LIBLZMA_NODE_HPP

#if __cplusplus > 199711L
#define ASYNC_CODE_AVAILABLE
#include <mutex>
#endif

#include <node.h>
#include <node_object_wrap.h>
#include <v8.h>

#include <lzma.h>

#include <vector>
#include <string>

namespace lzma {
	using namespace v8;
	
	/* util */
	/**
	 * Return the filter constant associated with a v8 String handle
	 */
	lzma_vli FilterByName(Handle<Value> v);
	
	/**
	 * Return a string describing the error indicated by rv
	 */
	const char* lzmaStrError(lzma_ret rv);
	
	/**
	 * If rv represents an error, throw a javascript exception representing it.
	 * Always returns rv as a v8 Integer.
	 */
	Handle<Value> lzmaRet(lzma_ret rv);
	
	/**
	 * Takes a Node.js SlowBuffer or Buffer as input and populates ptr and len accordingly.
	 * Returns true on success, false on failure.
	 */
	bool readBufferFromObj(Handle<Value> value, const uint8_t*& ptr, size_t& len);
	
	/**
	 * Return a lzma_options_lzma struct as described by the v8 Object obj.
	 */
	lzma_options_lzma parseOptionsLZMA (Handle<Object> obj);
	
	/**
	 * Return a v8 Number representation of an uint64_t.
	 */
	Handle<Number> Uint64ToNumber(uint64_t in);
	
	/**
	 * Return a v8 Number representation of an uint64_t where UINT64_MAX will be mapped to null
	 */
	Handle<Value> Uint64ToNumberMaxNull(uint64_t in);
	
	/**
	 * Return a v8 Number representation of an uint64_t where 0 will be mapped to null
	 */
	Handle<Value> Uint64ToNumber0Null(uint64_t in);
	
	/**
	 * Return a uint64_t representation of a v8 Number,
	 * where values above UINT64_MAX map to UINT64_MAX and null to UINT64_MAX.
	 * Throws an TypeError if the input is not a number.
	 */
	uint64_t NumberToUint64ClampNullMax(Handle<Value> in);
	
	/* bindings in one-to-one correspondence to the lzma functions */
	Handle<Value> lzmaVersionNumber(const Arguments& args);
	Handle<Value> lzmaVersionString(const Arguments& args);
	Handle<Value> lzmaCheckIsSupported(const Arguments& args);
	Handle<Value> lzmaCheckSize(const Arguments& args);
	Handle<Value> lzmaFilterEncoderIsSupported(const Arguments& args);
	Handle<Value> lzmaFilterDecoderIsSupported(const Arguments& args);
	Handle<Value> lzmaMfIsSupported(const Arguments& args);
	Handle<Value> lzmaModeIsSupported(const Arguments& args);
	Handle<Value> lzmaEasyEncoderMemusage(const Arguments& args);
	Handle<Value> lzmaEasyDecoderMemusage(const Arguments& args);
	Handle<Value> lzmaCRC32(const Arguments& args);
	Handle<Value> lzmaRawEncoderMemusage(const Arguments& args);
	Handle<Value> lzmaRawDecoderMemusage(const Arguments& args);
	
	/* wrappers */
	/**
	 * List of liblzma filters with corresponding options
	 */
	class FilterArray {
		public:
			FilterArray() { finish(); }
			explicit FilterArray(Local<Array> arr);
			
			lzma_filter* array() { return filters.data(); }
			const lzma_filter* array() const { return filters.data(); }
			bool ok() const { return ok_; }
		private:
			FilterArray(const FilterArray&);
			FilterArray& operator=(const FilterArray&);
			
			void finish();
		
			union options {
				lzma_options_delta delta;
				lzma_options_lzma lzma;
			};
			
			bool ok_;
			std::vector<lzma_filter> filters;
			std::vector<options> optbuf;
	};

	/**
	 * Node.js object wrap for lzma_stream wrapper. Corresponds to exports.Stream
	 */
	class LZMAStream : public node::ObjectWrap {
		public:
			static void Init(Handle<Object> exports, Handle<Object> module);
		private:	
			explicit LZMAStream();
			~LZMAStream();
			
			static Persistent<Function> constructor;
			static Handle<Value> New(const Arguments& args);
			
			static Handle<Value> _failMissingSelf();

#ifdef ASYNC_CODE_AVAILABLE
			std::mutex mutex;
			static Handle<Value> AsyncCode(const Arguments& args);
#define LZMA_ASYNC_LOCK(strm) std::lock_guard<std::mutex> lock(strm->mutex);
#else
#define LZMA_ASYNC_LOCK(strm)
#endif 

			static Handle<Value> Code(const Arguments& args);
			static Handle<Value> Memusage(const Arguments& args);
			static Handle<Value> TotalIn(const Arguments& args);
			static Handle<Value> TotalOut(const Arguments& args);
			static Handle<Value> MemlimitGet(const Arguments& args);
			static Handle<Value> MemlimitSet(const Arguments& args);
			static Handle<Value> GetCheck(const Arguments& args);
			static Handle<Value> RawEncoder(const Arguments& args);
			static Handle<Value> RawDecoder(const Arguments& args);
			static Handle<Value> FiltersUpdate(const Arguments& args);
			static Handle<Value> EasyEncoder(const Arguments& args);
			static Handle<Value> StreamEncoder(const Arguments& args);
			static Handle<Value> AloneEncoder(const Arguments& args);
			static Handle<Value> StreamDecoder(const Arguments& args);
			static Handle<Value> AutoDecoder(const Arguments& args);
			static Handle<Value> AloneDecoder(const Arguments& args);
			static Handle<Value> CheckError(const Arguments& args);
			
			lzma_stream _;
			size_t bufsize;
			std::string error;
	};
	
	/**
	 * Node.js addon init function
	 */
	void moduleInit(Handle<Object> exports, Handle<Object> module);
}

#endif

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

#include <node.h>
#include <node_object_wrap.h>
#include <v8.h>
#include <uv.h>

#include <lzma.h>

#include <vector>
#include <string>

namespace lzma {
	using namespace v8;
	
	/* internal util */
	struct uv_mutex_guard {
		explicit uv_mutex_guard(uv_mutex_t& m_) : m(m_) { lock(); }
		~uv_mutex_guard() { unlock(); }
	
		inline void lock () { uv_mutex_lock(&m); }
		inline void unlock () { uv_mutex_unlock(&m); }
		
		uv_mutex_t& m;
	};
	
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
	 * Always returns rv as an integer value.
	 */
	int32_t lzmaRet(Isolate* isolate, lzma_ret rv);
	
	/**
	 * Takes a Node.js SlowBuffer or Buffer as input and populates ptr and len accordingly.
	 * Returns true on success, false on failure.
	 */
	bool readBufferFromObj(Isolate* isolate, Handle<Value> value, const uint8_t*& ptr, size_t& len);
	
	/**
	 * Return a lzma_options_lzma struct as described by the v8 Object obj.
	 */
	lzma_options_lzma parseOptionsLZMA (Isolate* isolate, Handle<Object> obj);
	
	/**
	 * Return a v8 Number representation of an uint64_t.
	 */
	Handle<Number> Uint64ToNumber(Isolate* isolate, uint64_t in);
	
	/**
	 * Return a v8 Number representation of an uint64_t where UINT64_MAX will be mapped to null
	 */
	Handle<Value> Uint64ToNumberMaxNull(Isolate* isolate, uint64_t in);
	
	/**
	 * Return a v8 Number representation of an uint64_t where 0 will be mapped to null
	 */
	Handle<Value> Uint64ToNumber0Null(Isolate* isolate, uint64_t in);
	
	/**
	 * Return a uint64_t representation of a v8 Number,
	 * where values above UINT64_MAX map to UINT64_MAX and null to UINT64_MAX.
	 * Throws an TypeError if the input is not a number.
	 */
	uint64_t NumberToUint64ClampNullMax(Isolate* isolate, Handle<Value> in);
	
	/* bindings in one-to-one correspondence to the lzma functions */
	void lzmaVersionNumber(const FunctionCallbackInfo<Value>& args);
	void lzmaVersionString(const FunctionCallbackInfo<Value>& args);
	void lzmaCheckIsSupported(const FunctionCallbackInfo<Value>& args);
	void lzmaCheckSize(const FunctionCallbackInfo<Value>& args);
	void lzmaFilterEncoderIsSupported(const FunctionCallbackInfo<Value>& args);
	void lzmaFilterDecoderIsSupported(const FunctionCallbackInfo<Value>& args);
	void lzmaMfIsSupported(const FunctionCallbackInfo<Value>& args);
	void lzmaModeIsSupported(const FunctionCallbackInfo<Value>& args);
	void lzmaEasyEncoderMemusage(const FunctionCallbackInfo<Value>& args);
	void lzmaEasyDecoderMemusage(const FunctionCallbackInfo<Value>& args);
	void lzmaCRC32(const FunctionCallbackInfo<Value>& args);
	void lzmaRawEncoderMemusage(const FunctionCallbackInfo<Value>& args);
	void lzmaRawDecoderMemusage(const FunctionCallbackInfo<Value>& args);
	
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
			
		/* regard as private: */
			void asyncWorker(void*);
		private:	
			explicit LZMAStream();
			~LZMAStream();
			
			static Persistent<Function> constructor;
			static void New(const FunctionCallbackInfo<Value>& args);
			
			static Handle<Value> _failMissingSelf(Isolate* isolate);

			bool hasRunningThread;
			
			uv_mutex_t lifespanMutex;
			uv_cond_t lifespanCond;
			uv_mutex_t mutex;
			
			static void AsyncCode(const FunctionCallbackInfo<Value>& args);
			
#define LZMA_ASYNC_LOCK(strm)    uv_mutex_guard lock(strm->mutex);
#define LZMA_ASYNC_LOCK_LS(strm) uv_mutex_guard lockLS(strm->lifespanMutex);

			static void Code(const FunctionCallbackInfo<Value>& args);
			static void Memusage(const FunctionCallbackInfo<Value>& args);
			static void TotalIn(const FunctionCallbackInfo<Value>& args);
			static void TotalOut(const FunctionCallbackInfo<Value>& args);
			static void MemlimitGet(const FunctionCallbackInfo<Value>& args);
			static void MemlimitSet(const FunctionCallbackInfo<Value>& args);
			static void GetCheck(const FunctionCallbackInfo<Value>& args);
			static void RawEncoder(const FunctionCallbackInfo<Value>& args);
			static void RawDecoder(const FunctionCallbackInfo<Value>& args);
			static void FiltersUpdate(const FunctionCallbackInfo<Value>& args);
			static void EasyEncoder(const FunctionCallbackInfo<Value>& args);
			static void StreamEncoder(const FunctionCallbackInfo<Value>& args);
			static void AloneEncoder(const FunctionCallbackInfo<Value>& args);
			static void StreamDecoder(const FunctionCallbackInfo<Value>& args);
			static void AutoDecoder(const FunctionCallbackInfo<Value>& args);
			static void AloneDecoder(const FunctionCallbackInfo<Value>& args);
			static void CheckError(const FunctionCallbackInfo<Value>& args);
			
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

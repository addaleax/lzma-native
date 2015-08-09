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
#include <nan.h>

#include <lzma.h>

#include <vector>
#include <set>
#include <queue>
#include <string>
#include <utility>

// C++11 features in libstdc++ shipped with Apple Clang
// See e.g. http://svn.boost.org/trac/boost/ticket/8092
#if __cplusplus <= 199711L || (__APPLE__ && (__GNUC_LIBSTD__ <= 4) && (__GNUC_LIBSTD_MINOR__ <= 2))
# define LZMA_NO_CXX11_RVALUE_REFERENCES
#endif

#ifndef LZMA_NO_CXX11_RVALUE_REFERENCES
# define LZMA_NATIVE_MOVE std::move
#else
# define LZMA_NATIVE_MOVE
#endif

#if NODE_MODULE_VERSION >= 11
#define LZMA_ASYNC_AVAILABLE
#endif

namespace lzma {
	using namespace v8;
	
	/* internal util */
#ifdef LZMA_ASYNC_AVAILABLE
	struct uv_mutex_guard {
		explicit uv_mutex_guard(uv_mutex_t& m_, bool autolock = true)
			: locked(false), m(m_)
		{
			if (autolock)
				lock(); 
		}
		
		~uv_mutex_guard() {
			if (locked)
				unlock();
		}
	
		inline void lock () {
			uv_mutex_lock(&m);
			locked = true;
		}
		
		inline void unlock () {
			uv_mutex_unlock(&m);
			locked = false;
		}
		
		bool locked;
		uv_mutex_t& m;
	};
#endif
	
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
	 * Return a javascript exception representing rv.
	 */
	Handle<Value> lzmaRetError(lzma_ret rv);
	
	/**
	 * Takes a Node.js SlowBuffer or Buffer as input and populates data accordingly.
	 * Returns true on success, false on failure.
	 */
	bool readBufferFromObj(Handle<Value> value, std::vector<uint8_t>& data);
	
	/**
	 * Return a lzma_options_lzma struct as described by the v8 Object obj.
	 */
	lzma_options_lzma parseOptionsLZMA (Handle<Object> obj);
	
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
	NAN_METHOD(lzmaVersionNumber);
	NAN_METHOD(lzmaVersionString);
	NAN_METHOD(lzmaCheckIsSupported);
	NAN_METHOD(lzmaCheckSize);
	NAN_METHOD(lzmaFilterEncoderIsSupported);
	NAN_METHOD(lzmaFilterDecoderIsSupported);
	NAN_METHOD(lzmaMfIsSupported);
	NAN_METHOD(lzmaModeIsSupported);
	NAN_METHOD(lzmaEasyEncoderMemusage);
	NAN_METHOD(lzmaEasyDecoderMemusage);
	NAN_METHOD(lzmaCRC32);
	NAN_METHOD(lzmaRawEncoderMemusage);
	NAN_METHOD(lzmaRawDecoderMemusage);
	
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
			static void Init(Handle<Object> exports);
			static const bool asyncCodeAvailable;
			
		/* regard as private: */
			void doLZMACodeFromAsync();
			void invokeBufferHandlersFromAsync();
		private:
			void resetUnderlying();
			void doLZMACode(bool async);
			void invokeBufferHandlers(bool async, bool hasLock);
			
			explicit LZMAStream();
			~LZMAStream();
			
			static Persistent<Function> constructor;
			static NAN_METHOD(New);
			
			static Handle<Value> _failMissingSelf();

			bool hasRunningThread;
			bool hasPendingCallbacks;
			bool hasRunningCallbacks;
			bool isNearDeath;

#ifdef LZMA_ASYNC_AVAILABLE
			uv_cond_t lifespanCond;
			uv_mutex_t mutex;
			uv_cond_t inputDataCond;
			
			uv_async_t* outputDataAsync;
			
#define LZMA_ASYNC_LOCK(strm)    uv_mutex_guard lock(strm->mutex);
#else
#define LZMA_ASYNC_LOCK(strm)
#endif

			static NAN_METHOD(Code);
			static NAN_METHOD(Memusage);
			static NAN_METHOD(TotalIn);
			static NAN_METHOD(TotalOut);
			static NAN_METHOD(MemlimitGet);
			static NAN_METHOD(MemlimitSet);
			static NAN_METHOD(GetCheck);
			static NAN_METHOD(RawEncoder);
			static NAN_METHOD(RawDecoder);
			static NAN_METHOD(FiltersUpdate);
			static NAN_METHOD(EasyEncoder);
			static NAN_METHOD(StreamEncoder);
			static NAN_METHOD(AloneEncoder);
			static NAN_METHOD(StreamDecoder);
			static NAN_METHOD(AutoDecoder);
			static NAN_METHOD(AloneDecoder);
			
			lzma_stream _;
			size_t bufsize;
			std::string error;
			
			bool shouldFinish;
			bool shouldInvokeChunkCallbacks;
			lzma_ret lastCodeResult;
			std::queue<std::vector<uint8_t> > inbufs;
			std::queue<std::vector<uint8_t> > outbufs;
	};
	
	/**
	 * Node.js addon init function
	 */
	void moduleInit(Handle<Object> exports);
}

#endif

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
#include <node_buffer.h>
#include <cstring>
#include <cstdlib>

namespace lzma {
#ifdef LZMA_ASYNC_AVAILABLE
uv_async_t LZMAStream::outputDataAsync;
uv_mutex_t LZMAStream::odp_mutex;
uv_once_t LZMAStream::outputDataAsyncSetupOnce = UV_ONCE_INIT;
const bool LZMAStream::asyncCodeAvailable = true;
#else
const bool LZMAStream::asyncCodeAvailable = false;
#endif

std::set<LZMAStream*> LZMAStream::outputDataPendingStreams;

namespace {
	extern "C" void worker(void* opaque) {
		LZMAStream* self = static_cast<LZMAStream*>(opaque);
		
		self->doLZMACodeFromAsync();
	}
	
	extern "C" void invoke_buffer_handlers_async(uv_async_t* async
#if UV_VERSION_MAJOR < 1
	, int status
#endif
	) {
		LZMAStream::odp_invoke();
	}
	
	extern "C" void output_data_async_setup() {
		LZMAStream::odp_setup_once();
	}
}

Persistent<Function> LZMAStream::constructor;

void LZMAStream::odp_setup_once() {
#ifdef LZMA_ASYNC_AVAILABLE
	uv_mutex_init(&odp_mutex);
	uv_async_init(uv_default_loop(), &outputDataAsync, invoke_buffer_handlers_async);
#endif
}

void LZMAStream::odp_invoke() {
	std::set<LZMAStream*> pendingStreams;
	
	{
		LZMA_ODP_LOCK(odp_mutex);
		
		std::swap(pendingStreams, outputDataPendingStreams);
	}
	
	for (std::set<LZMAStream*>::const_iterator it = pendingStreams.begin(); it != pendingStreams.end(); ++it)
		(*it)->invokeBufferHandlersFromAsync();
}

LZMAStream::LZMAStream()
	: hasRunningThread(false), hasPendingCallbacks(false), hasRunningCallbacks(false),
	isNearDeath(false), bufsize(8192), shouldFinish(false),
	shouldInvokeChunkCallbacks(false), lastCodeResult(LZMA_OK) 
{
	std::memset(&_, 0, sizeof(lzma_stream));

#ifdef LZMA_ASYNC_AVAILABLE
	uv_mutex_init(&mutex);
	uv_cond_init(&lifespanCond);
	uv_cond_init(&inputDataCond);
#endif
}

void LZMAStream::resetUnderlying() {
	lzma_end(&_);
	
	std::memset(&_, 0, sizeof(lzma_stream));
	lastCodeResult = LZMA_OK;
}

LZMAStream::~LZMAStream() {
	{
		// we do not need to invoke any output callbacks
		// since there are no references to us anyway
		LZMA_ODP_LOCK(odp_mutex);
		
		outputDataPendingStreams.erase(this);
	}
	
#ifdef LZMA_ASYNC_AVAILABLE
	{
		LZMA_ASYNC_LOCK(this)
		
		isNearDeath = true;
		uv_cond_broadcast(&inputDataCond);
		
		while (hasRunningThread || hasRunningCallbacks)
			uv_cond_wait(&lifespanCond, &mutex);
	}
#endif
	
	// no locking necessary from now on, we are the only active thread
	
	resetUnderlying();
	
#ifdef LZMA_ASYNC_AVAILABLE
	uv_mutex_destroy(&mutex);
	uv_cond_destroy(&lifespanCond);
	uv_cond_destroy(&inputDataCond);
#endif
}

NAN_METHOD(LZMAStream::Code) {
	NanScope();
	
	LZMAStream* self = node::ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	
	LZMA_ASYNC_LOCK(self)
	
	std::vector<uint8_t> inputData;
	
	Local<Object> bufarg = Local<Object>::Cast(args[0]);
	if (bufarg.IsEmpty() || bufarg->IsUndefined() || bufarg->IsNull()) {
		self->shouldFinish = true;
	} else {
		if (!readBufferFromObj(bufarg, inputData)) 
			NanReturnUndefined();
		
		if (inputData.empty())
			self->shouldFinish = true;
	}
	
	self->inbufs.push(LZMA_NATIVE_MOVE(inputData));
	
	bool hadRunningThread = self->hasRunningThread;
	bool async = args[1]->BooleanValue() || hadRunningThread;
	self->hasRunningThread = async;
	
	if (async) {
#ifdef LZMA_ASYNC_AVAILABLE
		if (!hadRunningThread) {
			uv_once(&outputDataAsyncSetupOnce, odp_setup_once);
			
			uv_thread_t worker_id;
			uv_thread_create(&worker_id, worker, static_cast<void*>(self));
		}
		
		uv_cond_broadcast(&self->inputDataCond);
#else
		std::abort();
#endif
	} else {
		self->doLZMACode(false);
	}
	
	NanReturnUndefined();
}

void LZMAStream::invokeBufferHandlersFromAsync() {
	invokeBufferHandlers(false, false);
}

void LZMAStream::invokeBufferHandlers(bool async, bool hasLock) {
#ifdef LZMA_ASYNC_AVAILABLE
	uv_mutex_guard lock(mutex, !hasLock);
#define POSSIBLY_LOCK_MX    do { if (!hasLock) lock.lock(); } while(0)
#define POSSIBLY_UNLOCK_MX  do { if (!hasLock) lock.unlock(); } while(0)
#else
#define POSSIBLY_LOCK_MX
#define POSSIBLY_UNLOCK_MX
#endif

	if (!hasLock && !hasPendingCallbacks)
		return;
	
	if (async) {
#ifdef LZMA_ASYNC_AVAILABLE
		hasPendingCallbacks = true;
		
		LZMA_ODP_LOCK(odp_mutex);
		
		outputDataPendingStreams.insert(this);
		// this calls invokeBufferHandler(false) from the main loop thread
		uv_async_send(&outputDataAsync);
		return;
#else
		std::abort();
#endif
	}
	
	hasRunningCallbacks = true;
	hasPendingCallbacks = false;
	
	struct _ScopeGuard {
		_ScopeGuard(LZMAStream* self_) : self(self_) {}
		~_ScopeGuard() {
			self->hasRunningCallbacks = false;

#ifdef LZMA_ASYNC_AVAILABLE
			uv_cond_broadcast(&self->lifespanCond);
#endif
		}
		
		LZMAStream* self;
	};
	_ScopeGuard guard(this);
	
	NanScope();
	
	Local<Function> bufferHandler = Local<Function>::Cast(NanObjectWrapHandle(this)->Get(NanNew<String>("bufferHandler")));
	std::vector<uint8_t> outbuf;
	
#define CALL_BUFFER_HANDLER_WITH_ARGV \
	POSSIBLY_UNLOCK_MX; \
	bufferHandler->Call(NanObjectWrapHandle(this), 3, argv); \
	POSSIBLY_LOCK_MX;
	
	while (outbufs.size() > 0) {
		outbuf = LZMA_NATIVE_MOVE(outbufs.front());
		outbufs.pop();
		
		Handle<Value> argv[3] = {
			NanNewBufferHandle(reinterpret_cast<const char*>(outbuf.data()), outbuf.size()),
			NanUndefined(), NanUndefined()
		};
		CALL_BUFFER_HANDLER_WITH_ARGV
	}
	
	if (lastCodeResult != LZMA_OK) {
		Handle<Value> errorArg = Handle<Value>(NanNull());
		
		if (lastCodeResult != LZMA_STREAM_END)
			errorArg = lzmaRetError(lastCodeResult);
		
		resetUnderlying(); // resets lastCodeResult!
		
		Handle<Value> argv[3] = { NanNull(), NanUndefined(), errorArg };
		CALL_BUFFER_HANDLER_WITH_ARGV
	}
	
	if (shouldInvokeChunkCallbacks) {
		shouldInvokeChunkCallbacks = false;
		
		Handle<Value> argv[3] = { NanUndefined(), NanNew<Boolean>(true), NanUndefined() };
		CALL_BUFFER_HANDLER_WITH_ARGV
	}
}

void LZMAStream::doLZMACodeFromAsync() {
	LZMA_ASYNC_LOCK(this)
	
	struct _ScopeGuard {
		_ScopeGuard(LZMAStream* self_) : self(self_) {}
		~_ScopeGuard() {
			self->hasRunningThread = false;

#ifdef LZMA_ASYNC_AVAILABLE
			uv_cond_broadcast(&self->lifespanCond);
#endif
		}
		
		LZMAStream* self;
	};
	_ScopeGuard guard(this);
	
	doLZMACode(true);
}

void LZMAStream::doLZMACode(bool async) {
	bool invokedBufferHandlers = false;
	
	std::vector<uint8_t> outbuf(bufsize), inbuf;
	_.next_out = outbuf.data();
	_.avail_out = outbuf.size();
	_.avail_in = 0;

	lzma_action action = LZMA_RUN;
	
	shouldInvokeChunkCallbacks = false;
	bool hasConsumedChunks = false;
	
	// _.internal is set to NULL when lzma_end() is called via resetUnderlying()
	while (_.internal) {
		if (_.avail_in == 0 && _.avail_out != 0) { // more input neccessary?
			if (inbufs.empty()) { // more input available?
				if (async) {
#ifdef LZMA_ASYNC_AVAILABLE
					if (hasConsumedChunks) {
						shouldInvokeChunkCallbacks = true;
						
						invokeBufferHandlers(async, true);
						invokedBufferHandlers = true;
					}
					
					// wait until more data is available
					while (inbufs.empty() && !shouldFinish && !isNearDeath)
						uv_cond_wait(&inputDataCond, &mutex);
#else
					std::abort();
#endif
				}
				
				if (action == LZMA_FINISH || isNearDeath)
					break;
				
				if (shouldFinish)
					action = LZMA_FINISH;
				
				if (!shouldFinish && !async) {
					shouldInvokeChunkCallbacks = true;
					invokedBufferHandlers = false;
					break;
				}
			}
			
			while (_.avail_in == 0 && !inbufs.empty()) {
				inbuf = LZMA_NATIVE_MOVE(inbufs.front());
				inbufs.pop();
			
				_.next_in = inbuf.data();
				_.avail_in = inbuf.size();
				hasConsumedChunks = true;
			}
		}
			
		_.next_out = outbuf.data();
		_.avail_out = outbuf.size();
		
		invokedBufferHandlers = false;
		lastCodeResult = lzma_code(&_, action);
		
		if (lastCodeResult != LZMA_OK && lastCodeResult != LZMA_STREAM_END)
			break;
		
		if (_.avail_out == 0 || _.avail_in == 0 || lastCodeResult == LZMA_STREAM_END) {
			size_t outsz = outbuf.size() - _.avail_out;
			
			if (outsz > 0) {
#if __cplusplus > 199711L // C++11
				outbufs.emplace(outbuf.data(), outbuf.data() + outsz);
#else
				outbufs.push(std::vector<uint8_t>(outbuf.data(), outbuf.data() + outsz));
#endif
			}
			
			// save status, since invokeBufferHandlers() may reset
			lzma_ret oldLCR = lastCodeResult;
			
			if (lastCodeResult == LZMA_STREAM_END)
				shouldInvokeChunkCallbacks = true;
			
			invokeBufferHandlers(async, true);
			invokedBufferHandlers = true;
			
			if (oldLCR == LZMA_STREAM_END)
				break;
		}
	}
	
	if (!invokedBufferHandlers)
		invokeBufferHandlers(async, true);
}

void LZMAStream::Init(Handle<Object> exports, Handle<Object> module) {
	Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
	tpl->SetClassName(NanNew<String>("LZMAStream"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
	tpl->PrototypeTemplate()->Set(NanNew<String>("code"),           NanNew<FunctionTemplate>(Code)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("memusage"),       NanNew<FunctionTemplate>(Memusage)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("getCheck"),       NanNew<FunctionTemplate>(GetCheck)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("memlimitGet"),    NanNew<FunctionTemplate>(MemlimitSet)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("memlimitSet"),    NanNew<FunctionTemplate>(MemlimitGet)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("totalIn"),        NanNew<FunctionTemplate>(TotalIn)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("totalOut"),       NanNew<FunctionTemplate>(TotalOut)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("rawEncoder_"),    NanNew<FunctionTemplate>(RawEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("rawDecoder_"),    NanNew<FunctionTemplate>(RawDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("filtersUpdate"),  NanNew<FunctionTemplate>(FiltersUpdate)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("easyEncoder_"),   NanNew<FunctionTemplate>(EasyEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("streamEncoder_"), NanNew<FunctionTemplate>(StreamEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("aloneEncoder"),   NanNew<FunctionTemplate>(AloneEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("streamDecoder_"), NanNew<FunctionTemplate>(StreamDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("autoDecoder_"),   NanNew<FunctionTemplate>(AutoDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("aloneDecoder_"),  NanNew<FunctionTemplate>(AloneDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(NanNew<String>("checkError"),     NanNew<FunctionTemplate>(CheckError)->GetFunction());
	NanAssignPersistent(constructor, tpl->GetFunction());
	exports->Set(NanNew<String>("Stream"), NanNew<Function>(constructor));
}

NAN_METHOD(LZMAStream::New) {
	NanScope();
	
	if (args.IsConstructCall()) {
		(new LZMAStream())->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		Local<Value> argv[0] = {};
		NanReturnValue(NanNew<Function>(constructor)->NewInstance(0, argv));
	}
}

Handle<Value> LZMAStream::_failMissingSelf() {
	NanThrowTypeError("LZMAStream methods need to be called on an LZMAStream object");
	return NanUndefined();
}

NAN_METHOD(LZMAStream::Memusage) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnUndefined();
	LZMA_ASYNC_LOCK(self)
	
	NanReturnValue(Uint64ToNumber0Null(lzma_memusage(&self->_)));
}

NAN_METHOD(LZMAStream::GetCheck) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	NanReturnValue(NanNew<Number>(lzma_get_check(&self->_)));
}

NAN_METHOD(LZMAStream::TotalIn) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	NanReturnValue(NanNew<Number>(self->_.total_in));
}

NAN_METHOD(LZMAStream::TotalOut) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	NanReturnValue(NanNew<Number>(self->_.total_out));
}

NAN_METHOD(LZMAStream::MemlimitGet) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	NanReturnValue(Uint64ToNumber0Null(lzma_memlimit_get(&self->_)));
}

NAN_METHOD(LZMAStream::MemlimitSet) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	Local<Number> arg = Local<Number>::Cast(args[0]);
	if (args[0]->IsUndefined() || arg.IsEmpty()) {
		NanThrowTypeError("memlimitSet() needs an number argument");
		NanReturnUndefined();
	}
	
	NanReturnValue(lzmaRet(lzma_memlimit_set(&self->_, NumberToUint64ClampNullMax(arg))));
}

NAN_METHOD(LZMAStream::RawEncoder) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	
	NanReturnValue(lzmaRet(lzma_raw_encoder(&self->_, filters.array())));
}

NAN_METHOD(LZMAStream::RawDecoder) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	
	NanReturnValue(lzmaRet(lzma_raw_decoder(&self->_, filters.array())));
}

NAN_METHOD(LZMAStream::FiltersUpdate) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	
	NanReturnValue(lzmaRet(lzma_filters_update(&self->_, filters.array())));
}

NAN_METHOD(LZMAStream::EasyEncoder) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	Local<Integer> preset = Local<Integer>::Cast(args[0]);
	Local<Integer> check = Local<Integer>::Cast(args[1]);
	
	NanReturnValue(lzmaRet(lzma_easy_encoder(&self->_, preset->Value(), (lzma_check) check->Value())));
}

NAN_METHOD(LZMAStream::StreamEncoder) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	Local<Integer> check = Local<Integer>::Cast(args[1]);
	
	NanReturnValue(lzmaRet(lzma_stream_encoder(&self->_, filters.array(), (lzma_check) check->Value())));
}

NAN_METHOD(LZMAStream::AloneEncoder) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	Local<Object> opt = Local<Object>::Cast(args[0]);
	lzma_options_lzma o = parseOptionsLZMA(opt);
	
	NanReturnValue(lzmaRet(lzma_alone_encoder(&self->_, &o)));
}

NAN_METHOD(LZMAStream::StreamDecoder) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	uint64_t memlimit = NumberToUint64ClampNullMax(args[0]);
	Local<Integer> flags = Local<Integer>::Cast(args[1]);
	
	NanReturnValue(lzmaRet(lzma_stream_decoder(&self->_, memlimit, flags->Value())));
}

NAN_METHOD(LZMAStream::AutoDecoder) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	uint64_t memlimit = NumberToUint64ClampNullMax(args[0]);
	Local<Integer> flags = Local<Integer>::Cast(args[1]);
	
	NanReturnValue(lzmaRet(lzma_auto_decoder(&self->_, memlimit, flags->Value())));
}

NAN_METHOD(LZMAStream::AloneDecoder) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	uint64_t memlimit = NumberToUint64ClampNullMax(args[0]);
	
	NanReturnValue(lzmaRet(lzma_alone_decoder(&self->_, memlimit)));
}

NAN_METHOD(LZMAStream::CheckError) {
	NanScope();
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		NanReturnValue(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	if (!self->error.empty()) {
		NanThrowError(self->error.c_str());
		self->error.clear();
	}
	
	NanReturnUndefined();
}

}

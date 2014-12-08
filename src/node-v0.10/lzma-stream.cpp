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

namespace lzma {

namespace {
	extern "C" void worker(void* opaque) {
		LZMAStream* self = static_cast<LZMAStream*>(opaque);
		self->doLZMACode(true);
	}
	
	extern "C" void invoke_buffer_handlers_async(uv_async_t* async, int status) {
		LZMAStream* data = static_cast<LZMAStream*>(async->data);
		data->invokeBufferHandlers(false);
	}
}

Persistent<Function> LZMAStream::constructor;

LZMAStream::LZMAStream()
	: hasRunningThread(false), bufsize(8192), shouldFinish(false),
	shouldInvokeChunkCallbacks(false), lastCodeResult(LZMA_OK) 
{
	std::memset(&_, 0, sizeof(lzma_stream));
	uv_mutex_init(&lifespanMutex);
	uv_mutex_init(&mutex);
	uv_cond_init(&lifespanCond);
	uv_cond_init(&inputDataCond);
	uv_async_init(uv_default_loop(), &outputDataAsync, invoke_buffer_handlers_async);
	outputDataAsync.data = static_cast<void*>(this);
}

LZMAStream::~LZMAStream() {
	{
		LZMA_ASYNC_LOCK(this)
		LZMA_ASYNC_LOCK_LS(this)
		
		while (hasRunningThread)
			uv_cond_wait(&lifespanCond, &lifespanMutex);
	}
	
	lzma_end(&_);
	
	uv_mutex_destroy(&mutex);
	uv_mutex_destroy(&lifespanMutex);
	uv_cond_destroy(&lifespanCond);
	uv_cond_destroy(&inputDataCond);
	uv_unref((uv_handle_t*) &outputDataAsync);
}

void LZMAStream::Init(Handle<Object> exports, Handle<Object> module) {
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewSymbol("LZMAStream"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
	tpl->PrototypeTemplate()->Set(String::NewSymbol("code"),           FunctionTemplate::New(Code)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("memusage"),       FunctionTemplate::New(Memusage)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("getCheck"),       FunctionTemplate::New(GetCheck)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("memlimitGet"),    FunctionTemplate::New(MemlimitSet)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("memlimitSet"),    FunctionTemplate::New(MemlimitGet)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("totalIn"),        FunctionTemplate::New(TotalIn)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("totalOut"),       FunctionTemplate::New(TotalOut)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("rawEncoder_"),    FunctionTemplate::New(RawEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("rawDecoder_"),    FunctionTemplate::New(RawDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("filtersUpdate"),  FunctionTemplate::New(FiltersUpdate)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("easyEncoder_"),   FunctionTemplate::New(EasyEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("streamEncoder_"), FunctionTemplate::New(StreamEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("aloneEncoder"),   FunctionTemplate::New(AloneEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("streamDecoder_"), FunctionTemplate::New(StreamDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("autoDecoder_"),   FunctionTemplate::New(AutoDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("aloneDecoder_"),  FunctionTemplate::New(AloneDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("checkError"),     FunctionTemplate::New(CheckError)->GetFunction());
	constructor = Persistent<Function>::New(tpl->GetFunction());
	exports->Set(String::NewSymbol("Stream"), constructor);
}

Handle<Value> LZMAStream::New(const Arguments& args) {
	HandleScope scope;
	
	if (args.IsConstructCall()) {
		(new LZMAStream())->Wrap(args.This());
		return scope.Close(args.This());
	} else {
		Local<Value> argv[0] = {};
		return scope.Close(constructor->NewInstance(0, argv));
	}
}

Handle<Value> LZMAStream::_failMissingSelf() {
	ThrowException(Exception::TypeError(String::New("LZMAStream methods need to be called on an LZMAStream object")));
	return Undefined();
}

Handle<Value> LZMAStream::Code(const Arguments& args) {	
	HandleScope scope;
	
	LZMAStream* self = node::ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	
	LZMA_ASYNC_LOCK(self)
	
	bool hadRunningThread = self->hasRunningThread;
	bool async = args[1]->BooleanValue() || hadRunningThread;
	self->hasRunningThread = async;
	
	std::vector<uint8_t> inputData;
	
	Local<Object> bufarg = Local<Object>::Cast(args[0]);
	if (bufarg.IsEmpty() || bufarg->IsUndefined() || bufarg->IsNull()) {
		self->shouldFinish = true;
	} else {
		if (!readBufferFromObj(bufarg, inputData)) 
			return scope.Close(Undefined());
		
		if (inputData.empty())
			self->shouldFinish = true;
	}
	
	self->inbufs.push(std::move(inputData));
	
	if (async) {
		if (!hadRunningThread) {
			uv_thread_t worker_id;
			uv_thread_create(&worker_id, worker, static_cast<void*>(self));
		}
		
		uv_cond_broadcast(&self->inputDataCond);
	} else {
		lock.unlock(); // doLZMACode has its own lock
		self->doLZMACode(false);
		lock.lock();
	}
	
	return scope.Close(Undefined());
}

void LZMAStream::invokeBufferHandlers(bool async) {
	if (async) {
		// this calls invokeBufferHandler(false) from the main loop thread
		uv_async_send(&outputDataAsync);
		return;
	}
	
	HandleScope scope;
	
	Local<Function> bufferHandler = Local<Function>::Cast(handle_->Get(String::New("bufferHandler")));
	std::vector<uint8_t> outbuf;
	
	while (outbufs.size() > 0) {
		outbuf = std::move(outbufs.front());
		outbufs.pop();
		
		Handle<Value> argv[3] = {
			node::Buffer::New(reinterpret_cast<const char*>(outbuf.data()), outbuf.size())->handle_,
			Undefined(), Undefined()
		};
		
		bufferHandler->Call(handle_, 3, argv);
	}
	
	if (lastCodeResult != LZMA_OK) {
		Handle<Value> argv[3] = { 
			Null(), Undefined(),
			lastCodeResult == LZMA_STREAM_END ? Handle<Value>(Null()) : lzmaRetError(lastCodeResult)
		};
		
		bufferHandler->Call(handle_, 3, argv);
	}
	
	if (shouldInvokeChunkCallbacks) {
		Handle<Value> argv[3] = { Undefined(), Boolean::New(true), Undefined() };
		bufferHandler->Call(handle_, 3, argv);
		shouldInvokeChunkCallbacks = false;
	}
}

void LZMAStream::doLZMACode(bool async) {
	LZMA_ASYNC_LOCK(this)
	LZMA_ASYNC_LOCK_LS(this)
	
	struct _ScopeGuard {
		_ScopeGuard(LZMAStream* self_) : self(self_) {}
		~_ScopeGuard() {
			self->hasRunningThread = false;
			uv_cond_broadcast(&self->lifespanCond);
		}
		
		LZMAStream* self;
	};
	_ScopeGuard guard(this);
	
	bool invokedBufferHandlers;
	
	std::vector<uint8_t> outbuf(bufsize), inbuf;
	_.next_out = outbuf.data();
	_.avail_out = outbuf.size();
	_.avail_in = 0;

	lzma_action action = LZMA_RUN;
	
	shouldInvokeChunkCallbacks = false;
	bool hasConsumedChunks = false;
	
	while (true) {
		if (_.avail_in == 0 && _.avail_out != 0) { // more input neccessary?
			if (inbufs.empty()) { // more input available?
				if (async) {
					if (hasConsumedChunks) {
						shouldInvokeChunkCallbacks = true;
						
						invokeBufferHandlers(async);
						invokedBufferHandlers = true;
					}
					
					// wait until more data is available
					while (inbufs.empty() && !shouldFinish)
						uv_cond_wait(&inputDataCond, &mutex);
				}
				
				if (action == LZMA_FINISH)
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
				inbuf = std::move(inbufs.front());
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
			
			if (outsz > 0)
				outbufs.emplace(outbuf.data(), outbuf.data() + outsz);
			
			invokeBufferHandlers(async);
			invokedBufferHandlers = true;
		}
	}
	
	if (!invokedBufferHandlers)
		invokeBufferHandlers(async);
}

Handle<Value> LZMAStream::Memusage(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(Undefined());
	LZMA_ASYNC_LOCK(self)
	
	return scope.Close(Uint64ToNumber0Null(lzma_memusage(&self->_)));
}

Handle<Value> LZMAStream::GetCheck(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	return scope.Close(Integer::NewFromUnsigned(lzma_get_check(&self->_)));
}

Handle<Value> LZMAStream::TotalIn(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	return scope.Close(Uint64ToNumber(self->_.total_in));
}

Handle<Value> LZMAStream::TotalOut(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	return scope.Close(Uint64ToNumber(self->_.total_out));
}

Handle<Value> LZMAStream::MemlimitGet(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	return scope.Close(Uint64ToNumber0Null(lzma_memlimit_get(&self->_)));
}

Handle<Value> LZMAStream::MemlimitSet(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	Local<Number> arg = Local<Number>::Cast(args[0]);
	if (args[0]->IsUndefined() || arg.IsEmpty()) {
		ThrowException(Exception::TypeError(String::New("memlimitSet() needs an number argument")));
		return scope.Close(Undefined());
	}
	
	return scope.Close(lzmaRet(lzma_memlimit_set(&self->_, NumberToUint64ClampNullMax(arg))));
}

Handle<Value> LZMAStream::RawEncoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	
	return scope.Close(lzmaRet(lzma_raw_encoder(&self->_, filters.array())));
}

Handle<Value> LZMAStream::RawDecoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	
	return scope.Close(lzmaRet(lzma_raw_decoder(&self->_, filters.array())));
}

Handle<Value> LZMAStream::FiltersUpdate(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	
	return scope.Close(lzmaRet(lzma_filters_update(&self->_, filters.array())));
}

Handle<Value> LZMAStream::EasyEncoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	Local<Integer> preset = Local<Integer>::Cast(args[0]);
	Local<Integer> check = Local<Integer>::Cast(args[1]);
	
	return scope.Close(lzmaRet(lzma_easy_encoder(&self->_, preset->Value(), (lzma_check) check->Value())));
}

Handle<Value> LZMAStream::StreamEncoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	Local<Integer> check = Local<Integer>::Cast(args[1]);
	
	return scope.Close(lzmaRet(lzma_stream_encoder(&self->_, filters.array(), (lzma_check) check->Value())));
}

Handle<Value> LZMAStream::AloneEncoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	Local<Object> opt = Local<Object>::Cast(args[0]);
	lzma_options_lzma o = parseOptionsLZMA(opt);
	
	return scope.Close(lzmaRet(lzma_alone_encoder(&self->_, &o)));
}

Handle<Value> LZMAStream::StreamDecoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	uint64_t memlimit = NumberToUint64ClampNullMax(args[0]);
	Local<Integer> flags = Local<Integer>::Cast(args[1]);
	
	return scope.Close(lzmaRet(lzma_stream_decoder(&self->_, memlimit, flags->Value())));
}

Handle<Value> LZMAStream::AutoDecoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	uint64_t memlimit = NumberToUint64ClampNullMax(args[0]);
	Local<Integer> flags = Local<Integer>::Cast(args[1]);
	
	return scope.Close(lzmaRet(lzma_auto_decoder(&self->_, memlimit, flags->Value())));
}

Handle<Value> LZMAStream::AloneDecoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	uint64_t memlimit = NumberToUint64ClampNullMax(args[0]);
	
	return scope.Close(lzmaRet(lzma_alone_decoder(&self->_, memlimit)));
}

Handle<Value> LZMAStream::CheckError(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	LZMA_ASYNC_LOCK(self)
	
	if (!self->error.empty()) {
		ThrowException(Exception::Error(String::New(self->error.c_str())));
		self->error.clear();
	}
	
	return scope.Close(Undefined());
}

}

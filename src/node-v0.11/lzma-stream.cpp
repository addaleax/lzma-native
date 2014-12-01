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

Persistent<Function> LZMAStream::constructor;

LZMAStream::LZMAStream()
	: hasRunningThread(false), bufsize(8192) {
	std::memset(&_, 0, sizeof(lzma_stream));
	uv_mutex_init(&lifespanMutex);
	uv_mutex_init(&mutex);
	uv_cond_init(&lifespanCond);
}

LZMAStream::~LZMAStream() {
	{
		LZMA_ASYNC_LOCK(this)
		LZMA_ASYNC_LOCK_LS(this) // declares lockLS
		
		while (hasRunningThread)
			uv_cond_wait(&lifespanCond, &lifespanMutex);
	}
	
	lzma_end(&_);
	
	uv_mutex_destroy(&mutex);
	uv_mutex_destroy(&lifespanMutex);
	uv_cond_destroy(&lifespanCond);
}

void LZMAStream::Init(Handle<Object> exports, Handle<Object> module) {
	Isolate* isolate = Isolate::GetCurrent();
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	tpl->SetClassName(String::NewFromUtf8(isolate, "LZMAStream"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "asyncCode_"),     FunctionTemplate::New(isolate, AsyncCode)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "code"),           FunctionTemplate::New(isolate, Code)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "memusage"),       FunctionTemplate::New(isolate, Memusage)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "getCheck"),       FunctionTemplate::New(isolate, GetCheck)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "memlimitGet"),    FunctionTemplate::New(isolate, MemlimitSet)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "memlimitSet"),    FunctionTemplate::New(isolate, MemlimitGet)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "totalIn"),        FunctionTemplate::New(isolate, TotalIn)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "totalOut"),       FunctionTemplate::New(isolate, TotalOut)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "rawEncoder_"),    FunctionTemplate::New(isolate, RawEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "rawDecoder_"),    FunctionTemplate::New(isolate, RawDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "filtersUpdate"),  FunctionTemplate::New(isolate, FiltersUpdate)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "easyEncoder_"),   FunctionTemplate::New(isolate, EasyEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "streamEncoder_"), FunctionTemplate::New(isolate, StreamEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "aloneEncoder"),   FunctionTemplate::New(isolate, AloneEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "streamDecoder_"), FunctionTemplate::New(isolate, StreamDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "autoDecoder_"),   FunctionTemplate::New(isolate, AutoDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "aloneDecoder_"),  FunctionTemplate::New(isolate, AloneDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewFromUtf8(isolate, "checkError"),     FunctionTemplate::New(isolate, CheckError)->GetFunction());
	
	constructor.Reset(isolate, tpl->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "Stream"), tpl->GetFunction());
}

void LZMAStream::New(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	if (args.IsConstructCall()) {
		(new LZMAStream())->Wrap(args.This());
		args.GetReturnValue().Set(args.This());
		return;
	} else {
		Local<Value> argv[0] = {};
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		args.GetReturnValue().Set(cons->NewInstance(0, argv));
		return;
	}
}

Handle<Value> LZMAStream::_failMissingSelf(Isolate* isolate) {
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "LZMAStream methods need to be called on an LZMAStream object")));
	return Undefined(isolate);
}

void LZMAStream::Code(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = node::ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().SetUndefined();
		return;
	}
	
	LZMA_ASYNC_LOCK(self)
	lzma_stream* strm = &self->_;
	lzma_action action;
	
	Local<Object> bufarg = Local<Object>::Cast(args[0]);
	if (bufarg.IsEmpty() || bufarg->IsUndefined() || bufarg->IsNull()) {
	finish_nodata:
		action = LZMA_FINISH;
		strm->next_in = NULL;
		strm->avail_in = 0;
	} else {
		action = LZMA_RUN;
		
		if (!readBufferFromObj(isolate, bufarg, strm->next_in, strm->avail_in)) {
			args.GetReturnValue().SetUndefined();
			return;
		}
		
		if (strm->avail_in == 0)
			goto finish_nodata;
	}
	
	std::vector<uint8_t> outbuf(self->bufsize);
	strm->next_out = outbuf.data();
	strm->avail_out = outbuf.size();

	Local<Function> bufferHandler = Local<Function>::Cast(args[1]);
	lzma_ret code = LZMA_OK;
	
	while (true) {
		code = lzma_code(&self->_, action);
		
		if (code != LZMA_OK && code != LZMA_STREAM_END)
			break;
		
		if (strm->avail_out == 0 || strm->avail_in == 0 || code == LZMA_STREAM_END) {
			size_t outsz = outbuf.size() - strm->avail_out;
			
			Handle<Value> argv[1] = { node::Buffer::New(isolate, reinterpret_cast<const char*>(outbuf.data()), outsz) };
			
			bufferHandler->Call(self->handle(isolate), 1, argv);
			
			if (strm->avail_out == 0) {
				strm->next_out = outbuf.data();
				strm->avail_out = outbuf.size();
				continue;
			}
		}
		
		if (strm->avail_in == 0)
			break;
	}
	
	args.GetReturnValue().Set(lzmaRet(isolate, code));
}

void LZMAStream::Memusage(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().SetUndefined();
		return;
	}
	
	LZMA_ASYNC_LOCK(self)
	
	args.GetReturnValue().Set(Uint64ToNumber0Null(isolate, lzma_memusage(&self->_)));
}

void LZMAStream::GetCheck(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().Set(_failMissingSelf(isolate));
		return;
	}
	
	LZMA_ASYNC_LOCK(self)
	
	args.GetReturnValue().Set(Integer::NewFromUnsigned(isolate, lzma_get_check(&self->_)));
}

void LZMAStream::TotalIn(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().Set(_failMissingSelf(isolate));
		return;
	}
	
	LZMA_ASYNC_LOCK(self)
	
	args.GetReturnValue().Set(Uint64ToNumber(isolate, self->_.total_in));
}

void LZMAStream::TotalOut(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().Set(_failMissingSelf(isolate));
		return;
	}
	
	LZMA_ASYNC_LOCK(self)
	
	args.GetReturnValue().Set(Uint64ToNumber(isolate, self->_.total_out));
}

void LZMAStream::MemlimitGet(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().Set(_failMissingSelf(isolate));
		return;
	}
	LZMA_ASYNC_LOCK(self)
	
	args.GetReturnValue().Set(Uint64ToNumber0Null(isolate, lzma_memlimit_get(&self->_)));
}

void LZMAStream::MemlimitSet(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().Set(_failMissingSelf(isolate));
		return;
	}
	
	LZMA_ASYNC_LOCK(self)
	
	Local<Number> arg = Local<Number>::Cast(args[0]);
	if (args[0]->IsUndefined() || arg.IsEmpty()) {
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "memlimitSet() needs an number argument")));
		args.GetReturnValue().SetUndefined();
		return;
	}
	
	args.GetReturnValue().Set(lzmaRet(isolate, lzma_memlimit_set(&self->_, NumberToUint64ClampNullMax(isolate, arg))));
}

void LZMAStream::RawEncoder(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().Set(_failMissingSelf(isolate));
		return;
	}
	
	LZMA_ASYNC_LOCK(self)
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	
	args.GetReturnValue().Set(lzmaRet(isolate, lzma_raw_encoder(&self->_, filters.array())));
}

void LZMAStream::RawDecoder(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().Set(_failMissingSelf(isolate));
		return;
	}
	LZMA_ASYNC_LOCK(self)
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	
	args.GetReturnValue().Set(lzmaRet(isolate, lzma_raw_decoder(&self->_, filters.array())));
}

void LZMAStream::FiltersUpdate(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().Set(_failMissingSelf(isolate));
		return;
	}
	
	LZMA_ASYNC_LOCK(self)
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	
	args.GetReturnValue().Set(lzmaRet(isolate, lzma_filters_update(&self->_, filters.array())));
}

void LZMAStream::EasyEncoder(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().Set(_failMissingSelf(isolate));
		return;
	}
	
	LZMA_ASYNC_LOCK(self)
	
	Local<Integer> preset = Local<Integer>::Cast(args[0]);
	Local<Integer> check = Local<Integer>::Cast(args[1]);
	
	args.GetReturnValue().Set(lzmaRet(isolate, lzma_easy_encoder(&self->_, preset->Value(), (lzma_check) check->Value())));
}

void LZMAStream::StreamEncoder(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().Set(_failMissingSelf(isolate));
		return;
	}
	LZMA_ASYNC_LOCK(self)
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	Local<Integer> check = Local<Integer>::Cast(args[1]);
	
	args.GetReturnValue().Set(lzmaRet(isolate, lzma_stream_encoder(&self->_, filters.array(), (lzma_check) check->Value())));
}

void LZMAStream::AloneEncoder(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().Set(_failMissingSelf(isolate));
		return;
	}
	
	LZMA_ASYNC_LOCK(self)
	
	Local<Object> opt = Local<Object>::Cast(args[0]);
	lzma_options_lzma o = parseOptionsLZMA(isolate, opt);
	
	args.GetReturnValue().Set(lzmaRet(isolate, lzma_alone_encoder(&self->_, &o)));
}

void LZMAStream::StreamDecoder(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().Set(_failMissingSelf(isolate));
		return;
	}
	
	LZMA_ASYNC_LOCK(self)
	
	uint64_t memlimit = NumberToUint64ClampNullMax(isolate, args[0]);
	Local<Integer> flags = Local<Integer>::Cast(args[1]);
	
	args.GetReturnValue().Set(lzmaRet(isolate, lzma_stream_decoder(&self->_, memlimit, flags->Value())));
}

void LZMAStream::AutoDecoder(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().Set(_failMissingSelf(isolate));
		return;
	}
	
	LZMA_ASYNC_LOCK(self)
	
	uint64_t memlimit = NumberToUint64ClampNullMax(isolate, args[0]);
	Local<Integer> flags = Local<Integer>::Cast(args[1]);
	
	args.GetReturnValue().Set(lzmaRet(isolate, lzma_auto_decoder(&self->_, memlimit, flags->Value())));
}

void LZMAStream::AloneDecoder(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().Set(_failMissingSelf(isolate));
		return;
	}
	
	LZMA_ASYNC_LOCK(self)
	
	uint64_t memlimit = NumberToUint64ClampNullMax(isolate, args[0]);
	
	args.GetReturnValue().Set(lzmaRet(isolate, lzma_alone_decoder(&self->_, memlimit)));
}

void LZMAStream::CheckError(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self) {
		args.GetReturnValue().Set(_failMissingSelf(isolate));
		return;
	}
	
	LZMA_ASYNC_LOCK(self)
	
	if (!self->error.empty()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, self->error.c_str())));
		self->error.clear();
	}
	
	args.GetReturnValue().SetUndefined();
}

}

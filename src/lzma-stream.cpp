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

namespace lzma {

Persistent<Function> LZMAStream::constructor;

void LZMAStream::Init(Handle<Object> exports, Handle<Object> module) {
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewSymbol("LZMAStream"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
#ifdef ASYNC_CODE_AVAILABLE
	tpl->PrototypeTemplate()->Set(String::NewSymbol("asyncCode_"),     FunctionTemplate::New(AsyncCode)->GetFunction());
#endif
	tpl->PrototypeTemplate()->Set(String::NewSymbol("code"),           FunctionTemplate::New(Code)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("memusage"),       FunctionTemplate::New(Memusage)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("getCheck"),       FunctionTemplate::New(GetCheck)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("memlimitGet"),    FunctionTemplate::New(MemlimitSet)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("memlimitSet"),    FunctionTemplate::New(MemlimitGet)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("rawEncoder_"),    FunctionTemplate::New(RawEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("rawDecoder_"),    FunctionTemplate::New(RawDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("filtersUpdate"),  FunctionTemplate::New(FiltersUpdate)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("easyEncoder_"),   FunctionTemplate::New(EasyEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("streamEncoder_"), FunctionTemplate::New(StreamEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("aloneEncoder"),   FunctionTemplate::New(AloneEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("streamDecoder_"), FunctionTemplate::New(StreamDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("autoDecoder_"),   FunctionTemplate::New(AutoDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("aloneDecoder_"),  FunctionTemplate::New(AloneDecoder)->GetFunction());
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
		return scope.Close(Undefined());
	
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
		
		if (!readBufferFromObj(bufarg, strm->next_in, strm->avail_in)) 
			return scope.Close(Undefined());
		
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
		
		if (strm->avail_out == 0 || strm->avail_in == 0 || code == LZMA_STREAM_END) {
			size_t outsz = outbuf.size() - strm->avail_out;
			
			Handle<Value> argv[1] = { node::Buffer::New(reinterpret_cast<const char*>(outbuf.data()), outsz)->handle_ };
			
			bufferHandler->Call(self->handle_, 1, argv);
			
			if (strm->avail_out == 0) {
				strm->next_out = outbuf.data();
				strm->avail_out = outbuf.size();
				continue;
			}
		}
		
		if (strm->avail_in == 0 || code != LZMA_OK)
			break;
	}
	
	return lzmaRet(code);
}

Handle<Value> LZMAStream::Memusage(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(Undefined());
	
	return scope.Close(Integer::NewFromUnsigned(lzma_memusage(&self->_)));
}

Handle<Value> LZMAStream::MemlimitGet(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	
	return scope.Close(Integer::NewFromUnsigned(lzma_memlimit_get(&self->_)));
}

Handle<Value> LZMAStream::GetCheck(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	
	return scope.Close(Integer::NewFromUnsigned(lzma_get_check(&self->_)));
}

Handle<Value> LZMAStream::MemlimitSet(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
		
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	if (args[0]->IsUndefined() || arg.IsEmpty()) {
		ThrowException(Exception::TypeError(String::New("memlimitSet() needs an integer argument")));
		return scope.Close(Undefined());
	}
	
	lzma_memlimit_set(&self->_, arg->Value());
	
	return scope.Close(arg);
}

Handle<Value> LZMAStream::RawEncoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	
	return scope.Close(lzmaRet(lzma_raw_encoder(&self->_, filters.array())));
}

Handle<Value> LZMAStream::RawDecoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	
	return scope.Close(lzmaRet(lzma_raw_decoder(&self->_, filters.array())));
}

Handle<Value> LZMAStream::FiltersUpdate(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	
	return scope.Close(lzmaRet(lzma_filters_update(&self->_, filters.array())));
}

Handle<Value> LZMAStream::EasyEncoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	
	Local<Integer> preset = Local<Integer>::Cast(args[0]);
	Local<Integer> check = Local<Integer>::Cast(args[1]);
	
	return scope.Close(lzmaRet(lzma_easy_encoder(&self->_, preset->Value(), (lzma_check) check->Value())));
}

Handle<Value> LZMAStream::StreamEncoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	
	const FilterArray filters(Local<Array>::Cast(args[0]));
	Local<Integer> check = Local<Integer>::Cast(args[1]);
	
	return scope.Close(lzmaRet(lzma_stream_encoder(&self->_, filters.array(), (lzma_check) check->Value())));
}

Handle<Value> LZMAStream::AloneEncoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	
	Local<Object> opt = Local<Object>::Cast(args[0]);
	lzma_options_lzma o = parseOptionsLZMA(opt);
	
	return scope.Close(lzmaRet(lzma_alone_encoder(&self->_, &o)));
}

Handle<Value> LZMAStream::StreamDecoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	
	uint64_t memlimit = args[0]->IsNull() ? UINT64_MAX : uint64_t(Local<Integer>::Cast(args[0])->Value());
	Local<Integer> flags = Local<Integer>::Cast(args[1]);
	
	return scope.Close(lzmaRet(lzma_stream_decoder(&self->_, memlimit, flags->Value())));
}

Handle<Value> LZMAStream::AutoDecoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	
	uint64_t memlimit = args[0]->IsNull() ? UINT64_MAX : uint64_t(Local<Integer>::Cast(args[0])->Value());
	Local<Integer> flags = Local<Integer>::Cast(args[1]);
	
	return scope.Close(lzmaRet(lzma_auto_decoder(&self->_, memlimit, flags->Value())));
}

Handle<Value> LZMAStream::AloneDecoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	
	uint64_t memlimit = args[0]->IsNull() ? UINT64_MAX : uint64_t(Local<Integer>::Cast(args[0])->Value());
	
	return scope.Close(lzmaRet(lzma_alone_decoder(&self->_, memlimit)));
}

}

#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif

#include <node.h>
#include <node_object_wrap.h>
#include <node_buffer.h>
#include <v8.h>
#include <lzma.h>
#include <vector>
#include <cstring>

using namespace v8;

lzma_vli Unfilter(Handle<Value> v) {
	String::AsciiValue cmpto(v);
	if (cmpto.length() == 0)
		return LZMA_VLI_UNKNOWN;
	
	struct searchEntry {
		const char* str;
		lzma_vli value;
	};
	
	static const struct searchEntry search[] = {
		{ "LZMA_FILTER_X86", LZMA_FILTER_X86 },
		{ "LZMA_FILTER_POWERPC", LZMA_FILTER_POWERPC },
		{ "LZMA_FILTER_IA64", LZMA_FILTER_IA64 },
		{ "LZMA_FILTER_ARM", LZMA_FILTER_ARM },
		{ "LZMA_FILTER_ARMTHUMB", LZMA_FILTER_ARMTHUMB },
		{ "LZMA_FILTER_SPARC", LZMA_FILTER_SPARC },
		{ "LZMA_FILTER_DELTA", LZMA_FILTER_DELTA },
		{ "LZMA_FILTERS_MAX", LZMA_FILTERS_MAX },
		{ "LZMA_FILTER_LZMA1", LZMA_FILTER_LZMA1 },
		{ "LZMA_FILTER_LZMA2", LZMA_FILTER_LZMA2 },
		{ "LZMA_VLI_UNKNOWN", LZMA_VLI_UNKNOWN }
	};
	
	for (const struct searchEntry* p = search; ; ++p) 
		if (p->value == LZMA_VLI_UNKNOWN || std::strcmp(*cmpto, p->str) == 0)
			return p->value;
}

Handle<Value> lzmaRet(lzma_ret rv) {
	switch (rv) {
		case LZMA_OK:                break;
		case LZMA_STREAM_END:        break;
		case LZMA_NO_CHECK:          ThrowException(Exception::Error(String::New("LZMA_NO_CHECK"))); break;
		case LZMA_UNSUPPORTED_CHECK: ThrowException(Exception::Error(String::New("LZMA_UNSUPPORTED_CHECK"))); break;
		case LZMA_GET_CHECK:         ThrowException(Exception::Error(String::New("LZMA_GET_CHECK"))); break;
		case LZMA_MEM_ERROR:         ThrowException(Exception::Error(String::New("LZMA_MEM_ERROR"))); break;
		case LZMA_MEMLIMIT_ERROR:    ThrowException(Exception::Error(String::New("LZMA_MEMLIMIT_ERROR"))); break;
		case LZMA_FORMAT_ERROR:      ThrowException(Exception::Error(String::New("LZMA_FORMAT_ERROR"))); break;
		case LZMA_OPTIONS_ERROR:     ThrowException(Exception::Error(String::New("LZMA_OPTIONS_ERROR"))); break;
		case LZMA_DATA_ERROR:        ThrowException(Exception::Error(String::New("LZMA_DATA_ERROR"))); break;
		case LZMA_PROG_ERROR:        ThrowException(Exception::Error(String::New("LZMA_PROG_ERROR"))); break;
		case LZMA_BUF_ERROR:         ThrowException(Exception::Error(String::New("LZMA_BUF_ERROR"))); break;
	}
	
	return Integer::New(rv);
}

Handle<Value> lzmaVersionNumber(const Arguments& args) {
	HandleScope scope; 
	return scope.Close(Integer::NewFromUnsigned(lzma_version_number()));
}

Handle<Value> lzmaVersionString(const Arguments& args) {
	HandleScope scope;
	return scope.Close(String::New(lzma_version_string()));
}

Handle<Value> lzmaCheckIsSupported(const Arguments& args) {
	HandleScope scope;
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	return scope.Close(Integer::NewFromUnsigned(lzma_check_is_supported((lzma_check) arg->Value())));
}

Handle<Value> lzmaCheckSize(const Arguments& args) {
	HandleScope scope;
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	return scope.Close(Integer::NewFromUnsigned(lzma_check_size((lzma_check) arg->Value())));
}

Handle<Value> lzmaFilterEncoderIsSupported(const Arguments& args) {
	HandleScope scope;
	uint64_t arg = Unfilter(args[0]);
	
	return scope.Close(Integer::NewFromUnsigned(lzma_filter_encoder_is_supported(arg)));
}

Handle<Value> lzmaFilterDecoderIsSupported(const Arguments& args) {
	HandleScope scope;
	uint64_t arg = Unfilter(args[0]);
	
	return scope.Close(Integer::NewFromUnsigned(lzma_filter_decoder_is_supported(arg)));
}

Handle<Value> lzmaMfIsSupported(const Arguments& args) {
	HandleScope scope;
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	return scope.Close(Integer::NewFromUnsigned(lzma_mf_is_supported((lzma_match_finder) arg->Value())));
}

Handle<Value> lzmaModeIsSupported(const Arguments& args) {
	HandleScope scope;
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	return scope.Close(Integer::NewFromUnsigned(lzma_mode_is_supported((lzma_mode) arg->Value())));
}

Handle<Value> lzmaEasyEncoderMemusage(const Arguments& args) {
	HandleScope scope;
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	return scope.Close(Number::New(lzma_easy_encoder_memusage(arg->Value())));
}

Handle<Value> lzmaEasyDecoderMemusage(const Arguments& args) {
	HandleScope scope;
	Local<Integer> arg = Local<Integer>::Cast(args[0]);
	
	return scope.Close(Number::New(lzma_easy_decoder_memusage(arg->Value())));
}

Handle<Value> lzmaStreamBufferBound(const Arguments& args) {
	HandleScope scope;
	Local<Number> arg = Local<Number>::Cast(args[0]);
	
	return scope.Close(Number::New(lzma_stream_buffer_bound(arg->Value())));
}

Handle<Value> lzmaCRC32(const Arguments& args) {
	HandleScope scope;
	Local<Integer> arg = Local<Integer>::Cast(args[1]);
	
	if (arg.IsEmpty() || args[1]->IsUndefined())
		arg = Integer::New(0);
	
	node::Buffer* buf = node::ObjectWrap::Unwrap<node::Buffer>(Handle<Object>::Cast(args[0]));
	if (!buf) {
		ThrowException(Exception::TypeError(String::New("CRC32 expects Buffer as input")));
		return scope.Close(Undefined());
	}
	
	return scope.Close(Integer::NewFromUnsigned(lzma_crc32(reinterpret_cast<const uint8_t*>(node::Buffer::Data(buf)), node::Buffer::Length(buf), arg->Value())));
}

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

lzma_options_lzma parseOptionsLZMA (Handle<Object> obj) {
	HandleScope scope;
	lzma_options_lzma r;
	
	if (obj.IsEmpty() || obj->IsUndefined())
		obj = Object::New();
	
	Local<Value> dict_size = obj->Get(String::NewSymbol("dictSize"));
	Local<Value> lp = obj->Get(String::NewSymbol("lp"));
	Local<Value> lc = obj->Get(String::NewSymbol("lc"));
	Local<Value> pb = obj->Get(String::NewSymbol("pb"));
	Local<Value> mode = obj->Get(String::NewSymbol("mode"));
	Local<Value> nice_len = obj->Get(String::NewSymbol("niceLen"));
	Local<Value> mf = obj->Get(String::NewSymbol("mf"));
	Local<Value> depth = obj->Get(String::NewSymbol("depth"));
	Local<Value> preset = obj->Get(String::NewSymbol("preset"));
	r.dict_size = dict_size.IsEmpty() || dict_size->IsUndefined() ? LZMA_DICT_SIZE_DEFAULT : Local<Integer>::Cast(dict_size)->Value();
	r.lc = lc.IsEmpty() || lc->IsUndefined() ? LZMA_LC_DEFAULT : Local<Integer>::Cast(lc)->Value();
	r.lp = lp.IsEmpty() || lp->IsUndefined() ? LZMA_LP_DEFAULT : Local<Integer>::Cast(lp)->Value();
	r.pb = pb.IsEmpty() || pb->IsUndefined() ? LZMA_PB_DEFAULT : Local<Integer>::Cast(pb)->Value();
	r.mode = mode.IsEmpty() || mode->IsUndefined() ? LZMA_MODE_FAST : (lzma_mode)Local<Integer>::Cast(mode)->Value();
	r.nice_len = nice_len.IsEmpty() || nice_len->IsUndefined() ? 64 : Local<Integer>::Cast(nice_len)->Value();
	r.mf = mf.IsEmpty() || mf->IsUndefined() ? LZMA_MF_HC4 : (lzma_match_finder)Local<Integer>::Cast(mf)->Value();
	r.depth = mf.IsEmpty() || depth->IsUndefined() ? 0 : Local<Integer>::Cast(depth)->Value();
	r.preset_dict = NULL;
	
	if (!preset.IsEmpty() && !preset->IsUndefined()) 
		lzma_lzma_preset(&r, Local<Integer>::Cast(preset)->Value());
	
	return r;
}

FilterArray::FilterArray(Local<Array> arr) : ok_(false) {
	HandleScope scope;
	
	uint32_t len = arr.IsEmpty() ? 0 : arr->Length();
	
	Local<String> id_ = String::NewSymbol("id");
	Local<String> options_ = String::NewSymbol("options");
	
	for (uint32_t i = 0; i < len; ++i) {
		Local<Object> entry = Local<Object>::Cast(arr);
		if (entry.IsEmpty() || entry->IsUndefined()) {
			ThrowException(Exception::TypeError(String::New("Filter array needs object entries")));
			return;
		}
		
		Local<String> id = Local<String>::Cast(entry->Get(id_));
		Local<Object> opt = Local<Object>::Cast(entry->Get(options_));
		
		lzma_filter f;
		f.id = Unfilter(id);
		f.options = NULL;
		if (opt.IsEmpty() || opt->IsUndefined()) {
			filters.push_back(f);
			continue;
		}
		
		union options bopt;
		
		switch (Unfilter(id)) {
			case LZMA_FILTER_DELTA:
				bopt.delta.type = (lzma_delta_type) Local<Integer>::Cast(opt->Get(String::NewSymbol("type")))->Value();
				bopt.delta.dist = Local<Integer>::Cast(opt->Get(String::NewSymbol("dist")))->Value();
				break;
			case LZMA_FILTER_LZMA1:
			case LZMA_FILTER_LZMA2:
				bopt.lzma = parseOptionsLZMA(opt);
				break;
			default:
				ThrowException(Exception::TypeError(String::New("LZMA Wrapper library understands .options only for DELTA and LZMA[12] filters")));
				return;
		}
		
		optbuf.push_back(bopt);
		f.options = &optbuf.back();
		filters.push_back(f);
	}
	
	finish();
}

void FilterArray::finish() {
	lzma_filter end;
	end.id = LZMA_VLI_UNKNOWN;
	filters.push_back(end);
	ok_ = true;
}

Handle<Value> lzmaRawEncoderMemusage(const Arguments& args) {
	HandleScope scope;
	Local<Array> arg = Local<Array>::Cast(args[0]);
	if (arg.IsEmpty() || args[0]->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("rawEncoderMemusage requires filter array as arguments")));
		return scope.Close(Undefined());
	}
	
	const FilterArray fa(arg);
	if (!fa.ok()) 
		return scope.Close(Undefined());
	
	return scope.Close(Integer::NewFromUnsigned(lzma_raw_encoder_memusage(fa.array())));
}

Handle<Value> lzmaRawDecoderMemusage(const Arguments& args) {
	HandleScope scope;
	Local<Array> arg = Local<Array>::Cast(args[0]);
	if (arg.IsEmpty() || args[0]->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("rawdecoderMemusage requires filter array as arguments")));
		return scope.Close(Undefined());
	}
	
	const FilterArray fa(arg);
	if (!fa.ok()) 
		return scope.Close(Undefined());
	
	return scope.Close(Integer::NewFromUnsigned(lzma_raw_decoder_memusage(fa.array())));
}

class LZMAStream : public node::ObjectWrap {
	public:
		static void Init(Handle<Object> exports);
	private:
		explicit LZMAStream() : _(LZMA_STREAM_INIT) {}
		~LZMAStream() { lzma_end(&_); }
		
		static Persistent<Function> constructor;
		static Handle<Value> New(const Arguments& args);
		
		static Handle<Value> _failMissingSelf();
		
		static Handle<Value> Code(const Arguments& args);
		static Handle<Value> Memusage(const Arguments& args);
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
		
		lzma_stream _;
};

Persistent<Function> LZMAStream::constructor;

void LZMAStream::Init(Handle<Object> exports) {
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewSymbol("LZMAStream"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	tpl->PrototypeTemplate()->Set(String::NewSymbol("code"), FunctionTemplate::New(Code)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("memusage"), FunctionTemplate::New(Memusage)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("getCheck"), FunctionTemplate::New(GetCheck)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("memlimitGet"), FunctionTemplate::New(MemlimitSet)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("memlimitSet"), FunctionTemplate::New(MemlimitGet)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("rawEncoder"), FunctionTemplate::New(RawEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("rawDecoder"), FunctionTemplate::New(RawDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("filtersUpdate"), FunctionTemplate::New(FiltersUpdate)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("easyEncoder"), FunctionTemplate::New(EasyEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("streamEncoder"), FunctionTemplate::New(StreamEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("aloneEncoder"), FunctionTemplate::New(AloneEncoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("streamDecoder"), FunctionTemplate::New(StreamDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("autoDecoder"), FunctionTemplate::New(AutoDecoder)->GetFunction());
	tpl->PrototypeTemplate()->Set(String::NewSymbol("aloneDecoder"), FunctionTemplate::New(AloneDecoder)->GetFunction());
	constructor = Persistent<Function>::New(tpl->GetFunction());
	exports->Set(String::NewSymbol("Stream"), constructor);
}

Handle<Value> LZMAStream::New(const Arguments& args) {
	HandleScope scope;
	
	if (args.IsConstructCall()) {
		(new LZMAStream())->Wrap(args.This());
		return args.This();
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
	
	if (args[0].IsEmpty() || args[0]->IsUndefined()) {
	finish_nodata:
		action = LZMA_FINISH;
		strm->next_in = NULL;
		strm->avail_in = 0;
	} else {
		action = LZMA_RUN;
		
		node::Buffer* buf = node::ObjectWrap::Unwrap<node::Buffer>(Handle<Object>::Cast(args[0]));
		if (!buf) {
			ThrowException(Exception::TypeError(String::New("Cod() expects Buffer as input")));
			return scope.Close(Undefined());
		}
		
		if (node::Buffer::Length(buf) == 0)
			goto finish_nodata;
		
		strm->next_in = reinterpret_cast<const uint8_t*>(node::Buffer::Data(buf));
		strm->avail_in = node::Buffer::Length(buf);
	}
	
	std::vector<uint8_t> outbuf(8192);
	strm->next_out = outbuf.data();
	strm->avail_out = outbuf.size();

	Local<Function> bufferHandler = Local<Function>::Cast(self->handle_->Get(String::NewSymbol("bufferHandler")));
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
	
	Local<Integer> memlimit = Local<Integer>::Cast(args[0]);
	Local<Integer> flags = Local<Integer>::Cast(args[1]);
	
	return scope.Close(lzmaRet(lzma_stream_decoder(&self->_, memlimit->Value(), flags->Value())));
}

Handle<Value> LZMAStream::AutoDecoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	
	Local<Integer> memlimit = Local<Integer>::Cast(args[0]);
	Local<Integer> flags = Local<Integer>::Cast(args[1]);
	
	return scope.Close(lzmaRet(lzma_auto_decoder(&self->_, memlimit->Value(), flags->Value())));
}

Handle<Value> LZMAStream::AloneDecoder(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(_failMissingSelf());
	
	Local<Integer> memlimit = Local<Integer>::Cast(args[0]);
	
	return scope.Close(lzmaRet(lzma_alone_decoder(&self->_, memlimit->Value())));
}

void moduleInit(Handle<Object> exports) {
	LZMAStream::Init(exports);
	
	exports->Set(String::NewSymbol("versionNumber"),            FunctionTemplate::New(lzmaVersionNumber)->GetFunction());
	exports->Set(String::NewSymbol("versionString"),            FunctionTemplate::New(lzmaVersionString)->GetFunction());
	exports->Set(String::NewSymbol("checkIsSupported"),         FunctionTemplate::New(lzmaCheckIsSupported)->GetFunction());
	exports->Set(String::NewSymbol("checkSize"),                FunctionTemplate::New(lzmaCheckSize)->GetFunction());
	exports->Set(String::NewSymbol("crc32"),                    FunctionTemplate::New(lzmaCRC32)->GetFunction());
	exports->Set(String::NewSymbol("filterEncoderIsSupported"), FunctionTemplate::New(lzmaFilterEncoderIsSupported)->GetFunction());
	exports->Set(String::NewSymbol("filterDecoderIsSupported"), FunctionTemplate::New(lzmaFilterDecoderIsSupported)->GetFunction());
	exports->Set(String::NewSymbol("rawEncoderMemusage"),       FunctionTemplate::New(lzmaRawEncoderMemusage)->GetFunction());
	exports->Set(String::NewSymbol("rawDecoderMemusage"),       FunctionTemplate::New(lzmaRawDecoderMemusage)->GetFunction());
	exports->Set(String::NewSymbol("mfIsSupported"),            FunctionTemplate::New(lzmaMfIsSupported)->GetFunction());
	exports->Set(String::NewSymbol("modeIsSupported"),          FunctionTemplate::New(lzmaModeIsSupported)->GetFunction());
	exports->Set(String::NewSymbol("easyEncoderMemusage"),      FunctionTemplate::New(lzmaEasyEncoderMemusage)->GetFunction());
	exports->Set(String::NewSymbol("easyDecoderMemusage"),      FunctionTemplate::New(lzmaEasyDecoderMemusage)->GetFunction());
	exports->Set(String::NewSymbol("streamBufferBound"),        FunctionTemplate::New(lzmaStreamBufferBound)->GetFunction());
	
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
}

NODE_MODULE(lzma, moduleInit)

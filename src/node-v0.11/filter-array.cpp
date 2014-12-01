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

FilterArray::FilterArray(Local<Array> arr) : ok_(false) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	
	uint32_t len = arr.IsEmpty() ? 0 : arr->Length();
	
	Local<String> id_ = String::NewFromUtf8(isolate, "id");
	Local<String> options_ = String::NewFromUtf8(isolate, "options");
	
	for (uint32_t i = 0; i < len; ++i) {
		Local<Object> entry = Local<Object>::Cast(arr->Get(i));
		if (entry.IsEmpty() || entry->IsUndefined()) {
			isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Filter array needs object entries")));
			return;
		}
		
		Local<String> id = Local<String>::Cast(entry->Get(id_));
		Local<Object> opt = Local<Object>::Cast(entry->Get(options_));
		
		lzma_filter f;
		f.id = FilterByName(id);
		f.options = NULL;
		if ((opt.IsEmpty() || opt->IsUndefined() || opt->IsNull()) &&
			(f.id != LZMA_FILTER_LZMA1 && f.id != LZMA_FILTER_LZMA2)) {
			filters.push_back(f);
			continue;
		}
		
		optbuf.push_back(options());
		union options& bopt = optbuf.back();
		
		switch (f.id) {
			case LZMA_FILTER_DELTA:
				bopt.delta.type = (lzma_delta_type) Local<Integer>::Cast(opt->Get(String::NewFromUtf8(isolate, "type")))->Value();
				bopt.delta.dist = Local<Integer>::Cast(opt->Get(String::NewFromUtf8(isolate, "dist")))->Value();
				f.options = &bopt.delta;
				break;
			case LZMA_FILTER_LZMA1:
			case LZMA_FILTER_LZMA2:
				bopt.lzma = parseOptionsLZMA(isolate, opt);
				f.options = &bopt.lzma;
				break;
			default:
				isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "LZMA Wrapper library understands .options only for DELTA and LZMA1, LZMA2 filters")));
				return;
		}
		
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

}

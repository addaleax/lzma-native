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

#include <sstream>

#ifdef WINDOWS
#include <windows.h>

namespace {
	typedef PHANDLE pipe_t;

	inline int pipeCreate(pipe_t[2] pipefd) {
		return CreatePipe(pipefd[0], pipefd[1], NULL, 0) != 0 ? -1 : 0;
	}

	inline int pipeRead(pipe_t file, void* buffer, size_t count) {
		return ReadFile(file, buffer, count, NULL, NULL) != 0  ? -1 : 0;
	}

	inline int pipeWrite(pipe_t file, const void* buffer, size_t count) {
		return WriteFile(file, buffer, count, NULL, NULL) != 0  ? -1 : 0;
	}

	inline int pipeClose(pipe_t file) {
		return CloseHandle(file) != 0  ? -1 : 0;
	}
}

#define pipeErrno GetLastError
#else
#include <unistd.h>
#include <cerrno>

namespace {
	typedef int pipe_t;
}

#define pipeCreate   pipe
#define pipeRead     read
#define pipeWrite    write
#define pipeClose    close
#define pipeErrno()  errno
#endif

namespace lzma {
namespace {
	struct asyncCodeThreadInfo {
		LZMAStream* self;
		pipe_t input, output;
	};
	
	extern "C" void worker(void* opaque) {
		asyncCodeThreadInfo* ti = static_cast<asyncCodeThreadInfo*>(opaque);
		LZMAStream* self = ti->self;
		self->asyncWorker(ti);
	}
}

void LZMAStream::asyncWorker(void* opaque) {
	asyncCodeThreadInfo* ti = static_cast<asyncCodeThreadInfo*>(opaque);
	pipe_t input = ti->input, output = ti->output;
	delete ti;
	
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
	
	lzma_action action = LZMA_RUN;

	std::vector<uint8_t> inbuf(bufsize), outbuf(bufsize);

	_.next_in = NULL;
	_.avail_in = 0;
	_.next_out = outbuf.data();
	_.avail_out = outbuf.size();
	bool inputEnd = false, failure = false;
	std::ostringstream errstream;

	while (!failure) {
		if (_.avail_in == 0 && !inputEnd) {
			_.next_in = inbuf.data();
			_.avail_in = 0;
			
			lock.unlock();
			int readBytes = pipeRead(input, inbuf.data(), inbuf.size());
			lock.lock();
			
			if (readBytes == 0) {
				inputEnd = true;
			} else if (readBytes == -1) {
				errstream << "Read error: " << pipeErrno() << "\n";
				error = errstream.str();
				
				inputEnd = true;
				failure = true;
			} else {
				_.avail_in = readBytes;
			}
			
			if (inputEnd)
				action = LZMA_FINISH;
		}

		lzma_ret ret = lzma_code(&_, action);

		if (_.avail_out == 0 || ret == LZMA_STREAM_END) {
			size_t writeBytes = outbuf.size() - _.avail_out;

			if (pipeWrite(output, outbuf.data(), writeBytes) == -1) {
				errstream << "Write error: " << pipeErrno() << "\n";
				error = errstream.str();
				failure = true;
			}

			_.next_out = outbuf.data();
			_.avail_out = outbuf.size();
		}

		if (ret != LZMA_OK) {
			if (ret == LZMA_STREAM_END)
				break;
			
			errstream << "LZMA coding error: " << lzmaStrError(ret) << "\n";
			error = errstream.str();
			failure = true;
		}
	}
	
	pipeClose(input);
	pipeClose(output);
}

Handle<Value> LZMAStream::AsyncCode(const Arguments& args) {
	HandleScope scope;
	
	LZMAStream* self = node::ObjectWrap::Unwrap<LZMAStream>(args.This());
	if (!self)
		return scope.Close(Undefined());
	
	LZMA_ASYNC_LOCK(self)
	LZMA_ASYNC_LOCK_LS(self)
	
	if (self->hasRunningThread) {
		ThrowException(Exception::Error(String::New("Can only have one running thread per stream")));
		return scope.Close(Undefined());
	}
	
	self->hasRunningThread = true;
	
	pipe_t toCompressor[2], fromCompressor[2];
	if (pipeCreate(toCompressor) == -1) {
	pipe_create_failure:
		ThrowException(Exception::Error(String::New("Could not create pipe")));
		return scope.Close(Undefined());
	}
	
	if (pipeCreate(fromCompressor) == -1) {
		pipeClose(toCompressor[0]);
		pipeClose(toCompressor[1]);
		
		goto pipe_create_failure;
	}
	
	asyncCodeThreadInfo* ti = new asyncCodeThreadInfo();
	ti->self = self;
	ti->input = toCompressor[0];
	ti->output = fromCompressor[1];
	
	uv_thread_t worker_id;
	uv_thread_create(&worker_id, worker, static_cast<void*>(ti));
	
	Local<Array> ret = Array::New(2);
	ret->Set(0, Integer::New(fromCompressor[0]));
	ret->Set(1, Integer::New(toCompressor[1]));
	return scope.Close(ret);
}

}

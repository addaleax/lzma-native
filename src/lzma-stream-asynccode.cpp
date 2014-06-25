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

#ifdef ASYNC_CODE_AVAILABLE

#include <thread>
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
struct ScopeGuard {
	std::function<void()> fn;
	~ScopeGuard() { fn(); }
};

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
	
	std::thread worker([] (LZMAStream* self, pipe_t input, pipe_t output) {
		LZMA_ASYNC_LOCK(self)
		LZMA_ASYNC_LOCK_LS(self)
		
		ScopeGuard marker = { [&self] () {
				self->hasRunningThread = false;
				self->lifespanCond.notify_all();
		} };
		
		lzma_stream* strm = &self->_;
		
		lzma_action action = LZMA_RUN;

		std::vector<uint8_t> inbuf(self->bufsize), outbuf(self->bufsize);

		strm->next_in = NULL;
		strm->avail_in = 0;
		strm->next_out = outbuf.data();
		strm->avail_out = outbuf.size();
		bool inputEnd = false, failure = false;
		std::ostringstream errstream;

		while (!failure) {
			if (strm->avail_in == 0 && !inputEnd) {
				strm->next_in = inbuf.data();
				strm->avail_in = 0;
				
				self->mutex.unlock();
				int readBytes = pipeRead(input, inbuf.data(), inbuf.size());
				self->mutex.lock();
				
				if (readBytes == 0) {
					inputEnd = true;
				} else if (readBytes == -1) {
					errstream << "Read error: " << pipeErrno() << "\n";
					self->error = errstream.str();
					
					inputEnd = true;
					failure = true;
				} else {
					strm->avail_in = readBytes;
				}
				
				if (inputEnd)
					action = LZMA_FINISH;
			}

			lzma_ret ret = lzma_code(strm, action);

			if (strm->avail_out == 0 || ret == LZMA_STREAM_END) {
				size_t writeBytes = outbuf.size() - strm->avail_out;

				if (pipeWrite(output, outbuf.data(), writeBytes) == -1) {
					errstream << "Write error: " << pipeErrno() << "\n";
					self->error = errstream.str();
					failure = true;
				}

				strm->next_out = outbuf.data();
				strm->avail_out = outbuf.size();
			}

			if (ret != LZMA_OK) {
				if (ret == LZMA_STREAM_END)
					break;
				
				errstream << "LZMA coding error: " << lzmaStrError(ret) << "\n";
				self->error = errstream.str();
				failure = true;
			}
		}
		
		pipeClose(input);
		pipeClose(output);
	}, self, toCompressor[0], fromCompressor[1]);
	
	worker.detach();
	
	Local<Array> ret = Array::New(2);
	ret->Set(0, Integer::New(fromCompressor[0]));
	ret->Set(1, Integer::New(toCompressor[1]));
	return scope.Close(ret);
}

}
#endif

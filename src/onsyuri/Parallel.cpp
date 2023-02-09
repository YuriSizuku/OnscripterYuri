/* -*- C++ -*-
*
*  Parallel.cpp
*
*  Copyright (C) 2018-2019 jh10001 <jh10001@live.cn>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#if defined(USE_PARALLEL)
#include "Parallel.h"

using namespace parallel;

ThreadPool parallel::threadPool;

namespace parallel {
    void ThreadPool::createThread() {
      assert(threadCreated < thread_num);
      Thread *thread = &threads[threadCreated];
      thread->threadData.sem = SDL_CreateSemaphore(0);
      thread->thread = SDL_CreateThread([](void *ptr) {
        auto *td = (Thread::ThreadData*)ptr;
        for (;;) {
          td->body(td->data);
          int status = Thread::Status::IDLE;
          SDL_AtomicSet(&td->status, status);
          while (status != Thread::Status::RUNNING) {
            if (status == Thread::Status::EXIT) goto exit_thread;
            SDL_SemWait(td->sem);
            status = SDL_AtomicGet(&td->status);
          }
        }
      exit_thread:
        SDL_DestroySemaphore(td->sem);
        return 0;
      }, "Parallel", &thread->threadData);
      SDL_DetachThread(thread->thread);
      ++threadCreated;
    }

    ThreadPool::~ThreadPool() {
      sync();
      for (int i = 0; i < threadCreated; ++i) {
        SDL_AtomicSet(&threads[i].threadData.status, Thread::Status::EXIT);
        SDL_SemPost(threads[i].threadData.sem);
      }
    }

    ThreadPool::Thread* ThreadPool::newThread(void(*func)(void *body), void* threadData) {
      assert(threadNum < thread_num);
      Thread *thread = &threads[threadNum];
      Thread::ThreadData &td = thread->threadData;
      td.body = func;
      td.data = threadData;
      SDL_AtomicSet(&td.status, Thread::Status::RUNNING);
      if (threadNum + 1 > threadCreated) createThread();
      else SDL_SemPost(td.sem);
      ++threadNum;
      return thread;
    }

    void ThreadPool::sync() {
      for (int i = threadNum - 1; i >= 0; --i) {
        while (SDL_AtomicGet(&threads[i].threadData.status) != Thread::Status::IDLE) SDL_Delay(0);
        --threadNum;
      }
    }
}
#endif

#include "nan.h"
#include "v8.h"
#include <node_buffer.h>
#include <stdint.h>
#include <sstream>

#include <aio.h>
#include <signal.h>

#define FIXED_ONE_BYTE_STR(str) \
  Nan::NewOneByteString(reinterpret_cast<const uint8_t*>(str), sizeof(str) - 1).ToLocalChecked()

#include <iostream>

class AioReadResource : public Nan::AsyncResource {
  public:
    AioReadResource(v8::Local<v8::Function> cb, v8::Local<v8::Object> _output) : AsyncResource("aio:read") {
      callback.Reset(cb);
      output.Reset(_output);
    }
    ~AioReadResource() {
      callback.Reset();
      output.Reset();
    }
    Nan::Callback callback;
    Nan::Persistent<v8::Object> output;
    aiocb req;
    int status = 0;
};

void CheckWork(uv_work_t* uvwork) {
  AioReadResource* resource = static_cast<AioReadResource*>(uvwork->data);
  resource->status = aio_error(&resource->req);
}

void AfterWork(uv_work_t* uvwork, int status) {
  AioReadResource* resource = static_cast<AioReadResource*>(uvwork->data);
  if (resource->status == EINPROGRESS) {
    uv_work_t* work = new uv_work_t();
    work->data = resource;
    uv_queue_work(Nan::GetCurrentEventLoop(), work, CheckWork, AfterWork);

  } else if (resource->status == 0) {
    Nan::HandleScope scope;
    v8::Local<v8::Value> argv[2] = { Nan::Null(), Nan::New(resource->output) };
    resource->callback.Call(2, argv, resource);
    delete resource;
    delete uvwork;

  } else {
    Nan::HandleScope scope;
    v8::Local<v8::Value> argv[1] = { Nan::ErrnoException(resource->status) };
    resource->callback.Call(1, argv, resource);
    delete resource;
    delete uvwork;

  }
}

NAN_METHOD(readFile) {
  int32_t fd = Nan::To<int32_t>(info[0]).FromJust();
  uint32_t len = Nan::To<uint32_t>(info[1]).FromJust();
  v8::Local<v8::Function> cb = Nan::To<v8::Function>(info[2]).ToLocalChecked();
  v8::Local<v8::Object> output = node::Buffer::New(v8::Isolate::GetCurrent(), len).ToLocalChecked();

  AioReadResource* resource = new AioReadResource(cb, output);

  resource->req.aio_fildes = fd;
  resource->req.aio_offset = 0;
  resource->req.aio_buf = node::Buffer::Data(output);
  resource->req.aio_nbytes = len;
  resource->req.aio_reqprio = 0;
  resource->req.aio_sigevent.sigev_notify = SIGEV_NONE;

  int status = aio_read(&resource->req);
  if (status) {
    info.GetReturnValue().Set(Nan::ErrnoException(errno, "aio_read"));
    return;
  }

  uv_work_t* work = new uv_work_t();
  work->data = resource;
  uv_queue_work(Nan::GetCurrentEventLoop(), work, CheckWork, AfterWork);
}

NAN_MODULE_INIT(Init) {
  NAN_EXPORT(target, readFile);
}

NODE_MODULE(aio, Init);

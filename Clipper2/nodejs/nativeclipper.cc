#include <node.h>
#include <node_object_wrap.h>
#include <string>
#include <math.h>
#include "../CPP/clipper.h"

namespace nativeclipper {
  using v8::Context;
  using v8::ArrayBuffer;
  using v8::Function;
  using v8::FunctionCallbackInfo;
  using v8::FunctionTemplate;
  using v8::Isolate;
  using v8::Local;
  using v8::MaybeLocal;
  using v8::Number;
  using v8::Object;
  using v8::Persistent;
  using v8::String;
  using v8::Value;
  using v8::Array;
  using v8::String;
  using v8::Exception;
  using v8::Undefined;
  using v8::NewStringType;
  using v8::Float64Array;
  using v8::Float32Array;
  using v8::Int32Array;


  using clipperlib::Point64;
  using clipperlib::Path;
  using clipperlib::Paths;
  using clipperlib::PathType;

  static void handle_exception(
    const FunctionCallbackInfo<Value>& args,
    const char* error_message) {
    Isolate* isolate = args.GetIsolate();
    auto e = Exception::TypeError(String::NewFromUtf8(isolate, error_message));
    int last = (int)(args.Length() - 1);
    if (args[last]->IsFunction()) {
      Local<Function> cb = Local<Function>::Cast(args[last]);
      const unsigned argc = 2;
      Local<Value> argv[argc] = { 
          Local<Value>::New(isolate, e),
          Local<Value>::New(isolate, Undefined(isolate)) };
      cb->Call(Null(isolate), argc, argv);
    } else {
      isolate->ThrowException(e);
    }
    args.GetReturnValue().Set(Undefined(isolate));
  }

  class Clipper : public node::ObjectWrap {
  public:
    static void Init(Local<Object> exports);
  private:
    explicit Clipper(double precision_multiplier, Isolate* isolate);
    ~Clipper();

    void addPath(
      const FunctionCallbackInfo<Value>& args,
      Local<Value> path,
      Local<Value> path_type,
      bool is_open);

    static void New(const FunctionCallbackInfo<Value>& args);
    static void Precision(const FunctionCallbackInfo<Value>& args);
    static void AddPath(const FunctionCallbackInfo<Value>& args);
    static void AddPaths(const FunctionCallbackInfo<Value>& args);
    static void Execute(const FunctionCallbackInfo<Value>& args);
    static Persistent<Function> constructor;

    clipperlib::Clipper clipper_;
    double precision_multiplier_;
  };

  Persistent<Function> Clipper::constructor;

  Clipper::Clipper(double value, Isolate* isolate)
    : precision_multiplier_(value) {
  }

  Clipper::~Clipper() {
  }

  void Clipper::Init(Local<Object> exports) {
    Isolate* isolate = exports->GetIsolate();

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "Clipper"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "precision", Precision);
    NODE_SET_PROTOTYPE_METHOD(tpl, "addPath", AddPath);
    NODE_SET_PROTOTYPE_METHOD(tpl, "addPaths", AddPaths);
    NODE_SET_PROTOTYPE_METHOD(tpl, "execute", Execute);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "Clipper"), tpl->GetFunction());
  }

  void Clipper::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    if (args.IsConstructCall()) {
      // Invoked as constructor: `new Clipper(...)`
      if (!args[0]->IsNumber()) {
        handle_exception(args, "Wrong type for argument 1: expected number");
        return;
      }
      double value = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
      Clipper* obj = new Clipper(value, isolate);
      obj->Wrap(args.This());
      args.GetReturnValue().Set(args.This());
    } else {
      // Invoked as plain function `Clipper(...)`, turn into construct call.
      const int argc = 1;
      Local<Value> argv[argc] = { args[0] };
      Local<Context> context = isolate->GetCurrentContext();
      Local<Function> cons = Local<Function>::New(isolate, constructor);
      MaybeLocal<Object> result = cons->NewInstance(context, argc, argv);
      if (!result.IsEmpty()) {
        args.GetReturnValue().Set(result.ToLocalChecked());
      } else {
        args.GetReturnValue().Set(Undefined(isolate));
      }
    }
  }

  void Clipper::Precision(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Clipper* obj = ObjectWrap::Unwrap<Clipper>(args.Holder());
    args.GetReturnValue().Set(Number::New(isolate, obj->precision_multiplier_));
  }

  void Clipper::AddPath(const FunctionCallbackInfo<Value>& args) {
    if (args.Length() < 2) {
        handle_exception(args, "Expected at least 2 arguments");
        return;
    }
    Clipper* obj = ObjectWrap::Unwrap<Clipper>(args.Holder());
    Isolate* isolate = args.GetIsolate();
    Local<Value> path = args[0];
    Local<Value> path_type = args[1];
    bool is_open = (args.Length() > 2) ? args[2]->BooleanValue() : false;
    obj->addPath(args, path, path_type, is_open);
    args.GetReturnValue().Set(Undefined(isolate));
  }

  void Clipper::addPath(
      const FunctionCallbackInfo<Value>& args,
      Local<Value> path_value,
      Local<Value> path_type_value,
      bool is_open) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    Path clipper_path;
    if (path_value->IsArray()) {
      Local<Array> path = Local<Array>::Cast(path_value);
      uint32_t len = path->Length();
      if (len == 0) {
        handle_exception(args, "Argument 1 is empty path.");
        return;
      }
      bool has_numbers = path->Get(0)->IsNumber();
      clipper_path.reserve(has_numbers ? len / 2 : len);
      for (uint32_t idx = 0; idx < len; idx++) {
        Local<Value> array_value = path->Get(idx);
        double x;
        double y;
        if (has_numbers) {
          x = array_value->NumberValue();
          if (idx + 1 >= len) {
            handle_exception(args, "Array of numbers has to be even length.");
            return;
          }
          y = path->Get(idx + 1)->NumberValue();
          idx++;
        } else if (array_value->IsObject()) {
          Local<Object> object = Local<Object>::Cast(array_value);
          x = object->Get(context, String::NewFromUtf8(isolate, "x"))
            .ToLocalChecked()->NumberValue();
          y = object->Get(context, String::NewFromUtf8(isolate, "y"))
            .ToLocalChecked()->NumberValue();
        } else {
          handle_exception(args, "Unsupported path array element type.");
          return;
        }
        if (isnan(x) || isnan(y)) {
          handle_exception(args, "Either x or y value is NaN.");
          return;
        }
        clipper_path.push_back(
          Point64(x * precision_multiplier_, y * precision_multiplier_));
      }
    } else if (path_value->IsFloat64Array()) {
      Local<Float64Array> path = Local<Float64Array>::Cast(path_value);
      ArrayBuffer::Contents contents = path->Buffer()->GetContents();
      const double* start = reinterpret_cast<double*>(contents.Data());
      const double* end = start + path->Length();
      clipper_path.reserve(path->Length() / 2);
      while (start < end) {
        double x = *start++;
        double y = *start++;
        clipper_path.push_back(
          Point64(x * precision_multiplier_, y * precision_multiplier_));
      }
    } else if (path_value->IsFloat32Array()) {
      Local<Float32Array> path = Local<Float32Array>::Cast(path_value);
      ArrayBuffer::Contents contents = path->Buffer()->GetContents();
      const float* start = reinterpret_cast<float*>(contents.Data());
      const float* end = start + path->Length();
      clipper_path.reserve(path->Length() / 2);
      while (start < end) {
        double x = *start++;
        double y = *start++;
        clipper_path.push_back(
          Point64(x * precision_multiplier_, y * precision_multiplier_));
      }
    } else if (path_value->IsInt32Array()) {
      Local<Int32Array> path = Local<Int32Array>::Cast(path_value);
      ArrayBuffer::Contents contents = path->Buffer()->GetContents();
      const int32_t* start = reinterpret_cast<int32_t*>(contents.Data());
      const int32_t* end = start + path->Length();
      clipper_path.reserve(path->Length() / 2);
      while (start < end) {
        double x = *start++;
        double y = *start++;
        clipper_path.push_back(
          Point64(x * precision_multiplier_, y * precision_multiplier_));
      }
    } else {
        handle_exception(args, "Invalid type for argument 1. Expected Array, Float64Array, Float32Array or Int32Array.");
        return;
    }
    if (!path_type_value->IsString()) {
        handle_exception(args, "Invalid type for argument 2. Expected string.");
        return;
    }
    Local<String> path_type_str = Local<String>::Cast(path_type_value);
    PathType path_type;
    if (path_type_str->Equals(context, String::NewFromUtf8(isolate, "subject")).ToChecked()) {
      path_type = clipperlib::ptSubject;
    } else if (path_type_str->Equals(context, String::NewFromUtf8(isolate, "clip")).ToChecked()) {
      path_type = clipperlib::ptClip;
    } else {
      handle_exception(args, "Invalid path type. Has to be 'subject' or 'clip'.");
      return;
    }
    clipper_.AddPath(clipper_path, path_type, is_open);
  }

  void Clipper::AddPaths(const FunctionCallbackInfo<Value>& args) {
    handle_exception(args, "Not implemented");
  }

  void Clipper::Execute(const FunctionCallbackInfo<Value>& args) {
    handle_exception(args, "Not implemented");
  }

  void InitModule(Local<Object> exports) {
    Clipper::Init(exports);
  }

  NODE_MODULE(NODE_GYP_MODULE_NAME, InitModule);
} // namespace nativeclipper

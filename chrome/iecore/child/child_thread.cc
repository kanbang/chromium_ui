// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/iecore/child/child_thread.h"

#include "base/allocator/allocator_extension.h"
#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/lazy_instance.h"
#include "base/message_loop/message_loop.h"
#include "base/process/kill.h"
#include "base/process/process_handle.h"
#include "base/strings/string_util.h"
#include "base/synchronization/condition_variable.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread_local.h"
#include "base/tracked_objects.h"
#include "chrome/iecore/child/child_process.h"
#include "ipc/ipc_logging.h"
#include "ipc/ipc_switches.h"
#include "ipc/ipc_sync_channel.h"
#include "ipc/ipc_sync_message_filter.h"

#include "chrome/iecore/child/render_view.h"
#include "chrome/common/child_process_messages.h"
#include "chrome/iecore/trident/WebBrowserHost/IEBrowserWnd.h"
#include "chrome/iecore/iecore_message.h"

#if defined(OS_WIN)
//#include "content/common/handle_enumerator_win.h"
#endif

#if defined(TCMALLOC_TRACE_MEMORY_SUPPORTED)
#include "third_party/tcmalloc/chromium/src/gperftools/heap-profiler.h"
#endif

using tracked_objects::ThreadData;
base::Lock ipc_lock_;
namespace content {
namespace {

// How long to wait for a connection to the browser process before giving up.
const int kConnectionTimeoutS = 15;

base::LazyInstance<base::ThreadLocalPointer<ChildThread> > g_lazy_tls =
    LAZY_INSTANCE_INITIALIZER;

// This isn't needed on Windows because there the sandbox's job object
// terminates child processes automatically. For unsandboxed processes (i.e.
// plugins), PluginThread has EnsureTerminateMessageFilter.
#if defined(OS_POSIX)

class SuicideOnChannelErrorFilter : public IPC::ChannelProxy::MessageFilter {
 public:
  // IPC::ChannelProxy::MessageFilter
  virtual void OnChannelError() OVERRIDE {
    // For renderer/worker processes:
    // On POSIX, at least, one can install an unload handler which loops
    // forever and leave behind a renderer process which eats 100% CPU forever.
    //
    // This is because the terminate signals (ViewMsg_ShouldClose and the error
    // from the IPC channel) are routed to the main message loop but never
    // processed (because that message loop is stuck in V8).
    //
    // One could make the browser SIGKILL the renderers, but that leaves open a
    // large window where a browser failure (or a user, manually terminating
    // the browser because "it's stuck") will leave behind a process eating all
    // the CPU.
    //
    // So, we install a filter on the channel so that we can process this event
    // here and kill the process.
    if (CommandLine::ForCurrentProcess()->
        HasSwitch(switches::kChildCleanExit)) {
      // If clean exit is requested, we want to kill this process after giving
      // it 60 seconds to run exit handlers. Exit handlers may including ones
      // that write profile data to disk (which happens under profile collection
      // mode).
      alarm(60);
    } else {
      _exit(0);
    }
  }

 protected:
  virtual ~SuicideOnChannelErrorFilter() {}
};

#endif  // OS(POSIX)

#if defined(OS_ANDROID)
ChildThread* g_child_thread = NULL;

// A lock protects g_child_thread.
base::LazyInstance<base::Lock> g_lazy_child_thread_lock =
    LAZY_INSTANCE_INITIALIZER;

// base::ConditionVariable has an explicit constructor that takes
// a base::Lock pointer as parameter. The base::DefaultLazyInstanceTraits
// doesn't handle the case. Thus, we need our own class here.
struct CondVarLazyInstanceTraits {
  static const bool kRegisterOnExit = true;
  static const bool kAllowedToAccessOnNonjoinableThread = false;
  static base::ConditionVariable* New(void* instance) {
    return new (instance) base::ConditionVariable(
        g_lazy_child_thread_lock.Pointer());
  }
  static void Delete(base::ConditionVariable* instance) {
    instance->~ConditionVariable();
  }
};

// A condition variable that synchronize threads initializing and waiting
// for g_child_thread.
base::LazyInstance<base::ConditionVariable, CondVarLazyInstanceTraits>
    g_lazy_child_thread_cv = LAZY_INSTANCE_INITIALIZER;

void QuitMainThreadMessageLoop() {
  base::MessageLoop::current()->Quit();
}

#endif

}  // namespace

HWND ChildThread::s_wndParent = NULL;
ChildThread::ChildThread()
    : channel_connected_factory_(this),
      m_pBack(NULL){
  channel_name_ = CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
      switches::kProcessChannelID);
  Init();
}

ChildThread::ChildThread(const std::string& channel_name)
    : channel_name_(channel_name),
      channel_connected_factory_(this),
      m_pBack(NULL){
  Init();
}

void ChildThread::Init() {
  g_lazy_tls.Pointer()->Set(this);
  on_channel_error_called_ = false;
  message_loop_ = base::MessageLoop::current();
#ifdef IPC_MESSAGE_LOG_ENABLED
  // We must make sure to instantiate the IPC Logger *before* we create the
  // channel, otherwise we can get a callback on the IO thread which creates
  // the logger, and the logger does not like being created on the IO thread.
  IPC::Logging::GetInstance();
#endif
  channel_.reset(
      new IPC::SyncChannel(channel_name_,
                           IPC::Channel::MODE_CLIENT,
                           this,
                           ChildProcess::current()->io_message_loop_proxy(),
                           true,
                           ChildProcess::current()->GetShutDownEvent()));
#ifdef IPC_MESSAGE_LOG_ENABLED
  IPC::Logging::GetInstance()->SetIPCSender(this);
#endif

  /*sync_message_filter_ =
      new IPC::SyncMessageFilter(ChildProcess::current()->GetShutDownEvent());
  thread_safe_sender_ = new ThreadSafeSender(
      base::MessageLoopProxy::current().get(), sync_message_filter_.get());

  resource_dispatcher_.reset(new ResourceDispatcher(this));
  socket_stream_dispatcher_.reset(new SocketStreamDispatcher());
  websocket_dispatcher_.reset(new WebSocketDispatcher);
  file_system_dispatcher_.reset(new FileSystemDispatcher());

  histogram_message_filter_ = new ChildHistogramMessageFilter();
  resource_message_filter_ =
      new ChildResourceMessageFilter(resource_dispatcher());

  quota_message_filter_ =
      new QuotaMessageFilter(thread_safe_sender_.get());
  quota_dispatcher_.reset(new QuotaDispatcher(thread_safe_sender_.get(),
                                              quota_message_filter_.get()));

  channel_->AddFilter(histogram_message_filter_.get());
  channel_->AddFilter(sync_message_filter_.get());
  channel_->AddFilter(new tracing::ChildTraceMessageFilter(
      ChildProcess::current()->io_message_loop_proxy()));
  channel_->AddFilter(resource_message_filter_.get());
  channel_->AddFilter(quota_message_filter_.get());

  // In single process mode we may already have a power monitor
  if (!base::PowerMonitor::Get()) {
    scoped_ptr<PowerMonitorBroadcastSource> power_monitor_source(
      new PowerMonitorBroadcastSource());
    channel_->AddFilter(power_monitor_source->GetMessageFilter());

    power_monitor_.reset(new base::PowerMonitor(
        power_monitor_source.PassAs<base::PowerMonitorSource>()));
  }*/

#if defined(OS_POSIX)
  // Check that --process-type is specified so we don't do this in unit tests
  // and single-process mode.
  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kProcessType))
    channel_->AddFilter(new SuicideOnChannelErrorFilter());
#endif

  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kTraceToConsole)) {
    std::string category_string =
        CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
            switches::kTraceToConsole);

    if (!category_string.size())
      category_string = "*";

    base::debug::TraceLog::GetInstance()->SetEnabled(
        base::debug::CategoryFilter(category_string),
        base::debug::TraceLog::ECHO_TO_CONSOLE);
  }

  base::MessageLoop::current()->PostDelayedTask(
      FROM_HERE,
      base::Bind(&ChildThread::EnsureConnected,
                 channel_connected_factory_.GetWeakPtr()),
      base::TimeDelta::FromSeconds(kConnectionTimeoutS));

//   base::MessageLoop::current()->PostDelayedTask(
//     FROM_HERE,
//     base::Bind(&ChildThread::TestMsg,
//     base::Unretained(this)),
//     base::TimeDelta::FromSeconds(kConnectionTimeoutS));

#if defined(OS_ANDROID)
  {
    base::AutoLock lock(g_lazy_child_thread_lock.Get());
    g_child_thread = this;
  }
  // Signalling without locking is fine here because only
  // one thread can wait on the condition variable.
  g_lazy_child_thread_cv.Get().Signal();
#endif

#if defined(TCMALLOC_TRACE_MEMORY_SUPPORTED)
  trace_memory_controller_.reset(new base::debug::TraceMemoryController(
      message_loop_->message_loop_proxy(),
      ::HeapProfilerWithPseudoStackStart,
      ::HeapProfilerStop,
      ::GetHeapProfile));
#endif

  RECT rc = {0};
  s_wndParent = XWnd::Create( WS_EX_NOPARENTNOTIFY, L"Container", L"main", WS_POPUP, &rc, NULL, 0 );
}

ChildThread::~ChildThread() {
#ifdef IPC_MESSAGE_LOG_ENABLED
  IPC::Logging::GetInstance()->SetIPCSender(NULL);
#endif

  //channel_->RemoveFilter(quota_message_filter_.get());
  //channel_->RemoveFilter(histogram_message_filter_.get());
  //channel_->RemoveFilter(sync_message_filter_.get());

  // The ChannelProxy object caches a pointer to the IPC thread, so need to
  // reset it as it's not guaranteed to outlive this object.
  // NOTE: this also has the side-effect of not closing the main IPC channel to
  // the browser process.  This is needed because this is the signal that the
  // browser uses to know that this process has died, so we need it to be alive
  // until this process is shut down, and the OS closes the handle
  // automatically.  We used to watch the object handle on Windows to do this,
  // but it wasn't possible to do so on POSIX.
  channel_->ClearIPCTaskRunner();
  g_lazy_tls.Pointer()->Set(NULL);
}

void ChildThread::Shutdown() {
  // Delete objects that hold references to blink so derived classes can
  // safely shutdown blink in their Shutdown implementation.
  //file_system_dispatcher_.reset();
}

void ChildThread::OnChannelConnected(int32 peer_pid) {
  channel_connected_factory_.InvalidateWeakPtrs();
}

void ChildThread::OnChannelError() {
  set_on_channel_error_called(true);
  base::MessageLoop::current()->Quit();
}

bool ChildThread::Send(IPC::Message* msg) {
  //DCHECK(base::MessageLoop::current() == message_loop());
  base::AutoLock lock(ipc_lock_);
  if (!channel_) {
    delete msg;
    return false;
  }

  return channel_->Send(msg);
}

void ChildThread::AddRoute(int32 routing_id, IPC::Listener* listener) {
  DCHECK(base::MessageLoop::current() == message_loop());

  router_.AddRoute(routing_id, listener);
}

void ChildThread::RemoveRoute(int32 routing_id) {
  DCHECK(base::MessageLoop::current() == message_loop());

  router_.RemoveRoute(routing_id);
}

void ChildThread::AddRenderViewRoute(int routing_id, RenderView* rv) {
  render_view_routes_.AddWithID(rv, routing_id);
}

void ChildThread::RemoveRenderViewRoute(int routing_id) {
  render_view_routes_.Remove(routing_id);
}

// webkit_glue::ResourceLoaderBridge* ChildThread::CreateBridge(
//     const webkit_glue::ResourceLoaderBridge::RequestInfo& request_info) {
//   return resource_dispatcher()->CreateBridge(request_info);
// }

base::SharedMemory* ChildThread::AllocateSharedMemory(size_t buf_size) {
  return AllocateSharedMemory(buf_size, this);
}

// static
base::SharedMemory* ChildThread::AllocateSharedMemory(
    size_t buf_size,
    IPC::Sender* sender) {
  scoped_ptr<base::SharedMemory> shared_buf;
#if defined(OS_WIN)
  shared_buf.reset(new base::SharedMemory);
  if (!shared_buf->CreateAndMapAnonymous(buf_size)) {
    NOTREACHED();
    return NULL;
  }
#else
  // On POSIX, we need to ask the browser to create the shared memory for us,
  // since this is blocked by the sandbox.
  base::SharedMemoryHandle shared_mem_handle;
  if (sender->Send(new ChildProcessHostMsg_SyncAllocateSharedMemory(
                           buf_size, &shared_mem_handle))) {
    if (base::SharedMemory::IsHandleValid(shared_mem_handle)) {
      shared_buf.reset(new base::SharedMemory(shared_mem_handle, false));
      if (!shared_buf->Map(buf_size)) {
        NOTREACHED() << "Map failed";
        return NULL;
      }
    } else {
      NOTREACHED() << "Browser failed to allocate shared memory";
      return NULL;
    }
  } else {
    NOTREACHED() << "Browser allocation request message failed";
    return NULL;
  }
#endif
  return shared_buf.release();
}

bool ChildThread::OnMessageReceived(const IPC::Message& msg) {
  // Resource responses are sent to the resource dispatcher.
//   if (resource_dispatcher_->OnMessageReceived(msg))
//     return true;
//   if (socket_stream_dispatcher_->OnMessageReceived(msg))
//     return true;
//   if (websocket_dispatcher_->OnMessageReceived(msg))
//     return true;
//   if (file_system_dispatcher_->OnMessageReceived(msg))
//     return true;

  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ChildThread, msg)
    IPC_MESSAGE_HANDLER(ViewMsg_New, OnCreateNewView) 
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  if (handled)
    return true;

  if (msg.routing_id() == MSG_ROUTING_CONTROL)
    return OnControlMessageReceived(msg);

  return router_.OnMessageReceived(msg);
}

bool ChildThread::OnBrowserMessageReceive(const IPC::Message& msg) {
  bool handle = false;
  IPC_BEGIN_MESSAGE_MAP(ChildThread, msg)
    IPC_MESSAGE_HANDLER(Browser_Ready, Browser_OnReady)
    IPC_MESSAGE_UNHANDLED(handle = false)
  IPC_END_MESSAGE_MAP()
  if(handle)
    return true;

  if(msg.routing_id() != MSG_ROUTING_CONTROL) {
    RenderView* pView = render_view_routes_.Lookup(msg.routing_id());
    if(pView) {
      pView->OnBrowserMessageReceived(msg);
    }
  }
  return handle;
}

bool ChildThread::OnControlMessageReceived(const IPC::Message& msg) {
  return false;
}

void ChildThread::OnCreateNewView(const ViewMsg_New_Params& params) {
  // When bringing in render_view, also bring in webkit's glue and jsbindings.
  if(params.opener_route_id > 0) {
    RenderView* pView = render_view_routes_.Lookup(params.opener_route_id);
    if(pView) {
      pView->BindTab(params.hwnd, params.view_id);
    }
  } else {
    RenderView* pView = RenderView::Create(
      this, params.hwnd, MSG_ROUTING_NONE, new SharedRenderViewCounter(0), params.view_id,
      params.main_frame_routing_id,
      params.next_page_id);
  }
}

void ChildThread::Browser_OnNewWindow(Browser_NewWindow_Params* params) {
  RenderView* pView = RenderView::Create(
    this, m_hWnd, MSG_ROUTING_NONE, new SharedRenderViewCounter(0), 0,
    0,
    0,
    TRUE);

  if(pView) {
    pView->SetNewWindowParams(params);
    RenderView* parent_view = render_view_routes_.Lookup(params->browser_id);
    if(parent_view)
      Send(new ViewMsg_CreateNewWindow(parent_view->route_id(), pView->GetBrowserId()));
  }
}

void ChildThread::Browser_OnReady(const Browser_Ready_Params& params) {
  RenderView* pView = render_view_routes_.Lookup(params.roateid);
  if(pView) {
    pView->SetBrowserWnd(params.hwnd);
  }
}

void ChildThread::OnShutdown() {
  base::MessageLoop::current()->Quit();
}

#if defined(IPC_MESSAGE_LOG_ENABLED)
void ChildThread::OnSetIPCLoggingEnabled(bool enable) {
  if (enable)
    IPC::Logging::GetInstance()->Enable();
  else
    IPC::Logging::GetInstance()->Disable();
}
#endif  //  IPC_MESSAGE_LOG_ENABLED

void ChildThread::OnSetProfilerStatus(ThreadData::Status status) {
  ThreadData::InitializeAndSetTrackingStatus(status);
}

void ChildThread::OnGetChildProfilerData(int sequence_number) {
  tracked_objects::ProcessDataSnapshot process_data;
  ThreadData::Snapshot(false, &process_data);

  //Send(new ChildProcessHostMsg_ChildProfilerData(sequence_number,
  //                                               process_data));
}

void ChildThread::OnDumpHandles() {

  NOTIMPLEMENTED();
}

#if defined(USE_TCMALLOC)
void ChildThread::OnGetTcmallocStats() {
  std::string result;
  char buffer[1024 * 32];
  base::allocator::GetStats(buffer, sizeof(buffer));
  result.append(buffer);
  //Send(new ChildProcessHostMsg_TcmallocStats(result));
}
#endif

ChildThread* ChildThread::current() {
  return g_lazy_tls.Pointer()->Get();
}

#if defined(OS_ANDROID)
// The method must NOT be called on the child thread itself.
// It may block the child thread if so.
void ChildThread::ShutdownThread() {
  DCHECK(!ChildThread::current()) <<
      "this method should NOT be called from child thread itself";
  {
    base::AutoLock lock(g_lazy_child_thread_lock.Get());
    while (!g_child_thread)
      g_lazy_child_thread_cv.Get().Wait();
  }
  DCHECK_NE(base::MessageLoop::current(), g_child_thread->message_loop());
  g_child_thread->message_loop()->PostTask(
      FROM_HERE, base::Bind(&QuitMainThreadMessageLoop));
}

#endif

void ChildThread::OnProcessFinalRelease() {
  if (on_channel_error_called_) {
    base::MessageLoop::current()->Quit();
    return;
  }

  // The child process shutdown sequence is a request response based mechanism,
  // where we send out an initial feeler request to the child process host
  // instance in the browser to verify if it's ok to shutdown the child process.
  // The browser then sends back a response if it's ok to shutdown. This avoids
  // race conditions if the process refcount is 0 but there's an IPC message
  // inflight that would addref it.
  //Send(new ChildProcessHostMsg_ShutdownRequest);
}

void ChildThread::EnsureConnected() {
  LOG(INFO) << "ChildThread::EnsureConnected()";
  base::KillProcess(base::GetCurrentProcessHandle(), 0, false);
}

void ChildThread::TestMsg() {
  Send(new ChildProcessHostMsg_ShutdownRequest());
}

LRESULT __stdcall ChildThread::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  if(message == BROWSER_TO_CHILDTHREAD_IPC_MESSAGE) {
    IPC::Message* pMsg = (IPC::Message*)lParam;
    if(pMsg) {
      OnBrowserMessageReceive(*pMsg);
      delete pMsg;
    }
    return 0;
  }
  if(BROWSER_TO_CHILDTHREAD_NEWWINDOW == message) {
    Browser_NewWindow_Params* params = (Browser_NewWindow_Params*)lParam;
    if(params)
    Browser_OnNewWindow(params);
    return 0;
  }
  return ::DefWindowProc(hWnd, message, wParam, lParam);
}
}  // namespace content

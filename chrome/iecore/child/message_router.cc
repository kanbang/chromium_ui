// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/iecore/child/message_router.h"

#include "ipc/ipc_message.h"

namespace content {

MessageRouter::MessageRouter() {
}

MessageRouter::~MessageRouter() {
}

bool MessageRouter::OnControlMessageReceived(const IPC::Message& msg) {
  NOTREACHED() <<
      "should override in subclass if you care about control messages";
  return false;
}

bool MessageRouter::Send(IPC::Message* msg) {
  NOTREACHED() <<
      "should override in subclass if you care about sending messages";
  return false;
}

void MessageRouter::AddRoute(int32 routing_id, IPC::Listener* listener) {
  routes_.AddWithID(listener, routing_id);
}

void MessageRouter::RemoveRoute(int32 routing_id) {
  routes_.Remove(routing_id);
}

IPC::Listener* MessageRouter::ResolveRoute(int32 routing_id) {
  return routes_.Lookup(routing_id);
}

bool MessageRouter::OnMessageReceived(const IPC::Message& msg) {
  if (msg.routing_id() == MSG_ROUTING_CONTROL)
    return OnControlMessageReceived(msg);

  return RouteMessage(msg);
}

bool MessageRouter::RouteMessage(const IPC::Message& msg) {
  IPC::Listener* listener = routes_.Lookup(msg.routing_id());
  if (!listener)
    return false;

  listener->OnMessageReceived(msg);
  return true;
}

}  // namespace content

/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.35
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.freeswitch.esl;

class eslJNI {
  public final static native void ESLevent_event_set(long jarg1, ESLevent jarg1_, long jarg2);
  public final static native long ESLevent_event_get(long jarg1, ESLevent jarg1_);
  public final static native void ESLevent_serialized_string_set(long jarg1, ESLevent jarg1_, String jarg2);
  public final static native String ESLevent_serialized_string_get(long jarg1, ESLevent jarg1_);
  public final static native void ESLevent_mine_set(long jarg1, ESLevent jarg1_, int jarg2);
  public final static native int ESLevent_mine_get(long jarg1, ESLevent jarg1_);
  public final static native long new_ESLevent__SWIG_0(String jarg1, String jarg2);
  public final static native long new_ESLevent__SWIG_1(long jarg1, int jarg2);
  public final static native long new_ESLevent__SWIG_2(long jarg1, ESLevent jarg1_);
  public final static native void delete_ESLevent(long jarg1);
  public final static native String ESLevent_serialize(long jarg1, ESLevent jarg1_, String jarg2);
  public final static native boolean ESLevent_setPriority(long jarg1, ESLevent jarg1_, long jarg2);
  public final static native String ESLevent_getHeader(long jarg1, ESLevent jarg1_, String jarg2);
  public final static native String ESLevent_getBody(long jarg1, ESLevent jarg1_);
  public final static native String ESLevent_getType(long jarg1, ESLevent jarg1_);
  public final static native boolean ESLevent_addBody(long jarg1, ESLevent jarg1_, String jarg2);
  public final static native boolean ESLevent_addHeader(long jarg1, ESLevent jarg1_, String jarg2, String jarg3);
  public final static native boolean ESLevent_delHeader(long jarg1, ESLevent jarg1_, String jarg2);
  public final static native String ESLevent_firstHeader(long jarg1, ESLevent jarg1_);
  public final static native String ESLevent_nextHeader(long jarg1, ESLevent jarg1_);
  public final static native long new_ESLconnection__SWIG_0(String jarg1, String jarg2, String jarg3, String jarg4);
  public final static native long new_ESLconnection__SWIG_1(String jarg1, String jarg2, String jarg3);
  public final static native long new_ESLconnection__SWIG_2(int jarg1);
  public final static native void delete_ESLconnection(long jarg1);
  public final static native int ESLconnection_socketDescriptor(long jarg1, ESLconnection jarg1_);
  public final static native int ESLconnection_connected(long jarg1, ESLconnection jarg1_);
  public final static native long ESLconnection_getInfo(long jarg1, ESLconnection jarg1_);
  public final static native int ESLconnection_send(long jarg1, ESLconnection jarg1_, String jarg2);
  public final static native long ESLconnection_sendRecv(long jarg1, ESLconnection jarg1_, String jarg2);
  public final static native long ESLconnection_api(long jarg1, ESLconnection jarg1_, String jarg2, String jarg3);
  public final static native long ESLconnection_bgapi(long jarg1, ESLconnection jarg1_, String jarg2, String jarg3);
  public final static native long ESLconnection_sendEvent(long jarg1, ESLconnection jarg1_, long jarg2, ESLevent jarg2_);
  public final static native long ESLconnection_recvEvent(long jarg1, ESLconnection jarg1_);
  public final static native long ESLconnection_recvEventTimed(long jarg1, ESLconnection jarg1_, int jarg2);
  public final static native long ESLconnection_filter(long jarg1, ESLconnection jarg1_, String jarg2, String jarg3);
  public final static native int ESLconnection_events(long jarg1, ESLconnection jarg1_, String jarg2, String jarg3);
  public final static native long ESLconnection_execute(long jarg1, ESLconnection jarg1_, String jarg2, String jarg3, String jarg4);
  public final static native long ESLconnection_executeAsync(long jarg1, ESLconnection jarg1_, String jarg2, String jarg3, String jarg4);
  public final static native int ESLconnection_setAsyncExecute(long jarg1, ESLconnection jarg1_, String jarg2);
  public final static native int ESLconnection_setEventLock(long jarg1, ESLconnection jarg1_, String jarg2);
  public final static native int ESLconnection_disconnect(long jarg1, ESLconnection jarg1_);
  public final static native void eslSetLogLevel(int jarg1);
}

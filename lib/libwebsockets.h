/*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010-2015 Andy Green <andy@warmcat.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation:
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#ifndef LIBWEBSOCKET_H_3060898B846849FF9F88F5DB59B5950C
#define LIBWEBSOCKET_H_3060898B846849FF9F88F5DB59B5950C

#ifdef __cplusplus
#include <cstddef>
#include <cstdarg>
#ifdef MBED_OPERATORS
#include "mbed-drivers/mbed.h"
#include "sal-iface-eth/EthernetInterface.h"
#include "sockets/TCPListener.h"
#include "sal-stack-lwip/lwipv4_init.h"

namespace {
	const int SERVER_PORT = 80;
	const int BUFFER_SIZE = 4096;
}
using namespace mbed::Sockets::v0;

struct libwebsocket;
struct libwebsocket_context;

class lws_conn {
	public:
	lws_conn():
		ts(NULL),
		wsi(NULL),
		writeable(1),
		awaiting_on_writeable(0)
	{
	}

public:
	void set_wsi(struct libwebsocket *_wsi) { wsi = _wsi; }
	int actual_onRX(Socket *s);
	void onRX(Socket *s);
	void onError(Socket *s, socket_error_t err);
	void onDisconnect(TCPStream *s);
	void onSent(Socket *s, uint16_t len);
	void serialized_writeable(struct libwebsocket *wsi);

public:
	TCPStream *ts;
	
public:
	struct libwebsocket *wsi;
	char buffer[BUFFER_SIZE];
	char writeable;
	char awaiting_on_writeable;
};

class lws_conn_listener : lws_conn {
public:
	lws_conn_listener():
		srv(SOCKET_STACK_LWIP_IPV4)
	{
		srv.setOnError(TCPStream::ErrorHandler_t(this, &lws_conn_listener::onError));
	}

	void start(const uint16_t port);

protected:
	void onRX(Socket *s);
	void onError(Socket *s, socket_error_t err);
	void onIncoming(TCPListener *s, void *impl);
	void onDisconnect(TCPStream *s);

public:
	TCPListener srv;
};

#endif

extern "C" {
#else
#include <stdarg.h>
#endif

#ifdef MBED_OPERATORS
#define LWS_POSIX 0
#else
#define LWS_POSIX 1
#endif
	
#include "lws_config.h"

#if defined(WIN32) || defined(_WIN32)
#if (WINVER < 0x0501)
#undef WINVER
#undef _WIN32_WINNT
#define WINVER 0x0501
#define _WIN32_WINNT WINVER
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stddef.h>
#include <stdint.h>
#include <basetsd.h>

#define strcasecmp stricmp
#define getdtablesize() 30000

#define LWS_VISIBLE

#ifdef LWS_DLL
#ifdef LWS_INTERNAL
#define LWS_EXTERN extern __declspec(dllexport)
#else
#define LWS_EXTERN extern __declspec(dllimport)
#endif
#else
#define LWS_EXTERN
#endif

#else // NOT WIN32
#include <unistd.h>

#ifndef MBED_OPERATORS
#include <poll.h>
#include <netdb.h>
#else
#define getdtablesize() (20)
#endif

#if defined(__GNUC__)
#define LWS_VISIBLE __attribute__((visibility("default")))
#else
#define LWS_VISIBLE
#endif

#if defined(__ANDROID__)
#define getdtablesize() 1024
#endif

#endif

#ifdef LWS_USE_LIBEV
#include <ev.h>
#endif /* LWS_USE_LIBEV */

#include <assert.h>

#ifndef LWS_EXTERN
#define LWS_EXTERN extern
#endif
	
#ifdef _WIN32
#define random rand
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#ifdef LWS_OPENSSL_SUPPORT
#ifdef USE_WOLFSSL
#ifdef USE_OLD_CYASSL
#include <cyassl/openssl/ssl.h>
#else
#include <wolfssl/openssl/ssl.h>
#endif /* not USE_OLD_CYASSL */
#else
#include <openssl/ssl.h>
#endif /* not USE_WOLFSSL */
#endif

#define CONTEXT_PORT_NO_LISTEN -1
#define MAX_MUX_RECURSION 2

enum lws_log_levels {
	LLL_ERR = 1 << 0,
	LLL_WARN = 1 << 1,
	LLL_NOTICE = 1 << 2,
	LLL_INFO = 1 << 3,
	LLL_DEBUG = 1 << 4,
	LLL_PARSER = 1 << 5,
	LLL_HEADER = 1 << 6,
	LLL_EXT = 1 << 7,
	LLL_CLIENT = 1 << 8,
	LLL_LATENCY = 1 << 9,

	LLL_COUNT = 10 /* set to count of valid flags */
};

LWS_VISIBLE LWS_EXTERN void _lws_log(int filter, const char *format, ...);
LWS_VISIBLE LWS_EXTERN void _lws_logv(int filter, const char *format, va_list vl);

/* notice, warn and log are always compiled in */
#define lwsl_notice(...) _lws_log(LLL_NOTICE, __VA_ARGS__)
#define lwsl_warn(...) _lws_log(LLL_WARN, __VA_ARGS__)
#define lwsl_err(...) _lws_log(LLL_ERR, __VA_ARGS__)
/*
 *  weaker logging can be deselected at configure time using --disable-debug
 *  that gets rid of the overhead of checking while keeping _warn and _err
 *  active
 */
#ifdef _DEBUG

#define lwsl_info(...) _lws_log(LLL_INFO, __VA_ARGS__)
#define lwsl_debug(...) _lws_log(LLL_DEBUG, __VA_ARGS__)
#define lwsl_parser(...) _lws_log(LLL_PARSER, __VA_ARGS__)
#define lwsl_header(...)  _lws_log(LLL_HEADER, __VA_ARGS__)
#define lwsl_ext(...)  _lws_log(LLL_EXT, __VA_ARGS__)
#define lwsl_client(...) _lws_log(LLL_CLIENT, __VA_ARGS__)
#define lwsl_latency(...) _lws_log(LLL_LATENCY, __VA_ARGS__)
LWS_VISIBLE LWS_EXTERN void lwsl_hexdump(void *buf, size_t len);

#else /* no debug */

#define lwsl_info(...) {}
#define lwsl_debug(...) {}
#define lwsl_parser(...) {}
#define lwsl_header(...) {}
#define lwsl_ext(...) {}
#define lwsl_client(...) {}
#define lwsl_latency(...) {}
#define lwsl_hexdump(a, b)

#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

/* api change list for user code to test against */

#define LWS_FEATURE_SERVE_HTTP_FILE_HAS_OTHER_HEADERS_ARG

/* the struct libwebsocket_protocols has the id field present */
#define LWS_FEATURE_PROTOCOLS_HAS_ID_FIELD

/* you can call lws_get_peer_write_allowance */
#define LWS_FEATURE_PROTOCOLS_HAS_PEER_WRITE_ALLOWANCE

/* extra parameter introduced in 917f43ab821 */
#define LWS_FEATURE_SERVE_HTTP_FILE_HAS_OTHER_HEADERS_LEN

enum libwebsocket_context_options {
	LWS_SERVER_OPTION_REQUIRE_VALID_OPENSSL_CLIENT_CERT = 2,
	LWS_SERVER_OPTION_SKIP_SERVER_CANONICAL_NAME = 4,
	LWS_SERVER_OPTION_ALLOW_NON_SSL_ON_SSL_PORT = 8,
	LWS_SERVER_OPTION_LIBEV = 16,
	LWS_SERVER_OPTION_DISABLE_IPV6 = 32,
	LWS_SERVER_OPTION_DISABLE_OS_CA_CERTS = 64,
	LWS_SERVER_OPTION_PEER_CERT_NOT_REQUIRED = 128,
};

enum libwebsocket_callback_reasons {
	LWS_CALLBACK_ESTABLISHED,
	LWS_CALLBACK_CLIENT_CONNECTION_ERROR,
	LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH,
	LWS_CALLBACK_CLIENT_ESTABLISHED,
	LWS_CALLBACK_CLOSED,
	LWS_CALLBACK_CLOSED_HTTP,
	LWS_CALLBACK_RECEIVE,
	LWS_CALLBACK_RECEIVE_PONG,
	LWS_CALLBACK_CLIENT_RECEIVE,
	LWS_CALLBACK_CLIENT_RECEIVE_PONG,
	LWS_CALLBACK_CLIENT_WRITEABLE,
	LWS_CALLBACK_SERVER_WRITEABLE,
	LWS_CALLBACK_HTTP,
	LWS_CALLBACK_HTTP_BODY,
	LWS_CALLBACK_HTTP_BODY_COMPLETION,
	LWS_CALLBACK_HTTP_FILE_COMPLETION,
	LWS_CALLBACK_HTTP_WRITEABLE,
	LWS_CALLBACK_FILTER_NETWORK_CONNECTION,
	LWS_CALLBACK_FILTER_HTTP_CONNECTION,
	LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED,
	LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION,
	LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS,
	LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS,
	LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION,
	LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER,
	LWS_CALLBACK_CONFIRM_EXTENSION_OKAY,
	LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED,
	LWS_CALLBACK_PROTOCOL_INIT,
	LWS_CALLBACK_PROTOCOL_DESTROY,
	LWS_CALLBACK_WSI_CREATE, /* always protocol[0] */
	LWS_CALLBACK_WSI_DESTROY, /* always protocol[0] */
	LWS_CALLBACK_GET_THREAD_ID,

	/* external poll() management support */
	LWS_CALLBACK_ADD_POLL_FD,
	LWS_CALLBACK_DEL_POLL_FD,
	LWS_CALLBACK_CHANGE_MODE_POLL_FD,
	LWS_CALLBACK_LOCK_POLL,
	LWS_CALLBACK_UNLOCK_POLL,

	LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY,
	
	LWS_CALLBACK_USER = 1000, /* user code can use any including / above */
};


#if defined(_WIN32) && (_WIN32_WINNT < 0x0600)
typedef SOCKET lws_sockfd_type;
#define lws_sockfd_valid(sfd) (!!sfd)
struct libwebsocket_pollfd {
	lws_sockfd_type fd;
	SHORT events;
	SHORT revents;
};
WINSOCK_API_LINKAGE int WSAAPI WSAPoll(struct libwebsocket_pollfd fdArray[], ULONG fds, INT timeout);
#else

#if defined(MBED_OPERATORS)
/* it's a class lws_conn * */
typedef void * lws_sockfd_type;
#define lws_sockfd_valid(sfd) (!!sfd)
struct pollfd {
	lws_sockfd_type fd;
	short events;
	short revents;
};
#define POLLIN          0x0001
#define POLLPRI         0x0002
#define POLLOUT         0x0004
#define POLLERR         0x0008
#define POLLHUP         0x0010
#define POLLNVAL        0x0020

struct libwebsocket;

void * mbed3_create_tcp_stream_socket(void);
void mbed3_delete_tcp_stream_socket(void *sockfd);
void mbed3_tcp_stream_bind(void *sock, int port, struct libwebsocket *);
void mbed3_tcp_stream_accept(void *sock, struct libwebsocket *);
#else
typedef int lws_sockfd_type;
#define lws_sockfd_valid(sfd) (sfd >= 0)
#endif

#define libwebsocket_pollfd pollfd
#endif

// argument structure for all external poll related calls
// passed in via 'in'
struct libwebsocket_pollargs {
    lws_sockfd_type fd;            // applicable file descriptor
    int events;        // the new event mask
    int prev_events;   // the previous event mask
};

enum libwebsocket_extension_callback_reasons {
	LWS_EXT_CALLBACK_SERVER_CONTEXT_CONSTRUCT,
	LWS_EXT_CALLBACK_CLIENT_CONTEXT_CONSTRUCT,
	LWS_EXT_CALLBACK_SERVER_CONTEXT_DESTRUCT,
	LWS_EXT_CALLBACK_CLIENT_CONTEXT_DESTRUCT,
	LWS_EXT_CALLBACK_CONSTRUCT,
	LWS_EXT_CALLBACK_CLIENT_CONSTRUCT,
	LWS_EXT_CALLBACK_CHECK_OK_TO_REALLY_CLOSE,
	LWS_EXT_CALLBACK_CHECK_OK_TO_PROPOSE_EXTENSION,
	LWS_EXT_CALLBACK_DESTROY,
	LWS_EXT_CALLBACK_DESTROY_ANY_WSI_CLOSING,
	LWS_EXT_CALLBACK_ANY_WSI_ESTABLISHED,
	LWS_EXT_CALLBACK_PACKET_RX_PREPARSE,
	LWS_EXT_CALLBACK_PACKET_TX_PRESEND,
	LWS_EXT_CALLBACK_PACKET_TX_DO_SEND,
	LWS_EXT_CALLBACK_HANDSHAKE_REPLY_TX,
	LWS_EXT_CALLBACK_FLUSH_PENDING_TX,
	LWS_EXT_CALLBACK_EXTENDED_PAYLOAD_RX,
	LWS_EXT_CALLBACK_CAN_PROXY_CLIENT_CONNECTION,
	LWS_EXT_CALLBACK_1HZ,
	LWS_EXT_CALLBACK_REQUEST_ON_WRITEABLE,
	LWS_EXT_CALLBACK_IS_WRITEABLE,
	LWS_EXT_CALLBACK_PAYLOAD_TX,
	LWS_EXT_CALLBACK_PAYLOAD_RX,
};

enum libwebsocket_write_protocol {
	LWS_WRITE_TEXT,
	LWS_WRITE_BINARY,
	LWS_WRITE_CONTINUATION,
	LWS_WRITE_HTTP,

	/* special 04+ opcodes */

	LWS_WRITE_CLOSE,
	LWS_WRITE_PING,
	LWS_WRITE_PONG,

	/* Same as write_http but we know this write ends the transaction */
	LWS_WRITE_HTTP_FINAL,
	
	/* HTTP2 */

	LWS_WRITE_HTTP_HEADERS,
	
	/* flags */

	LWS_WRITE_NO_FIN = 0x40,
	/*
	 * client packet payload goes out on wire unmunged
	 * only useful for security tests since normal servers cannot
	 * decode the content if used
	 */
	LWS_WRITE_CLIENT_IGNORE_XOR_MASK = 0x80
};

/*
 * you need these to look at headers that have been parsed if using the
 * LWS_CALLBACK_FILTER_CONNECTION callback.  If a header from the enum
 * list below is absent, .token = NULL and token_len = 0.  Otherwise .token
 * points to .token_len chars containing that header content.
 */

struct lws_tokens {
	char *token;
	int token_len;
};

/*
 * don't forget to update test server header dump accordingly
 *
 * these have to be kept in sync with lextable.h / minilex.c
 */

enum lws_token_indexes {
	WSI_TOKEN_GET_URI,
	WSI_TOKEN_POST_URI,
	WSI_TOKEN_OPTIONS_URI,
	WSI_TOKEN_HOST,
	WSI_TOKEN_CONNECTION,
	WSI_TOKEN_UPGRADE,
	WSI_TOKEN_ORIGIN,
	WSI_TOKEN_DRAFT,
	WSI_TOKEN_CHALLENGE,
	WSI_TOKEN_EXTENSIONS,
	WSI_TOKEN_KEY1,
	WSI_TOKEN_KEY2,
	WSI_TOKEN_PROTOCOL,
	WSI_TOKEN_ACCEPT,
	WSI_TOKEN_NONCE,
	WSI_TOKEN_HTTP,
	WSI_TOKEN_HTTP2_SETTINGS,
	WSI_TOKEN_HTTP_ACCEPT,
	WSI_TOKEN_HTTP_AC_REQUEST_HEADERS,
	WSI_TOKEN_HTTP_IF_MODIFIED_SINCE,
	WSI_TOKEN_HTTP_IF_NONE_MATCH,
	WSI_TOKEN_HTTP_ACCEPT_ENCODING,
	WSI_TOKEN_HTTP_ACCEPT_LANGUAGE,
	WSI_TOKEN_HTTP_PRAGMA,
	WSI_TOKEN_HTTP_CACHE_CONTROL,
	WSI_TOKEN_HTTP_AUTHORIZATION,
	WSI_TOKEN_HTTP_COOKIE,
	WSI_TOKEN_HTTP_CONTENT_LENGTH,
	WSI_TOKEN_HTTP_CONTENT_TYPE,
	WSI_TOKEN_HTTP_DATE,
	WSI_TOKEN_HTTP_RANGE,
	WSI_TOKEN_HTTP_REFERER,
	WSI_TOKEN_KEY,
	WSI_TOKEN_VERSION,
	WSI_TOKEN_SWORIGIN,

	WSI_TOKEN_HTTP_COLON_AUTHORITY,
	WSI_TOKEN_HTTP_COLON_METHOD,
	WSI_TOKEN_HTTP_COLON_PATH,
	WSI_TOKEN_HTTP_COLON_SCHEME,
	WSI_TOKEN_HTTP_COLON_STATUS,
	
	WSI_TOKEN_HTTP_ACCEPT_CHARSET,
	WSI_TOKEN_HTTP_ACCEPT_RANGES,
	WSI_TOKEN_HTTP_ACCESS_CONTROL_ALLOW_ORIGIN,
	WSI_TOKEN_HTTP_AGE,
	WSI_TOKEN_HTTP_ALLOW,
	WSI_TOKEN_HTTP_CONTENT_DISPOSITION,
	WSI_TOKEN_HTTP_CONTENT_ENCODING,
	WSI_TOKEN_HTTP_CONTENT_LANGUAGE,
	WSI_TOKEN_HTTP_CONTENT_LOCATION,
	WSI_TOKEN_HTTP_CONTENT_RANGE,
	WSI_TOKEN_HTTP_ETAG,
	WSI_TOKEN_HTTP_EXPECT,
	WSI_TOKEN_HTTP_EXPIRES,
	WSI_TOKEN_HTTP_FROM,
	WSI_TOKEN_HTTP_IF_MATCH,
	WSI_TOKEN_HTTP_IF_RANGE,
	WSI_TOKEN_HTTP_IF_UNMODIFIED_SINCE,
	WSI_TOKEN_HTTP_LAST_MODIFIED,
	WSI_TOKEN_HTTP_LINK,
	WSI_TOKEN_HTTP_LOCATION,
	WSI_TOKEN_HTTP_MAX_FORWARDS,
	WSI_TOKEN_HTTP_PROXY_AUTHENTICATE,
	WSI_TOKEN_HTTP_PROXY_AUTHORIZATION,
	WSI_TOKEN_HTTP_REFRESH,
	WSI_TOKEN_HTTP_RETRY_AFTER,
	WSI_TOKEN_HTTP_SERVER,
	WSI_TOKEN_HTTP_SET_COOKIE,
	WSI_TOKEN_HTTP_STRICT_TRANSPORT_SECURITY,
	WSI_TOKEN_HTTP_TRANSFER_ENCODING,
	WSI_TOKEN_HTTP_USER_AGENT,
	WSI_TOKEN_HTTP_VARY,
	WSI_TOKEN_HTTP_VIA,
	WSI_TOKEN_HTTP_WWW_AUTHENTICATE,
	WSI_TOKEN_PROXY,
	
	WSI_TOKEN_PATCH_URI,
	WSI_TOKEN_PUT_URI,
	WSI_TOKEN_DELETE_URI,
	
	WSI_TOKEN_HTTP_URI_ARGS,
	
	/* use token storage to stash these */

	_WSI_TOKEN_CLIENT_SENT_PROTOCOLS,
	_WSI_TOKEN_CLIENT_PEER_ADDRESS,
	_WSI_TOKEN_CLIENT_URI,
	_WSI_TOKEN_CLIENT_HOST,
	_WSI_TOKEN_CLIENT_ORIGIN,
	
	/* always last real token index*/
	WSI_TOKEN_COUNT,
	/* parser state additions */
	WSI_TOKEN_NAME_PART,
	WSI_TOKEN_SKIPPING,
	WSI_TOKEN_SKIPPING_SAW_CR,
	WSI_PARSING_COMPLETE,
	WSI_INIT_TOKEN_MUXURL,
};

struct lws_token_limits {
    unsigned short token_limit[WSI_TOKEN_COUNT];
};

/*
 * From RFC 6455
   1000

      1000 indicates a normal closure, meaning that the purpose for
      which the connection was established has been fulfilled.

   1001

      1001 indicates that an endpoint is "going away", such as a server
      going down or a browser having navigated away from a page.

   1002

      1002 indicates that an endpoint is terminating the connection due
      to a protocol error.

   1003

      1003 indicates that an endpoint is terminating the connection
      because it has received a type of data it cannot accept (e.g., an
      endpoint that understands only text data MAY send this if it
      receives a binary message).

   1004

      Reserved.  The specific meaning might be defined in the future.

   1005

      1005 is a reserved value and MUST NOT be set as a status code in a
      Close control frame by an endpoint.  It is designated for use in
      applications expecting a status code to indicate that no status
      code was actually present.

   1006

      1006 is a reserved value and MUST NOT be set as a status code in a
      Close control frame by an endpoint.  It is designated for use in
      applications expecting a status code to indicate that the
      connection was closed abnormally, e.g., without sending or
      receiving a Close control frame.

   1007

      1007 indicates that an endpoint is terminating the connection
      because it has received data within a message that was not
      consistent with the type of the message (e.g., non-UTF-8 [RFC3629]
      data within a text message).

   1008

      1008 indicates that an endpoint is terminating the connection
      because it has received a message that violates its policy.  This
      is a generic status code that can be returned when there is no
      other more suitable status code (e.g., 1003 or 1009) or if there
      is a need to hide specific details about the policy.

   1009

      1009 indicates that an endpoint is terminating the connection
      because it has received a message that is too big for it to
      process.

   1010

      1010 indicates that an endpoint (client) is terminating the
      connection because it has expected the server to negotiate one or
      more extension, but the server didn't return them in the response
      message of the WebSocket handshake.  The list of extensions that
      are needed SHOULD appear in the /reason/ part of the Close frame.
      Note that this status code is not used by the server, because it
      can fail the WebSocket handshake instead.

   1011

      1011 indicates that a server is terminating the connection because
      it encountered an unexpected condition that prevented it from
      fulfilling the request.

   1015

      1015 is a reserved value and MUST NOT be set as a status code in a
      Close control frame by an endpoint.  It is designated for use in
      applications expecting a status code to indicate that the
      connection was closed due to a failure to perform a TLS handshake
      (e.g., the server certificate can't be verified).
*/

enum lws_close_status {
	LWS_CLOSE_STATUS_NOSTATUS = 0,
	LWS_CLOSE_STATUS_NORMAL = 1000,
	LWS_CLOSE_STATUS_GOINGAWAY = 1001,
	LWS_CLOSE_STATUS_PROTOCOL_ERR = 1002,
	LWS_CLOSE_STATUS_UNACCEPTABLE_OPCODE = 1003,
	LWS_CLOSE_STATUS_RESERVED = 1004,
	LWS_CLOSE_STATUS_NO_STATUS = 1005,
	LWS_CLOSE_STATUS_ABNORMAL_CLOSE = 1006,
	LWS_CLOSE_STATUS_INVALID_PAYLOAD = 1007,
	LWS_CLOSE_STATUS_POLICY_VIOLATION = 1008,
	LWS_CLOSE_STATUS_MESSAGE_TOO_LARGE = 1009,
	LWS_CLOSE_STATUS_EXTENSION_REQUIRED = 1010,
	LWS_CLOSE_STATUS_UNEXPECTED_CONDITION = 1011,
	LWS_CLOSE_STATUS_TLS_FAILURE = 1015,

	LWS_CLOSE_STATUS_NOSTATUS_CONTEXT_DESTROY = 9999,
};

enum http_status {
	HTTP_STATUS_OK = 200,
	HTTP_STATUS_NO_CONTENT = 204,

	HTTP_STATUS_BAD_REQUEST = 400,
	HTTP_STATUS_UNAUTHORIZED,
	HTTP_STATUS_PAYMENT_REQUIRED,
	HTTP_STATUS_FORBIDDEN,
	HTTP_STATUS_NOT_FOUND,
	HTTP_STATUS_METHOD_NOT_ALLOWED,
	HTTP_STATUS_NOT_ACCEPTABLE,
	HTTP_STATUS_PROXY_AUTH_REQUIRED,
	HTTP_STATUS_REQUEST_TIMEOUT,
	HTTP_STATUS_CONFLICT,
	HTTP_STATUS_GONE,
	HTTP_STATUS_LENGTH_REQUIRED,
	HTTP_STATUS_PRECONDITION_FAILED,
	HTTP_STATUS_REQ_ENTITY_TOO_LARGE,
	HTTP_STATUS_REQ_URI_TOO_LONG,
	HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE,
	HTTP_STATUS_REQ_RANGE_NOT_SATISFIABLE,
	HTTP_STATUS_EXPECTATION_FAILED,

	HTTP_STATUS_INTERNAL_SERVER_ERROR = 500,
	HTTP_STATUS_NOT_IMPLEMENTED,
	HTTP_STATUS_BAD_GATEWAY,
	HTTP_STATUS_SERVICE_UNAVAILABLE,
	HTTP_STATUS_GATEWAY_TIMEOUT,
	HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED,
};

struct libwebsocket;
struct libwebsocket_context;
/* needed even with extensions disabled for create context */
struct libwebsocket_extension;

/**
 * callback_function() - User server actions
 * @context:	Websockets context
 * @wsi:	Opaque websocket instance pointer
 * @reason:	The reason for the call
 * @user:	Pointer to per-session user data allocated by library
 * @in:		Pointer used for some callback reasons
 * @len:	Length set for some callback reasons
 *
 *	This callback is the way the user controls what is served.  All the
 *	protocol detail is hidden and handled by the library.
 *
 *	For each connection / session there is user data allocated that is
 *	pointed to by "user".  You set the size of this user data area when
 *	the library is initialized with libwebsocket_create_server.
 *
 *	You get an opportunity to initialize user data when called back with
 *	LWS_CALLBACK_ESTABLISHED reason.
 *
 *  LWS_CALLBACK_ESTABLISHED:  after the server completes a handshake with
 *				an incoming client
 *
 *  LWS_CALLBACK_CLIENT_CONNECTION_ERROR: the request client connection has
 *        been unable to complete a handshake with the remote server.  If
 *	  in is non-NULL, you can find an error string of length len where
 *	  it points to.
 *
 *  LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH: this is the last chance for the
 *				client user code to examine the http headers
 *				and decide to reject the connection.  If the
 *				content in the headers is interesting to the
 *				client (url, etc) it needs to copy it out at
 *				this point since it will be destroyed before
 *				the CLIENT_ESTABLISHED call
 *
 *  LWS_CALLBACK_CLIENT_ESTABLISHED: after your client connection completed
 *				a handshake with the remote server
 *
 *	LWS_CALLBACK_CLOSED: when the websocket session ends
 *
 *	LWS_CALLBACK_CLOSED_HTTP: when a HTTP (non-websocket) session ends
 *
 *	LWS_CALLBACK_RECEIVE: data has appeared for this server endpoint from a
 *				remote client, it can be found at *in and is
 *				len bytes long
 *
 *	LWS_CALLBACK_CLIENT_RECEIVE_PONG: if you elected to see PONG packets,
 *				they appear with this callback reason.  PONG
 *				packets only exist in 04+ protocol
 *
 *	LWS_CALLBACK_CLIENT_RECEIVE: data has appeared from the server for the
 *				client connection, it can be found at *in and
 *				is len bytes long
 *
 *	LWS_CALLBACK_HTTP: an http request has come from a client that is not
 *				asking to upgrade the connection to a websocket
 *				one.  This is a chance to serve http content,
 *				for example, to send a script to the client
 *				which will then open the websockets connection.
 *				@in points to the URI path requested and
 *				libwebsockets_serve_http_file() makes it very
 *				simple to send back a file to the client.
 *				Normally after sending the file you are done
 *				with the http connection, since the rest of the
 *				activity will come by websockets from the script
 *				that was delivered by http, so you will want to
 *				return 1; to close and free up the connection.
 *				That's important because it uses a slot in the
 *				total number of client connections allowed set
 *				by MAX_CLIENTS.
 *
 *	LWS_CALLBACK_HTTP_BODY: the next @len bytes data from the http
 *		request body HTTP connection is now available in @in.
 *
 *	LWS_CALLBACK_HTTP_BODY_COMPLETION: the expected amount of http request
 *		body has been delivered
 *
 *	LWS_CALLBACK_HTTP_WRITEABLE: you can write more down the http protocol
 *		link now.
 *
 *	LWS_CALLBACK_HTTP_FILE_COMPLETION: a file requested to be send down
 *				http link has completed.
 *
 *	LWS_CALLBACK_CLIENT_WRITEABLE:
 *      LWS_CALLBACK_SERVER_WRITEABLE:   If you call
 *		libwebsocket_callback_on_writable() on a connection, you will
 *		get one of these callbacks coming when the connection socket
 *		is able to accept another write packet without blocking.
 *		If it already was able to take another packet without blocking,
 *		you'll get this callback at the next call to the service loop
 *		function.  Notice that CLIENTs get LWS_CALLBACK_CLIENT_WRITEABLE
 *		and servers get LWS_CALLBACK_SERVER_WRITEABLE.
 *
 *	LWS_CALLBACK_FILTER_NETWORK_CONNECTION: called when a client connects to
 *		the server at network level; the connection is accepted but then
 *		passed to this callback to decide whether to hang up immediately
 *		or not, based on the client IP.  @in contains the connection
 *		socket's descriptor. Since the client connection information is
 *		not available yet, @wsi still pointing to the main server socket.
 *		Return non-zero to terminate the connection before sending or
 *		receiving anything. Because this happens immediately after the
 *		network connection from the client, there's no websocket protocol
 *		selected yet so this callback is issued only to protocol 0.
 * 
 *	LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED: A new client just had
 *		been connected, accepted, and instantiated into the pool. This
 *		callback allows setting any relevant property to it. Because this
 *		happens immediately after the instantiation of a new client,
 *		there's no websocket protocol selected yet so this callback is
 *		issued only to protocol 0. Only @wsi is defined, pointing to the
 *		new client, and the return value is ignored.
 *
 *	LWS_CALLBACK_FILTER_HTTP_CONNECTION: called when the request has
 *		been received and parsed from the client, but the response is
 *		not sent yet.  Return non-zero to disallow the connection.
 *		@user is a pointer to the connection user space allocation,
 *		@in is the URI, eg, "/"
 *		In your handler you can use the public APIs
 *		lws_hdr_total_length() / lws_hdr_copy() to access all of the
 *		headers using the header enums lws_token_indexes from
 *		libwebsockets.h to check for and read the supported header
 *		presence and content before deciding to allow the http
 *		connection to proceed or to kill the connection.
 *
 *	LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION: called when the handshake has
 *		been received and parsed from the client, but the response is
 *		not sent yet.  Return non-zero to disallow the connection.
 *		@user is a pointer to the connection user space allocation,
 *		@in is the requested protocol name
 *		In your handler you can use the public APIs
 *		lws_hdr_total_length() / lws_hdr_copy() to access all of the
 *		headers using the header enums lws_token_indexes from
 *		libwebsockets.h to check for and read the supported header
 *		presence and content before deciding to allow the handshake
 *		to proceed or to kill the connection.
 *
 *	LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS: if configured for
 *		including OpenSSL support, this callback allows your user code
 *		to perform extra SSL_CTX_load_verify_locations() or similar
 *		calls to direct OpenSSL where to find certificates the client
 *		can use to confirm the remote server identity.  @user is the
 *		OpenSSL SSL_CTX*
 *
 *	LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS: if configured for
 *		including OpenSSL support, this callback allows your user code
 *		to load extra certifcates into the server which allow it to
 *		verify the validity of certificates returned by clients.  @user
 *		is the server's OpenSSL SSL_CTX*
 *
 *	LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY: if configured for
 *		including OpenSSL support but no private key file has been specified
 *		(ssl_private_key_filepath is NULL), this callback is called to
 *		allow the user to set the private key directly via libopenssl
 *		and perform further operations if required; this might be useful
 *		in situations where the private key is not directly accessible by
 *		the OS, for example if it is stored on a smartcard
 *		@user is the server's OpenSSL SSL_CTX*
 *
 *	LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION: if the
 *		libwebsockets context was created with the option
 *		LWS_SERVER_OPTION_REQUIRE_VALID_OPENSSL_CLIENT_CERT, then this
 *		callback is generated during OpenSSL verification of the cert
 *		sent from the client.  It is sent to protocol[0] callback as
 *		no protocol has been negotiated on the connection yet.
 *		Notice that the libwebsockets context and wsi are both NULL
 *		during this callback.  See
 *		 http://www.openssl.org/docs/ssl/SSL_CTX_set_verify.html
 *		to understand more detail about the OpenSSL callback that
 *		generates this libwebsockets callback and the meanings of the
 *		arguments passed.  In this callback, @user is the x509_ctx,
 *		@in is the ssl pointer and @len is preverify_ok
 *		Notice that this callback maintains libwebsocket return
 *		conventions, return 0 to mean the cert is OK or 1 to fail it.
 *		This also means that if you don't handle this callback then
 *		the default callback action of returning 0 allows the client
 *		certificates.
 *
 *	LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER: this callback happens
 *		when a client handshake is being compiled.  @user is NULL,
 *		@in is a char **, it's pointing to a char * which holds the
 *		next location in the header buffer where you can add
 *		headers, and @len is the remaining space in the header buffer,
 *		which is typically some hundreds of bytes.  So, to add a canned
 *		cookie, your handler code might look similar to:
 *
 *		char **p = (char **)in;
 *
 *		if (len < 100)
 *			return 1;
 *
 *		*p += sprintf(*p, "Cookie: a=b\x0d\x0a");
 *
 *		return 0;
 *
 *		Notice if you add anything, you just have to take care about
 *		the CRLF on the line you added.  Obviously this callback is
 *		optional, if you don't handle it everything is fine.
 *
 *		Notice the callback is coming to protocols[0] all the time,
 *		because there is no specific protocol handshook yet.
 *
 *	LWS_CALLBACK_CONFIRM_EXTENSION_OKAY: When the server handshake code
 *		sees that it does support a requested extension, before
 *		accepting the extension by additing to the list sent back to
 *		the client it gives this callback just to check that it's okay
 *		to use that extension.  It calls back to the requested protocol
 *		and with @in being the extension name, @len is 0 and @user is
 *		valid.  Note though at this time the ESTABLISHED callback hasn't
 *		happened yet so if you initialize @user content there, @user
 *		content during this callback might not be useful for anything.
 *		Notice this callback comes to protocols[0].
 *
 *	LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:	When a client
 *		connection is being prepared to start a handshake to a server,
 *		each supported extension is checked with protocols[0] callback
 *		with this reason, giving the user code a chance to suppress the
 *		claim to support that extension by returning non-zero.  If
 *		unhandled, by default 0 will be returned and the extension
 *		support included in the header to the server.  Notice this
 *		callback comes to protocols[0].
 *
 *	LWS_CALLBACK_PROTOCOL_INIT:	One-time call per protocol so it can
 *		do initial setup / allocations etc
 *
 *	LWS_CALLBACK_PROTOCOL_DESTROY:	One-time call per protocol indicating
 *		this protocol won't get used at all after this callback, the
 *		context is getting destroyed.  Take the opportunity to
 *		deallocate everything that was allocated by the protocol.
 *
 *	LWS_CALLBACK_WSI_CREATE: outermost (earliest) wsi create notification
 *
 *	LWS_CALLBACK_WSI_DESTROY: outermost (latest) wsi destroy notification
 *
 *	The next five reasons are optional and only need taking care of if you
 *	will be integrating libwebsockets sockets into an external polling
 *	array.
 *
 *	For these calls, @in points to a struct libwebsocket_pollargs that
 *	contains @fd, @events and @prev_events members
 *
 *	LWS_CALLBACK_ADD_POLL_FD: libwebsocket deals with its poll() loop
 *		internally, but in the case you are integrating with another
 *		server you will need to have libwebsocket sockets share a
 *		polling array with the other server.  This and the other
 *		POLL_FD related callbacks let you put your specialized
 *		poll array interface code in the callback for protocol 0, the
 *		first protocol you support, usually the HTTP protocol in the
 *		serving case.
 *		This callback happens when a socket needs to be
 *		added to the polling loop: @in points to a struct
 *		libwebsocket_pollargs; the @fd member of the struct is the file
 *		descriptor, and @events contains the active events.
 *
 *		If you are using the internal polling loop (the "service"
 *		callback), you can just ignore these callbacks.
 *
 *	LWS_CALLBACK_DEL_POLL_FD: This callback happens when a socket descriptor
 *		needs to be removed from an external polling array.  @in is
 *		again the struct libwebsocket_pollargs containing the @fd member
 *		to be removed.  If you are using the internal polling
 *		loop, you can just ignore it.
 *
 *	LWS_CALLBACK_CHANGE_MODE_POLL_FD: This callback happens when
 *		libwebsockets wants to modify the events for a connectiion.
 *		@in is the struct libwebsocket_pollargs with the @fd to change.
 *		The new event mask is in @events member and the old mask is in
 *		the @prev_events member.
 *		If you are using the internal polling loop, you can just ignore
 *		it.
 *
 *	LWS_CALLBACK_LOCK_POLL:
 *	LWS_CALLBACK_UNLOCK_POLL: These allow the external poll changes driven
 *		by libwebsockets to participate in an external thread locking
 *		scheme around the changes, so the whole thing is threadsafe.
 *		These are called around three activities in the library,
 *			- inserting a new wsi in the wsi / fd table (len=1)
 *			- deleting a wsi from the wsi / fd table (len=1)
 *			- changing a wsi's POLLIN/OUT state (len=0)
 *		Locking and unlocking external synchronization objects when
 *		len == 1 allows external threads to be synchronized against
 *		wsi lifecycle changes if it acquires the same lock for the
 *		duration of wsi dereference from the other thread context.
 */
LWS_VISIBLE LWS_EXTERN int callback(struct libwebsocket_context *context,
			struct libwebsocket *wsi,
			 enum libwebsocket_callback_reasons reason, void *user,
							  void *in, size_t len);

typedef int (callback_function)(struct libwebsocket_context *context,
			struct libwebsocket *wsi,
			 enum libwebsocket_callback_reasons reason, void *user,
							  void *in, size_t len);

#ifndef LWS_NO_EXTENSIONS
/**
 * extension_callback_function() - Hooks to allow extensions to operate
 * @context:	Websockets context
 * @ext:	This extension
 * @wsi:	Opaque websocket instance pointer
 * @reason:	The reason for the call
 * @user:	Pointer to per-session user data allocated by library
 * @in:		Pointer used for some callback reasons
 * @len:	Length set for some callback reasons
 *
 *	Each extension that is active on a particular connection receives
 *	callbacks during the connection lifetime to allow the extension to
 *	operate on websocket data and manage itself.
 *
 *	Libwebsockets takes care of allocating and freeing "user" memory for
 *	each active extension on each connection.  That is what is pointed to
 *	by the @user parameter.
 *
 *	LWS_EXT_CALLBACK_CONSTRUCT:  called when the server has decided to
 *		select this extension from the list provided by the client,
 *		just before the server will send back the handshake accepting
 *		the connection with this extension active.  This gives the
 *		extension a chance to initialize its connection context found
 *		in @user.
 *
 *	LWS_EXT_CALLBACK_CLIENT_CONSTRUCT: same as LWS_EXT_CALLBACK_CONSTRUCT
 *		but called when client is instantiating this extension.  Some
 *		extensions will work the same on client and server side and then
 *		you can just merge handlers for both CONSTRUCTS.
 *
 *	LWS_EXT_CALLBACK_DESTROY:  called when the connection the extension was
 *		being used on is about to be closed and deallocated.  It's the
 *		last chance for the extension to deallocate anything it has
 *		allocated in the user data (pointed to by @user) before the
 *		user data is deleted.  This same callback is used whether you
 *		are in client or server instantiation context.
 *
 *	LWS_EXT_CALLBACK_PACKET_RX_PREPARSE: when this extension was active on
 *		a connection, and a packet of data arrived at the connection,
 *		it is passed to this callback to give the extension a chance to
 *		change the data, eg, decompress it.  @user is pointing to the
 *		extension's private connection context data, @in is pointing
 *		to an lws_tokens struct, it consists of a char * pointer called
 *		token, and an int called token_len.  At entry, these are
 *		set to point to the received buffer and set to the content
 *		length.  If the extension will grow the content, it should use
 *		a new buffer allocated in its private user context data and
 *		set the pointed-to lws_tokens members to point to its buffer.
 *
 *	LWS_EXT_CALLBACK_PACKET_TX_PRESEND: this works the same way as
 *		LWS_EXT_CALLBACK_PACKET_RX_PREPARSE above, except it gives the
 *		extension a chance to change websocket data just before it will
 *		be sent out.  Using the same lws_token pointer scheme in @in,
 *		the extension can change the buffer and the length to be
 *		transmitted how it likes.  Again if it wants to grow the
 *		buffer safely, it should copy the data into its own buffer and
 *		set the lws_tokens token pointer to it.
 */
LWS_VISIBLE LWS_EXTERN int extension_callback(struct libwebsocket_context *context,
			struct libwebsocket_extension *ext,
			struct libwebsocket *wsi,
			enum libwebsocket_extension_callback_reasons reason,
			void *user, void *in, size_t len);

typedef int (extension_callback_function)(struct libwebsocket_context *context,
			struct libwebsocket_extension *ext,
			struct libwebsocket *wsi,
			enum libwebsocket_extension_callback_reasons reason,
			void *user, void *in, size_t len);
#endif

/**
 * struct libwebsocket_protocols -	List of protocols and handlers server
 *					supports.
 * @name:	Protocol name that must match the one given in the client
 *		Javascript new WebSocket(url, 'protocol') name.
 * @callback:	The service callback used for this protocol.  It allows the
 *		service action for an entire protocol to be encapsulated in
 *		the protocol-specific callback
 * @per_session_data_size:	Each new connection using this protocol gets
 *		this much memory allocated on connection establishment and
 *		freed on connection takedown.  A pointer to this per-connection
 *		allocation is passed into the callback in the 'user' parameter
 * @rx_buffer_size: if you want atomic frames delivered to the callback, you
 *		should set this to the size of the biggest legal frame that
 *		you support.  If the frame size is exceeded, there is no
 *		error, but the buffer will spill to the user callback when
 *		full, which you can detect by using
 *		libwebsockets_remaining_packet_payload().  Notice that you
 *		just talk about frame size here, the LWS_SEND_BUFFER_PRE_PADDING
 *		and post-padding are automatically also allocated on top.
 * @id:		ignored by lws, but useful to contain user information bound
 *		to the selected protocol.  For example if this protocol was
 *		called "myprotocol-v2", you might set id to 2, and the user
 *		code that acts differently according to the version can do so by
 *		switch (wsi->protocol->id), user code might use some bits as
 *		capability flags based on selected protocol version, etc.
 * @user:	User provided context data at the protocol level.
 *		Accessible via libwebsockets_get_protocol(wsi)->user
 *		This should not be confused with wsi->user, it is not the same.
 *		The library completely ignores any value in here.
 * @owning_server:	the server init call fills in this opaque pointer when
 *		registering this protocol with the server.
 * @protocol_index: which protocol we are starting from zero
 *
 *	This structure represents one protocol supported by the server.  An
 *	array of these structures is passed to libwebsocket_create_server()
 *	allows as many protocols as you like to be handled by one server.
 *
 *	The first protocol given has its callback used for user callbacks when
 *	there is no agreed protocol name, that's true during HTTP part of the
 *	connection and true if the client did not send a Protocol: header.
 */

struct libwebsocket_protocols {
	const char *name;
	callback_function *callback;
	size_t per_session_data_size;
	size_t rx_buffer_size;
	unsigned int id;
	void *user;

	/*
	 * below are filled in on server init and can be left uninitialized,
	 * no need for user to use them directly either
	 */

	struct libwebsocket_context *owning_server;
	int protocol_index;
};

#ifndef LWS_NO_EXTENSIONS
/**
 * struct libwebsocket_extension -	An extension we know how to cope with
 *
 * @name:			Formal extension name, eg, "deflate-stream"
 * @callback:			Service callback
 * @per_session_data_size:	Libwebsockets will auto-malloc this much
 *				memory for the use of the extension, a pointer
 *				to it comes in the @user callback parameter
 * @per_context_private_data:   Optional storage for this extension that
 *				is per-context, so it can track stuff across
 *				all sessions, etc, if it wants
 */

struct libwebsocket_extension {
	const char *name;
	extension_callback_function *callback;
	size_t per_session_data_size;
	void *per_context_private_data;
};
#endif

/**
 * struct lws_context_creation_info: parameters to create context with
 *
 * @port:	Port to listen on... you can use CONTEXT_PORT_NO_LISTEN to
 *		suppress listening on any port, that's what you want if you are
 *		not running a websocket server at all but just using it as a
 *		client
 * @iface:	NULL to bind the listen socket to all interfaces, or the
 *		interface name, eg, "eth2"
 * @protocols:	Array of structures listing supported protocols and a protocol-
 *		specific callback for each one.  The list is ended with an
 *		entry that has a NULL callback pointer.
 *		It's not const because we write the owning_server member
 * @extensions: NULL or array of libwebsocket_extension structs listing the
 *		extensions this context supports.  If you configured with
 *		--without-extensions, you should give NULL here.
 * @token_limits: NULL or struct lws_token_limits pointer which is initialized
 *      with a token length limit for each possible WSI_TOKEN_*** 
 * @ssl_cert_filepath:	If libwebsockets was compiled to use ssl, and you want
 *			to listen using SSL, set to the filepath to fetch the
 *			server cert from, otherwise NULL for unencrypted
 * @ssl_private_key_filepath: filepath to private key if wanting SSL mode;
 *			if this is set to NULL but sll_cert_filepath is set, the
 *			OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY callback is called to allow
 *			setting of the private key directly via openSSL library calls
 * @ssl_ca_filepath: CA certificate filepath or NULL
 * @ssl_cipher_list:	List of valid ciphers to use (eg,
 * 			"RC4-MD5:RC4-SHA:AES128-SHA:AES256-SHA:HIGH:!DSS:!aNULL"
 * 			or you can leave it as NULL to get "DEFAULT"
 * @http_proxy_address: If non-NULL, attempts to proxy via the given address.
 *			If proxy auth is required, use format
 *			"username:password@server:port"
 * @http_proxy_port:	If http_proxy_address was non-NULL, uses this port at the address 
 * @gid:	group id to change to after setting listen socket, or -1.
 * @uid:	user id to change to after setting listen socket, or -1.
 * @options:	0, or LWS_SERVER_OPTION_DEFEAT_CLIENT_MASK
 * @user:	optional user pointer that can be recovered via the context
 *		pointer using libwebsocket_context_user
 * @ka_time:	0 for no keepalive, otherwise apply this keepalive timeout to
 *		all libwebsocket sockets, client or server
 * @ka_probes:	if ka_time was nonzero, after the timeout expires how many
 *		times to try to get a response from the peer before giving up
 *		and killing the connection
 * @ka_interval: if ka_time was nonzero, how long to wait before each ka_probes
 *		attempt
 * @provided_client_ssl_ctx: If non-null, swap out libwebsockets ssl
 *		implementation for the one provided by provided_ssl_ctx.
 *		Libwebsockets no longer is responsible for freeing the context
 *		if this option is selected.
 */

struct lws_context_creation_info {
	int port;
	const char *iface;
	struct libwebsocket_protocols *protocols;
	struct libwebsocket_extension *extensions;
	struct lws_token_limits *token_limits;
	const char *ssl_private_key_password;
	const char *ssl_cert_filepath;
	const char *ssl_private_key_filepath;
	const char *ssl_ca_filepath;
	const char *ssl_cipher_list;
	const char *http_proxy_address;
	unsigned int http_proxy_port;
	int gid;
	int uid;
	unsigned int options;
	void *user;
	int ka_time;
	int ka_probes;
	int ka_interval;
#ifdef LWS_OPENSSL_SUPPORT
	SSL_CTX *provided_client_ssl_ctx;
#else /* maintain structure layout either way */
    	void *provided_client_ssl_ctx;
#endif
};

LWS_VISIBLE LWS_EXTERN
void lws_set_log_level(int level,
			void (*log_emit_function)(int level, const char *line));

LWS_VISIBLE LWS_EXTERN void
lwsl_emit_syslog(int level, const char *line);

LWS_VISIBLE LWS_EXTERN struct libwebsocket_context *
libwebsocket_create_context(struct lws_context_creation_info *info);
	
LWS_VISIBLE LWS_EXTERN int
libwebsocket_set_proxy(struct libwebsocket_context *context, const char *proxy);

LWS_VISIBLE LWS_EXTERN void
libwebsocket_context_destroy(struct libwebsocket_context *context);

LWS_VISIBLE LWS_EXTERN int
libwebsocket_service(struct libwebsocket_context *context, int timeout_ms);

LWS_VISIBLE LWS_EXTERN void
libwebsocket_cancel_service(struct libwebsocket_context *context);

LWS_VISIBLE LWS_EXTERN const unsigned char *
lws_token_to_string(enum lws_token_indexes token);

LWS_VISIBLE LWS_EXTERN int
lws_add_http_header_by_name(struct libwebsocket_context *context,
			    struct libwebsocket *wsi,
			    const unsigned char *name,
			    const unsigned char *value,
			    int length,
			    unsigned char **p,
			    unsigned char *end);
LWS_VISIBLE LWS_EXTERN int 
lws_finalize_http_header(struct libwebsocket_context *context,
			    struct libwebsocket *wsi,
			    unsigned char **p,
			    unsigned char *end);
LWS_VISIBLE LWS_EXTERN int
lws_add_http_header_by_token(struct libwebsocket_context *context,
			    struct libwebsocket *wsi,
			    enum lws_token_indexes token,
			    const unsigned char *value,
			    int length,
			    unsigned char **p,
			    unsigned char *end);
LWS_VISIBLE LWS_EXTERN int lws_add_http_header_content_length(struct libwebsocket_context *context,
			    struct libwebsocket *wsi,
			    unsigned long content_length,
			    unsigned char **p,
			    unsigned char *end);
LWS_VISIBLE LWS_EXTERN int
lws_add_http_header_status(struct libwebsocket_context *context,
			    struct libwebsocket *wsi,
			    unsigned int code,
			    unsigned char **p,
			    unsigned char *end);

LWS_EXTERN int lws_http_transaction_completed(struct libwebsocket *wsi);

#ifdef LWS_USE_LIBEV
typedef void (lws_ev_signal_cb)(EV_P_ struct ev_signal *w, int revents);

LWS_VISIBLE LWS_EXTERN int
libwebsocket_sigint_cfg(
	struct libwebsocket_context *context,
	int use_ev_sigint,
	lws_ev_signal_cb* cb);

LWS_VISIBLE LWS_EXTERN int
libwebsocket_initloop(
	struct libwebsocket_context *context, struct ev_loop *loop);

LWS_VISIBLE void
libwebsocket_sigint_cb(
	struct ev_loop *loop, struct ev_signal *watcher, int revents);
#endif /* LWS_USE_LIBEV */

LWS_VISIBLE LWS_EXTERN int
libwebsocket_service_fd(struct libwebsocket_context *context,
							 struct libwebsocket_pollfd *pollfd);

LWS_VISIBLE LWS_EXTERN void *
libwebsocket_context_user(struct libwebsocket_context *context);

enum pending_timeout {
	NO_PENDING_TIMEOUT = 0,
	PENDING_TIMEOUT_AWAITING_PROXY_RESPONSE,
	PENDING_TIMEOUT_AWAITING_CONNECT_RESPONSE,
	PENDING_TIMEOUT_ESTABLISH_WITH_SERVER,
	PENDING_TIMEOUT_AWAITING_SERVER_RESPONSE,
	PENDING_TIMEOUT_AWAITING_PING,
	PENDING_TIMEOUT_CLOSE_ACK,
	PENDING_TIMEOUT_AWAITING_EXTENSION_CONNECT_RESPONSE,
	PENDING_TIMEOUT_SENT_CLIENT_HANDSHAKE,
	PENDING_TIMEOUT_SSL_ACCEPT,
	PENDING_TIMEOUT_HTTP_CONTENT,
	PENDING_TIMEOUT_AWAITING_CLIENT_HS_SEND,
	PENDING_FLUSH_STORED_SEND_BEFORE_CLOSE,
};

LWS_VISIBLE LWS_EXTERN void
libwebsocket_set_timeout(struct libwebsocket *wsi,
					 enum pending_timeout reason, int secs);

/*
 * IMPORTANT NOTICE!
 *
 * When sending with websocket protocol (LWS_WRITE_TEXT or LWS_WRITE_BINARY)
 * the send buffer has to have LWS_SEND_BUFFER_PRE_PADDING bytes valid BEFORE
 * buf, and LWS_SEND_BUFFER_POST_PADDING bytes valid AFTER (buf + len).
 *
 * This allows us to add protocol info before and after the data, and send as
 * one packet on the network without payload copying, for maximum efficiency.
 *
 * So for example you need this kind of code to use libwebsocket_write with a
 * 128-byte payload
 *
 *   char buf[LWS_SEND_BUFFER_PRE_PADDING + 128 + LWS_SEND_BUFFER_POST_PADDING];
 *
 *   // fill your part of the buffer... for example here it's all zeros
 *   memset(&buf[LWS_SEND_BUFFER_PRE_PADDING], 0, 128);
 *
 *   libwebsocket_write(wsi, &buf[LWS_SEND_BUFFER_PRE_PADDING], 128,
 *   								LWS_WRITE_TEXT);
 *
 * When sending LWS_WRITE_HTTP, there is no protocol addition and you can just
 * use the whole buffer without taking care of the above.
 */

/*
 * this is the frame nonce plus two header plus 8 length
 *   there's an additional two for mux extension per mux nesting level
 * 2 byte prepend on close will already fit because control frames cannot use
 * the big length style
 */

// Pad LWS_SEND_BUFFER_PRE_PADDING to the CPU word size, so that word references
// to the address immediately after the padding won't cause an unaligned access
// error. Sometimes the recommended padding is even larger than the size of a void *.
// For example, for the X86-64 architecture, in Intel's document
// https://software.intel.com/en-us/articles/data-alignment-when-migrating-to-64-bit-intel-architecture
// they recommend that structures larger than 16 bytes be aligned to 16-byte
// boundaries.
// 

#if !defined(LWS_SIZEOFPTR)
#define LWS_SIZEOFPTR (sizeof (void *))
#endif
#if !defined(u_int64_t)
#define u_int64_t unsigned long long
#endif

#if __x86_64__
#define _LWS_PAD_SIZE 16       // Intel recommended for best performance.
#else
#define _LWS_PAD_SIZE LWS_SIZEOFPTR   /* Size of a pointer on the target architecture */
#endif
#define _LWS_PAD(n) (((n) % _LWS_PAD_SIZE) ? ((n) + (_LWS_PAD_SIZE - ((n) % _LWS_PAD_SIZE))) : (n))
#define LWS_SEND_BUFFER_PRE_PADDING _LWS_PAD(4 + 10 + (2 * MAX_MUX_RECURSION))
#define LWS_SEND_BUFFER_POST_PADDING 4

LWS_VISIBLE LWS_EXTERN int
libwebsocket_write(struct libwebsocket *wsi, unsigned char *buf, size_t len,
				     enum libwebsocket_write_protocol protocol);

/* helper for case where buffer may be const */
#define libwebsocket_write_http(wsi, buf, len) \
	libwebsocket_write(wsi, (unsigned char *)(buf), len, LWS_WRITE_HTTP)

LWS_VISIBLE LWS_EXTERN int
libwebsockets_serve_http_file(struct libwebsocket_context *context,
			struct libwebsocket *wsi, const char *file,
			const char *content_type, const char *other_headers,
			int other_headers_len);
LWS_VISIBLE LWS_EXTERN int
libwebsockets_serve_http_file_fragment(struct libwebsocket_context *context,
			struct libwebsocket *wsi);

LWS_VISIBLE LWS_EXTERN int libwebsockets_return_http_status(
		struct libwebsocket_context *context,
			struct libwebsocket *wsi, unsigned int code,
							const char *html_body);

LWS_VISIBLE LWS_EXTERN const struct libwebsocket_protocols *
libwebsockets_get_protocol(struct libwebsocket *wsi);

LWS_VISIBLE LWS_EXTERN int
libwebsocket_callback_on_writable(struct libwebsocket_context *context,
						      struct libwebsocket *wsi);

LWS_VISIBLE LWS_EXTERN int
libwebsocket_callback_on_writable_all_protocol(
				 const struct libwebsocket_protocols *protocol);

LWS_VISIBLE LWS_EXTERN int
libwebsocket_callback_all_protocol(
		const struct libwebsocket_protocols *protocol, int reason);

LWS_VISIBLE LWS_EXTERN int
libwebsocket_get_socket_fd(struct libwebsocket *wsi);

LWS_VISIBLE LWS_EXTERN int
libwebsocket_is_final_fragment(struct libwebsocket *wsi);

LWS_VISIBLE LWS_EXTERN unsigned char
libwebsocket_get_reserved_bits(struct libwebsocket *wsi);

LWS_VISIBLE LWS_EXTERN int
libwebsocket_rx_flow_control(struct libwebsocket *wsi, int enable);

LWS_VISIBLE LWS_EXTERN void
libwebsocket_rx_flow_allow_all_protocol(
				const struct libwebsocket_protocols *protocol);

LWS_VISIBLE LWS_EXTERN size_t
libwebsockets_remaining_packet_payload(struct libwebsocket *wsi);

/*
 * if the protocol does not have any guidence, returns -1.  Currently only
 * http2 connections get send window information from this API.  But your code
 * should use it so it can work properly with any protocol.
 * 
 * If nonzero return is the amount of payload data the peer or intermediary has
 * reported it has buffer space for.  That has NO relationship with the amount
 * of buffer space your OS can accept on this connection for a write action.
 * 
 * This number represents the maximum you could send to the peer or intermediary
 * on this connection right now without it complaining.
 * 
 * lws manages accounting for send window updates and payload writes
 * automatically, so this number reflects the situation at the peer or
 * intermediary dynamically.
 */
LWS_VISIBLE LWS_EXTERN size_t
lws_get_peer_write_allowance(struct libwebsocket *wsi);

LWS_VISIBLE LWS_EXTERN struct libwebsocket *
libwebsocket_client_connect(struct libwebsocket_context *clients,
			      const char *address,
			      int port,
			      int ssl_connection,
			      const char *path,
			      const char *host,
			      const char *origin,
			      const char *protocol,
			      int ietf_version_or_minus_one);

LWS_VISIBLE LWS_EXTERN struct libwebsocket *
libwebsocket_client_connect_extended(struct libwebsocket_context *clients,
			      const char *address,
			      int port,
			      int ssl_connection,
			      const char *path,
			      const char *host,
			      const char *origin,
			      const char *protocol,
			      int ietf_version_or_minus_one,
			      void *userdata);

LWS_VISIBLE LWS_EXTERN const char *
libwebsocket_canonical_hostname(struct libwebsocket_context *context);


LWS_VISIBLE LWS_EXTERN void
libwebsockets_get_peer_addresses(struct libwebsocket_context *context,
		struct libwebsocket *wsi, lws_sockfd_type fd, char *name, int name_len,
					char *rip, int rip_len);

LWS_VISIBLE LWS_EXTERN int
libwebsockets_get_random(struct libwebsocket_context *context,
							    void *buf, int len);

LWS_VISIBLE LWS_EXTERN int
lws_daemonize(const char *_lock_path);

LWS_VISIBLE LWS_EXTERN int
lws_send_pipe_choked(struct libwebsocket *wsi);

LWS_VISIBLE LWS_EXTERN int
lws_partial_buffered(struct libwebsocket *wsi);

LWS_VISIBLE LWS_EXTERN int
lws_frame_is_binary(struct libwebsocket *wsi);

LWS_VISIBLE LWS_EXTERN int
lws_is_ssl(struct libwebsocket *wsi);
#ifdef LWS_SHA1_USE_OPENSSL_NAME
#define libwebsockets_SHA1 SHA1
#else
LWS_VISIBLE LWS_EXTERN unsigned char *
libwebsockets_SHA1(const unsigned char *d, size_t n, unsigned char *md);
#endif

LWS_VISIBLE LWS_EXTERN int
lws_b64_encode_string(const char *in, int in_len, char *out, int out_size);

LWS_VISIBLE LWS_EXTERN int
lws_b64_decode_string(const char *in, char *out, int out_size);

LWS_VISIBLE LWS_EXTERN const char *
lws_get_library_version(void);

/* access to headers... only valid while headers valid */

LWS_VISIBLE LWS_EXTERN int
lws_hdr_total_length(struct libwebsocket *wsi, enum lws_token_indexes h);

LWS_VISIBLE LWS_EXTERN int
lws_hdr_copy(struct libwebsocket *wsi, char *dest, int len,
						enum lws_token_indexes h);

/*
 * Note: this is not normally needed as a user api.  It's provided in case it is
 * useful when integrating with other app poll loop service code.
 */

LWS_VISIBLE LWS_EXTERN int
libwebsocket_read(struct libwebsocket_context *context,
				struct libwebsocket *wsi,
					       unsigned char *buf, size_t len);

#ifndef LWS_NO_EXTENSIONS
LWS_VISIBLE LWS_EXTERN struct libwebsocket_extension *libwebsocket_get_internal_extensions();
#endif

/*
 * custom allocator support
 */
LWS_VISIBLE LWS_EXTERN void
lws_set_allocator(void *(*realloc)(void *ptr, size_t size));

#ifdef __cplusplus
}
#endif

#endif

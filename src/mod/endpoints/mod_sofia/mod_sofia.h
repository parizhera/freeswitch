/* 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2009, Anthony Minessale II <anthm@freeswitch.org>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthm@freeswitch.org>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * 
 * Anthony Minessale II <anthm@freeswitch.org>
 * Ken Rice, Asteria Solutions Group, Inc <ken@asteriasgi.com>
 * Paul D. Tinsley <pdt at jackhammer.org>
 * Bret McDanel <trixter AT 0xdecafbad.com>
 * Marcel Barbulescu <marcelbarbulescu@gmail.com>
 *
 *
 * mod_sofia.h -- SOFIA SIP Endpoint
 *
 */

/*Defines etc..*/
/*************************************************************************************************************************************************************/
#define MANUAL_BYE

#define IREG_SECONDS 30
#define GATEWAY_SECONDS 1
#define SOFIA_QUEUE_SIZE 50000
#define HAVE_APR
#include <switch.h>
#include <switch_version.h>
#define SOFIA_NAT_SESSION_TIMEOUT 20
#define SOFIA_MAX_ACL 100
#ifdef _MSC_VER
#define HAVE_FUNCTION 1
#else
#define HAVE_FUNC 1
#endif

#define MAX_CODEC_CHECK_FRAMES 50
#define MAX_MISMATCH_FRAMES 5
#define MODNAME "mod_sofia"
#define SOFIA_DEFAULT_CONTACT_USER MODNAME
static const switch_state_handler_table_t noop_state_handler = { 0 };
struct sofia_gateway;
typedef struct sofia_gateway sofia_gateway_t;

struct sofia_gateway_subscription;
typedef struct sofia_gateway_subscription sofia_gateway_subscription_t;

struct sofia_profile;
typedef struct sofia_profile sofia_profile_t;
#define NUA_MAGIC_T sofia_profile_t

typedef struct sofia_private sofia_private_t;

struct private_object;
typedef struct private_object private_object_t;
#define NUA_HMAGIC_T sofia_private_t

#define SOFIA_SESSION_TIMEOUT "sofia_session_timeout"
#define MY_EVENT_REGISTER "sofia::register"
#define MY_EVENT_UNREGISTER "sofia::unregister"
#define MY_EVENT_EXPIRE "sofia::expire"
#define MY_EVENT_GATEWAY_STATE "sofia::gateway_state"
#define MY_EVENT_NOTIFY_REFER "sofia::notify_refer"

#define MULTICAST_EVENT "multicast::event"
#define SOFIA_REPLACES_HEADER "_sofia_replaces_"
#define SOFIA_USER_AGENT "FreeSWITCH-mod_sofia/" SWITCH_VERSION_MAJOR "." SWITCH_VERSION_MINOR "." SWITCH_VERSION_MICRO "-" SWITCH_VERSION_REVISION
#define SOFIA_CHAT_PROTO "sip"
#define SOFIA_SIP_HEADER_PREFIX "sip_h_"
#define SOFIA_SIP_RESPONSE_HEADER_PREFIX "sip_rh_"
#define SOFIA_SIP_BYE_HEADER_PREFIX "sip_bye_h_"
#define SOFIA_SIP_PROGRESS_HEADER_PREFIX "sip_ph_"
#define SOFIA_SIP_HEADER_PREFIX_T "~sip_h_"
#define SOFIA_DEFAULT_PORT "5060"
#define SOFIA_DEFAULT_TLS_PORT "5061"
#define SOFIA_REFER_TO_VARIABLE "sip_refer_to"
#define SOFIA_SECURE_MEDIA_VARIABLE "sip_secure_media"
#define SOFIA_SECURE_MEDIA_CONFIRMED_VARIABLE "sip_secure_media_confirmed"
#define SOFIA_HAS_CRYPTO_VARIABLE "sip_has_crypto"
#define SOFIA_CRYPTO_MANDATORY_VARIABLE "sip_crypto_mandatory"
#define SOFIA_ACTUALLY_SUPPORT "UPDATE"

#include <sofia-sip/nua.h>
#include <sofia-sip/sip_status.h>
#include <sofia-sip/sdp.h>
#include <sofia-sip/sip_protos.h>
#include <sofia-sip/auth_module.h>
#include <sofia-sip/su_md5.h>
#include <sofia-sip/su_log.h>
#include <sofia-sip/nea.h>
#include <sofia-sip/msg_addr.h>
#include <sofia-sip/tport_tag.h>
#include <sofia-sip/sip_extra.h>
#include "nua_stack.h"
#include "sofia-sip/msg_parser.h"
#include "sofia-sip/sip_parser.h"
#include "sofia-sip/tport_tag.h"
#include <sofia-sip/msg.h>

typedef enum {
	DTMF_2833,
	DTMF_INFO,
	DTMF_NONE
} sofia_dtmf_t;

struct sofia_private {
	char uuid[SWITCH_UUID_FORMATTED_LENGTH + 1];
	sofia_gateway_t *gateway;
	char gateway_name[256];
	char auth_gateway_name[256];
	int destroy_nh;
	int destroy_me;
	int is_call;
	int is_static;
};

#define set_param(ptr,val) if (ptr) {free(ptr) ; ptr = NULL;} if (val) {ptr = strdup(val);}
#define set_anchor(t,m) if (t->Anchor) {delete t->Anchor;} t->Anchor = new SipMessage(m);
#define sofia_private_free(_pvt) if (_pvt && ! _pvt->is_static) {free(_pvt); _pvt = NULL;}

/* Local Structures */
/*************************************************************************************************************************************************************/
struct sip_alias_node {
	char *url;
	nua_t *nua;
	struct sip_alias_node *next;
};

typedef struct sip_alias_node sip_alias_node_t;

typedef enum {
	MFLAG_REFER = (1 << 0),
	MFLAG_REGISTER = (1 << 1),
	MFLAG_UPDATE = (1 << 2)
} MFLAGS;

typedef enum {
	PFLAG_AUTH_CALLS,
	PFLAG_BLIND_REG,
	PFLAG_AUTH_ALL,
	PFLAG_FULL_ID,
	PFLAG_MULTIREG_CONTACT,
	PFLAG_PASS_RFC2833,
	PFLAG_DISABLE_TRANSCODING,
	PFLAG_REWRITE_TIMESTAMPS,
	PFLAG_RUNNING,
	PFLAG_RESPAWN,
	PFLAG_GREEDY,
	PFLAG_MULTIREG,
	PFLAG_SUPPRESS_CNG,
	PFLAG_TLS,
	PFLAG_CHECKUSER,
	PFLAG_SECURE,
	PFLAG_BLIND_AUTH,
	PFLAG_WORKER_RUNNING,
	PFLAG_UNREG_OPTIONS_FAIL,
	PFLAG_DISABLE_TIMER,
	PFLAG_DISABLE_100REL,
	PFLAG_AGGRESSIVE_NAT_DETECTION,
	PFLAG_RECIEVED_IN_NAT_REG_CONTACT,
	PFLAG_3PCC,
	PFLAG_DISABLE_RTP_AUTOADJ,
	PFLAG_DISABLE_SRTP_AUTH,
	PFLAG_FUNNY_STUN,
	PFLAG_STUN_ENABLED,
	PFLAG_STUN_AUTO_DISABLE,
	PFLAG_3PCC_PROXY,
	PFLAG_CALLID_AS_UUID,
	PFLAG_UUID_AS_CALLID,
	PFLAG_SCROOGE,
	PFLAG_MANAGE_SHARED_APPEARANCE,
	PFLAG_DISABLE_SRV,
	PFLAG_DISABLE_NAPTR,
	PFLAG_AUTOFLUSH,
	PFLAG_NAT_OPTIONS_PING,
	PFLAG_AUTOFIX_TIMING,
	PFLAG_MESSAGE_QUERY_ON_REGISTER,
	PFLAG_RTP_AUTOFLUSH_DURING_BRIDGE,
	PFLAG_MANUAL_REDIRECT,
	PFLAG_AUTO_NAT,
	PFLAG_SIPCOMPACT,
	/* No new flags below this line */
	PFLAG_MAX
} PFLAGS;

typedef enum {
	PFLAG_NDLB_TO_IN_200_CONTACT = (1 << 0),
	PFLAG_NDLB_BROKEN_AUTH_HASH = (1 << 1),
	PFLAG_NDLB_SENDRECV_IN_SESSION = (1 << 2)
} sofia_NDLB_t;

typedef enum {
	STUN_FLAG_SET = (1 << 0),
	STUN_FLAG_PING = (1 << 1),
	STUN_FLAG_FUNNY = (1 << 2)
} STUNFLAGS;

typedef enum {
	TFLAG_IO,
	TFLAG_CHANGE_MEDIA,
	TFLAG_OUTBOUND,
	TFLAG_READING,
	TFLAG_WRITING,
	TFLAG_HUP,
	TFLAG_RTP,
	TFLAG_BYE,
	TFLAG_ANS,
	TFLAG_EARLY_MEDIA,
	TFLAG_SECURE,
	TFLAG_VAD_IN,
	TFLAG_VAD_OUT,
	TFLAG_VAD,
	TFLAG_3PCC,
	TFLAG_READY,
	TFLAG_REINVITE,
	TFLAG_REFER,
	TFLAG_NOHUP,
	TFLAG_NOSDP_REINVITE,
	TFLAG_NAT,
	TFLAG_USEME,
	TFLAG_SIP_HOLD,
	TFLAG_INB_NOMEDIA,
	TFLAG_LATE_NEGOTIATION,
	TFLAG_SDP,
	TFLAG_VIDEO,
	TFLAG_TPORT_LOG,
	TFLAG_SENT_UPDATE,
	TFLAG_PROXY_MEDIA,
	TFLAG_HOLD_LOCK,
	TFLAG_3PCC_HAS_ACK,
	TFLAG_PASS_RFC2833,

	/* No new flags below this line */
	TFLAG_MAX
} TFLAGS;

struct mod_sofia_globals {
	switch_memory_pool_t *pool;
	switch_hash_t *profile_hash;
	switch_hash_t *gateway_hash;
	switch_mutex_t *hash_mutex;
	uint32_t callid;
	int32_t running;
	int32_t threads;
	switch_mutex_t *mutex;
	char guess_ip[80];
	char hostname[512];
	switch_queue_t *presence_queue;
	switch_queue_t *mwi_queue;
	struct sofia_private destroy_private;
	struct sofia_private keep_private;
	switch_event_node_t *in_node;
	switch_event_node_t *probe_node;
	switch_event_node_t *out_node;
	switch_event_node_t *roster_node;
	switch_event_node_t *custom_node;
	switch_event_node_t *mwi_node;
	int guess_mask;
	char guess_mask_str[16];
	int debug_presence;
	int auto_restart;
	int auto_nat;
	int tracelevel;
	int rewrite_multicasted_fs_path;
};
extern struct mod_sofia_globals mod_sofia_globals;

typedef enum {
	REG_FLAG_AUTHED,
	REG_FLAG_CALLERID,

	/* No new flags below this line */
	REG_FLAG_MAX
} reg_flags_t;

typedef enum {
	REG_STATE_UNREGED,
	REG_STATE_TRYING,
	REG_STATE_REGISTER,
	REG_STATE_REGED,
	REG_STATE_UNREGISTER,
	REG_STATE_FAILED,
	REG_STATE_FAIL_WAIT,
	REG_STATE_EXPIRED,
	REG_STATE_NOREG,
	REG_STATE_LAST
} reg_state_t;

typedef enum {
	SOFIA_TRANSPORT_UNKNOWN = 0,
	SOFIA_TRANSPORT_UDP,
	SOFIA_TRANSPORT_TCP,
	SOFIA_TRANSPORT_TCP_TLS,
	SOFIA_TRANSPORT_SCTP
} sofia_transport_t;

typedef enum {
	SOFIA_GATEWAY_DOWN,
	SOFIA_GATEWAY_UP
} sofia_gateway_status_t;

typedef enum {
	SUB_STATE_UNSUBED,
	SUB_STATE_TRYING,
	SUB_STATE_SUBSCRIBE,
	SUB_STATE_SUBED,
	SUB_STATE_UNSUBSCRIBE,
	SUB_STATE_FAILED,
	SUB_STATE_EXPIRED,
	SUB_STATE_NOSUB,
	v_STATE_LAST
} sub_state_t;

struct sofia_gateway_subscription {
	sofia_gateway_t *gateway;
	char *expires_str;
	char *event;  /* eg, 'message-summary' to subscribe to MWI events */
	char *content_type;  /* eg, application/simple-message-summary in the case of MWI events */
	uint32_t freq;
	int32_t retry_seconds;
	time_t expires;
	time_t retry;
	sub_state_t state;
	struct sofia_gateway_subscription *next;
};

struct sofia_gateway {
	sofia_private_t *sofia_private;
	nua_handle_t *nh;
	nua_handle_t *sub_nh;
	sofia_profile_t *profile;
	char *name;
	char *register_scheme;
	char *register_realm;
	char *register_username;
	char *auth_username;
	char *register_password;
	char *register_from;
	char *register_contact;
	char *extension;
	char *register_to;
	char *register_proxy;
	char *register_sticky_proxy;
	char *outbound_sticky_proxy;
	char *register_context;
	char *expires_str;
	char *register_url;
	char *from_domain;
	sofia_transport_t register_transport;
	uint32_t freq;
	time_t expires;
	time_t retry;
	time_t ping;
	int pinging;
	sofia_gateway_status_t status;
	uint32_t ping_freq;
	uint8_t flags[REG_FLAG_MAX];
	int32_t retry_seconds;
	reg_state_t state;
	switch_memory_pool_t *pool;
	int deleted;
	switch_event_t *ib_vars;
	switch_event_t *ob_vars;
	uint32_t ib_calls;
	uint32_t ob_calls;
	char uuid_str[SWITCH_UUID_FORMATTED_LENGTH + 1];
	int failures;
	struct sofia_gateway *next;
	sofia_gateway_subscription_t *subscriptions;
};

typedef enum {
	PRES_TYPE_NONE = 0,
	PRES_TYPE_FULL = 1,
	PRES_TYPE_PASSIVE = 2
} sofia_presence_type_t;

typedef enum {
	MEDIA_OPT_NONE = 0,
	MEDIA_OPT_MEDIA_ON_HOLD = (1 << 0),
	MEDIA_OPT_BYPASS_AFTER_ATT_XFER = (1 << 1)
} sofia_media_options_t;

typedef enum {
	CID_TYPE_RPID,
	CID_TYPE_PID,
	CID_TYPE_NONE
} sofia_cid_type_t;

struct sofia_profile {
	int debug;
	char *name;
	char *domain_name;
	char *dbname;
	char *dialplan;
	char *context;
	char *extrtpip;
	char *rtpip;
	char *sipip;
	char *extsipip;
	char *username;
	char *url;
	char *public_url;
	char *bindurl;
	char *tls_url;
	char *tls_public_url;
	char *tls_bindurl;
	char *tcp_public_contact;
	char *tls_public_contact;
	char *tcp_contact;
	char *tls_contact;
	char *sla_contact;
	char *sipdomain;
	char *timer_name;
	char *hold_music;
	char *outbound_proxy;
	char *bind_params;
	char *tls_bind_params;
	char *tls_cert_dir;
	char *reg_domain;
	char *sub_domain;
	char *reg_db_domain;
	char *user_agent;
	char *record_template;
	char *presence_hosts;
	char *challenge_realm;
	sofia_cid_type_t cid_type;
	sofia_dtmf_t dtmf_type;
	int auto_restart;
	switch_port_t sip_port;
	switch_port_t tls_sip_port;
	int tls_version;
	char *codec_string;
	int running;
	int dtmf_duration;
	uint8_t flags[TFLAG_MAX];
	uint8_t pflags[PFLAG_MAX];
	unsigned int mflags;
	unsigned int ndlb;
	uint32_t max_calls;
	uint32_t nonce_ttl;
	nua_t *nua;
	switch_memory_pool_t *pool;
	su_root_t *s_root;
	sip_alias_node_t *aliases;
	switch_payload_t te;
	switch_payload_t cng_pt;
	uint32_t codec_flags;
	switch_mutex_t *ireg_mutex;
	switch_mutex_t *gateway_mutex;
	sofia_gateway_t *gateways;
	su_home_t *home;
	switch_hash_t *chat_hash;
	switch_core_db_t *master_db;
	switch_thread_rwlock_t *rwlock;
	switch_mutex_t *flag_mutex;
	uint32_t inuse;
	time_t started;
	uint32_t session_timeout;
	uint32_t minimum_session_expires;
	uint32_t max_proceeding;
	uint32_t rtp_timeout_sec;
	uint32_t rtp_hold_timeout_sec;
	char *odbc_dsn;
	char *odbc_user;
	char *odbc_pass;
	switch_odbc_handle_t *master_odbc;
	switch_queue_t *sql_queue;
	char *acl[SOFIA_MAX_ACL];
	uint32_t acl_count;
	char *proxy_acl[SOFIA_MAX_ACL];
	uint32_t proxy_acl_count;
	char *reg_acl[SOFIA_MAX_ACL];
	uint32_t reg_acl_count;
	char *nat_acl[SOFIA_MAX_ACL];
	uint32_t nat_acl_count;
	int rport_level;
	sofia_presence_type_t pres_type;
	sofia_media_options_t media_options;
	uint32_t force_subscription_expires;
	switch_rtp_bug_flag_t auto_rtp_bugs;
	uint32_t ib_calls;
	uint32_t ob_calls;
	uint32_t ib_failed_calls;
	uint32_t ob_failed_calls;
	uint32_t timer_t1;
	uint32_t timer_t1x64;
	uint32_t timer_t2;
	uint32_t timer_t4;
	char *contact_user;
	char *local_network;
};

struct private_object {
	sofia_private_t *sofia_private;
	uint8_t flags[TFLAG_MAX];
	switch_payload_t agreed_pt;
	switch_core_session_t *session;
	switch_channel_t *channel;
	switch_frame_t read_frame;
	char *codec_order[SWITCH_MAX_CODECS];
	int codec_order_last;
	const switch_codec_implementation_t *codecs[SWITCH_MAX_CODECS];
	int num_codecs;
	switch_codec_t read_codec;
	switch_codec_t write_codec;
	uint32_t codec_ms;
	switch_caller_profile_t *caller_profile;
	uint32_t timestamp_send;
	switch_rtp_t *rtp_session;
	int ssrc;
	sofia_profile_t *profile;
	char *local_sdp_audio_ip;
	switch_port_t local_sdp_audio_port;
	char *remote_sdp_audio_ip;
	switch_port_t remote_sdp_audio_port;
	char *adv_sdp_audio_ip;
	switch_port_t adv_sdp_audio_port;
	char *proxy_sdp_audio_ip;
	switch_port_t proxy_sdp_audio_port;
	char *reply_contact;
	char *from_uri;
	char *to_uri;
	char *from_address;
	char *to_address;
	char *callid;
	char *contact_url;
	char *from_str;
	char *rpid;
	char *asserted_id;
	char *preferred_id;
	char *privacy;
	char *gateway_from_str;
	char *rm_encoding;
	char *iananame;
	char *rm_fmtp;
	char *fmtp_out;
	char *remote_sdp_str;
	int crypto_tag;
	unsigned char local_raw_key[SWITCH_RTP_MAX_CRYPTO_LEN];
	unsigned char remote_raw_key[SWITCH_RTP_MAX_CRYPTO_LEN];
	switch_rtp_crypto_key_type_t crypto_send_type;
	switch_rtp_crypto_key_type_t crypto_recv_type;
	switch_rtp_crypto_key_type_t crypto_type;
	char *local_sdp_str;
	char *dest;
	char *dest_to;
	char *key;
	char *xferto;
	char *kick;
	char *origin;
	char *hash_key;
	char *chat_from;
	char *chat_to;
	char *e_dest;
	char *call_id;
	char *invite_contact;
	char *local_url;
	char *gateway_name;
	char *local_crypto_key;
	char *remote_crypto_key;
	char *record_route;
	char *extrtpip;
	char *stun_ip;
	char *route_uri;
	char *x_actually_support_remote;
	char *x_actually_support_local;
	switch_port_t stun_port;
	uint32_t stun_flags;
	unsigned long rm_rate;
	switch_payload_t pt;
	switch_mutex_t *flag_mutex;
	switch_mutex_t *sofia_mutex;
	switch_payload_t te;
	switch_payload_t bte;
	switch_payload_t cng_pt;
	switch_payload_t bcng_pt;
	sofia_transport_t transport;
	nua_handle_t *nh;
	nua_handle_t *nh2;
	sip_contact_t *contact;
	uint32_t owner_id;
	uint32_t session_id;
	uint32_t max_missed_packets;
	uint32_t max_missed_hold_packets;
	/** VIDEO **/
	switch_frame_t video_read_frame;
	switch_codec_t video_read_codec;
	switch_codec_t video_write_codec;
	switch_rtp_t *video_rtp_session;
	switch_port_t adv_sdp_video_port;
	switch_port_t local_sdp_video_port;
	char *video_rm_encoding;
	switch_payload_t video_pt;
	unsigned long video_rm_rate;
	uint32_t video_codec_ms;
	char *remote_sdp_video_ip;
	switch_port_t remote_sdp_video_port;
	char *video_rm_fmtp;
	switch_payload_t video_agreed_pt;
	char *video_fmtp_out;
	uint32_t video_count;
	sofia_dtmf_t dtmf_type;
	int q850_cause;
	char *remote_ip;
	int remote_port;
	int got_bye;
	int hold_laps;
	switch_thread_id_t locker;
	switch_size_t last_ts;
	uint32_t check_frames;
	uint32_t mismatch_count;
	uint32_t last_codec_ms;
	nua_event_t want_event;
	switch_rtp_bug_flag_t rtp_bugs;
	switch_codec_implementation_t read_impl;
	switch_codec_implementation_t write_impl;
	char *user_via;
	char *redirected;
};

struct callback_t {
	char *val;
	switch_size_t len;
	int matches;
};

typedef enum {
	REG_REGISTER,
	REG_AUTO_REGISTER,
	REG_INVITE,
} sofia_regtype_t;

typedef enum {
	AUTH_OK,
	AUTH_FORBIDDEN,
	AUTH_STALE,
} auth_res_t;

typedef struct {
        char *to;
        char *contact;
        char *route;
        char *route_uri;
} sofia_destination_t;

#define sofia_test_pflag(obj, flag) ((obj)->pflags[flag] ? 1 : 0)
#define sofia_set_pflag(obj, flag) (obj)->pflags[flag] = 1
#define sofia_set_pflag_locked(obj, flag) assert(obj->flag_mutex != NULL);\
switch_mutex_lock(obj->flag_mutex);\
(obj)->pflags[flag] = 1;\
switch_mutex_unlock(obj->flag_mutex);
#define sofia_clear_pflag_locked(obj, flag) switch_mutex_lock(obj->flag_mutex); (obj)->pflags[flag] = 0; switch_mutex_unlock(obj->flag_mutex);
#define sofia_clear_pflag(obj, flag) (obj)->pflags[flag] = 0

#define sofia_set_flag_locked(obj, flag) assert(obj->flag_mutex != NULL);\
switch_mutex_lock(obj->flag_mutex);\
(obj)->flags[flag] = 1;\
switch_mutex_unlock(obj->flag_mutex);
#define sofia_set_flag(obj, flag) (obj)->flags[flag] = 1
#define sofia_clear_flag(obj, flag) (obj)->flags[flag] = 0
#define sofia_clear_flag_locked(obj, flag) switch_mutex_lock(obj->flag_mutex); (obj)->flags[flag] = 0; switch_mutex_unlock(obj->flag_mutex);
#define sofia_test_flag(obj, flag) ((obj)->flags[flag] ? 1 : 0)

/* Function Prototypes */
/*************************************************************************************************************************************************************/

switch_status_t sofia_glue_activate_rtp(private_object_t *tech_pvt, switch_rtp_flag_t myflags);

void sofia_glue_deactivate_rtp(private_object_t *tech_pvt);

void sofia_glue_set_local_sdp(private_object_t *tech_pvt, const char *ip, uint32_t port, const char *sr, int force);

void sofia_glue_tech_prepare_codecs(private_object_t *tech_pvt);

void sofia_glue_attach_private(switch_core_session_t *session, sofia_profile_t *profile, private_object_t *tech_pvt, const char *channame);

switch_status_t sofia_glue_tech_choose_port(private_object_t *tech_pvt, int force);

switch_status_t sofia_glue_do_invite(switch_core_session_t *session);

uint8_t sofia_glue_negotiate_sdp(switch_core_session_t *session, sdp_session_t *sdp);

void sofia_presence_establish_presence(sofia_profile_t *profile);

void sofia_handle_sip_i_refer(nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, switch_core_session_t *session, sip_t const *sip, tagi_t tags[]);

void sofia_handle_sip_i_info(nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, switch_core_session_t *session, sip_t const *sip, tagi_t tags[]);

void sofia_handle_sip_i_invite(nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, sofia_private_t *sofia_private, sip_t const *sip, tagi_t tags[]);

void sofia_reg_handle_sip_i_register(nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, sofia_private_t *sofia_private, sip_t const *sip, tagi_t tags[]);

void sofia_event_callback(nua_event_t event,
					int status,
					char const *phrase,
					nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, sofia_private_t *sofia_private, sip_t const *sip, tagi_t tags[]);

void *SWITCH_THREAD_FUNC sofia_profile_thread_run(switch_thread_t *thread, void *obj);

void launch_sofia_profile_thread(sofia_profile_t *profile);

switch_status_t sofia_presence_chat_send(const char *proto, const char *from, const char *to, const char *subject,
						  const char *body, const char *type, const char *hint);
void sofia_glue_tech_absorb_sdp(private_object_t *tech_pvt);

/*
 * \brief Sets the "ep_codec_string" channel variable, parsing r_sdp and taing codec_string in consideration 
 * \param channel Current channel
 * \param codec_string The profile's codec string or NULL if inexistant
 * \param sdp The parsed SDP content
 */
void sofia_glue_set_r_sdp_codec_string(switch_channel_t *channel,const char *codec_string, sdp_session_t *sdp);
switch_status_t sofia_glue_tech_media(private_object_t *tech_pvt, const char *r_sdp);
char *sofia_reg_find_reg_url(sofia_profile_t *profile, const char *user, const char *host, char *val, switch_size_t len);
void event_handler(switch_event_t *event);
void sofia_presence_event_handler(switch_event_t *event);
void sofia_presence_mwi_event_handler(switch_event_t *event);
void sofia_presence_cancel(void);
switch_status_t config_sofia(int reload, char *profile_name);
void sofia_reg_auth_challenge(nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, sofia_regtype_t regtype, const char *realm, int stale);
auth_res_t sofia_reg_parse_auth(sofia_profile_t *profile, sip_authorization_t const *authorization, 
								sip_t const *sip, const char *regstr, char *np, size_t nplen, char *ip, switch_event_t **v_event, 
								long exptime, sofia_regtype_t regtype, const char *to_user, switch_event_t **auth_params);
								

void sofia_reg_handle_sip_r_challenge(int status,
									  char const *phrase,
									  nua_t *nua, sofia_profile_t *profile,
									  nua_handle_t *nh, sofia_private_t *sofia_private,
									  switch_core_session_t *session, sofia_gateway_t *gateway, sip_t const *sip, tagi_t tags[]);
void sofia_reg_handle_sip_r_register(int status,
					char const *phrase,
					nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, sofia_private_t *sofia_private, sip_t const *sip, tagi_t tags[]);
void sofia_handle_sip_i_options(int status,
				   char const *phrase,
				   nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, sofia_private_t *sofia_private, sip_t const *sip, tagi_t tags[]);
void sofia_presence_handle_sip_i_publish(nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, sofia_private_t *sofia_private, sip_t const *sip, tagi_t tags[]);
void sofia_presence_handle_sip_i_message(int status,
				   char const *phrase,
				   nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, sofia_private_t *sofia_private, sip_t const *sip, tagi_t tags[]);
void sofia_presence_handle_sip_r_subscribe(int status,
					 char const *phrase,
					 nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, sofia_private_t *sofia_private, sip_t const *sip, tagi_t tags[]);
void sofia_presence_handle_sip_i_subscribe(int status,
					 char const *phrase,
					 nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, sofia_private_t *sofia_private, sip_t const *sip, tagi_t tags[]);

void sofia_glue_execute_sql(sofia_profile_t *profile, char **sqlp, switch_bool_t sql_already_dynamic);
void sofia_glue_actually_execute_sql(sofia_profile_t *profile, switch_bool_t master, char *sql, switch_mutex_t *mutex);
void sofia_reg_check_expire(sofia_profile_t *profile, time_t now, int reboot);
void sofia_reg_check_gateway(sofia_profile_t *profile, time_t now);
void sofia_sub_check_gateway(sofia_profile_t *profile, time_t now);
void sofia_reg_unregister(sofia_profile_t *profile);
switch_status_t sofia_glue_ext_address_lookup(sofia_profile_t *profile, private_object_t *tech_pvt, char **ip, switch_port_t *port, const char *sourceip, switch_memory_pool_t *pool);

void sofia_glue_pass_sdp(private_object_t *tech_pvt, char *sdp);
int sofia_glue_get_user_host(char *in, char **user, char **host);
switch_call_cause_t sofia_glue_sip_cause_to_freeswitch(int status);
void sofia_glue_do_xfer_invite(switch_core_session_t *session);
uint8_t sofia_reg_handle_register(nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, sip_t const *sip, 
								  sofia_regtype_t regtype, char *key, uint32_t keylen, switch_event_t **v_event, const char *is_nat);
extern switch_endpoint_interface_t *sofia_endpoint_interface;
void sofia_presence_set_chat_hash(private_object_t *tech_pvt, sip_t const *sip);
switch_status_t sofia_on_hangup(switch_core_session_t *session);
char *sofia_glue_get_url_from_contact(char *buf, uint8_t to_dup);
void sofia_presence_set_hash_key(char *hash_key, int32_t len, sip_t const *sip);
void sofia_glue_sql_close(sofia_profile_t *profile);
int sofia_glue_init_sql(sofia_profile_t *profile);
char *sofia_overcome_sip_uri_weakness(switch_core_session_t *session, const char *uri, const sofia_transport_t transport, switch_bool_t uri_only, const char *params);
switch_bool_t sofia_glue_execute_sql_callback(sofia_profile_t *profile,
											  switch_bool_t master,
											  switch_mutex_t *mutex,
											  char *sql,
											  switch_core_db_callback_func_t callback,
											  void *pdata);
char *sofia_glue_execute_sql2str(sofia_profile_t *profile, switch_mutex_t *mutex, char *sql, char *resbuf, size_t len);
void sofia_glue_check_video_codecs(private_object_t *tech_pvt);
void sofia_glue_del_profile(sofia_profile_t *profile);

switch_status_t sofia_glue_add_profile(char *key, sofia_profile_t *profile);
void sofia_glue_release_profile__(const char *file, const char *func, int line, sofia_profile_t *profile);
#define sofia_glue_release_profile(x) sofia_glue_release_profile__(__FILE__, __SWITCH_FUNC__, __LINE__,  x)

sofia_profile_t *sofia_glue_find_profile__(const char *file, const char *func, int line, const char *key);
#define sofia_glue_find_profile(x) sofia_glue_find_profile__(__FILE__, __SWITCH_FUNC__, __LINE__,  x)

switch_status_t sofia_reg_add_gateway(char *key, sofia_gateway_t *gateway);
sofia_gateway_t *sofia_reg_find_gateway__(const char *file, const char *func, int line, const char *key);
#define sofia_reg_find_gateway(x) sofia_reg_find_gateway__(__FILE__, __SWITCH_FUNC__, __LINE__,  x)

sofia_gateway_subscription_t *sofia_find_gateway_subscription(sofia_gateway_t *gateway_ptr, const char *event);

void sofia_reg_release_gateway__(const char *file, const char *func, int line, sofia_gateway_t *gateway);
#define sofia_reg_release_gateway(x) sofia_reg_release_gateway__(__FILE__, __SWITCH_FUNC__, __LINE__, x);

#define check_decode(_var, _session) do {								\
		assert(_session);												\
		if (!switch_strlen_zero(_var)) {								\
			int d = 0;													\
			char *p;													\
			if (strchr(_var, '%')) {									\
				char *tmp = switch_core_session_strdup(_session, _var);	\
				switch_url_decode(tmp);									\
				_var = tmp;												\
				d++;													\
			}															\
			if ((p = strchr(_var, '"'))) {								\
				if (!d) {												\
					char *tmp = switch_core_session_strdup(_session, _var); \
					_var = tmp;											\
				}														\
				if ((p = strchr(_var, '"'))) {							\
					_var = p+1;											\
				}														\
				if ((p = strrchr(_var, '"'))) {							\
					*p = '\0';											\
				}														\
			}															\
		}																\
																		\
		if (_session) break;											\
	} while(!_session)


/*
 * Transport handling helper functions
 */
sofia_transport_t sofia_glue_via2transport(const sip_via_t *via);
sofia_transport_t sofia_glue_url2transport(const url_t *url);
sofia_transport_t sofia_glue_str2transport(const char *str);

const char *sofia_glue_transport2str(const sofia_transport_t tp);
char * sofia_glue_find_parameter(const char *str, const char *param);
char *sofia_glue_create_via(switch_core_session_t *session, const char *ip, switch_port_t port, sofia_transport_t transport);
char *sofia_glue_create_external_via(switch_core_session_t *session, sofia_profile_t *profile, sofia_transport_t transport);
char *sofia_glue_strip_uri(const char *str);
int sofia_glue_check_nat(sofia_profile_t *profile, const char *network_ip);
int sofia_glue_transport_has_tls(const sofia_transport_t tp);
const char *sofia_glue_get_unknown_header(sip_t const *sip, const char *name);
switch_status_t sofia_glue_build_crypto(private_object_t *tech_pvt, int index, switch_rtp_crypto_key_type_t type, switch_rtp_crypto_direction_t direction);
void sofia_glue_tech_patch_sdp(private_object_t *tech_pvt);
switch_status_t sofia_glue_tech_proxy_remote_addr(private_object_t *tech_pvt);
void sofia_presence_event_thread_start(void);
void sofia_reg_expire_call_id(sofia_profile_t *profile, const char *call_id, int reboot);
switch_status_t sofia_glue_tech_choose_video_port(private_object_t *tech_pvt, int force);
switch_status_t sofia_glue_tech_set_video_codec(private_object_t *tech_pvt, int force);
const char *sofia_glue_strip_proto(const char *uri);
switch_status_t reconfig_sofia(sofia_profile_t *profile);
void sofia_glue_del_gateway(sofia_gateway_t *gp);
void sofia_reg_send_reboot(sofia_profile_t *profile, const char *user, const char *host, const char *contact, const char *user_agent, const char *network_ip);
void sofia_glue_restart_all_profiles(void);
void sofia_glue_toggle_hold(private_object_t *tech_pvt, int sendonly);
const char * sofia_state_string(int state);
switch_status_t sofia_glue_tech_set_codec(private_object_t *tech_pvt, int force);
void sofia_wait_for_reply(struct private_object *tech_pvt, nua_event_t event, uint32_t timeout);
void sofia_glue_set_image_sdp(private_object_t *tech_pvt, switch_t38_options_t *t38_options);

/*
 * SLA (shared line appearance) entrypoints
 */

void sofia_sla_handle_register(nua_t *nua, sofia_profile_t *profile, sip_t const *sip, long exptime, const char *full_contact);
void sofia_sla_handle_sip_i_publish(nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, sip_t const *sip, tagi_t tags[]);
void sofia_sla_handle_sip_i_subscribe(nua_t *nua, const char *contact_str, sofia_profile_t *profile, nua_handle_t *nh, sip_t const *sip, tagi_t tags[]);
void sofia_sla_handle_sip_r_subscribe(int status,
									  char const *phrase,
									  nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, sofia_private_t *sofia_private, sip_t const *sip, tagi_t tags[]);
void sofia_sla_handle_sip_i_notify(nua_t *nua, sofia_profile_t *profile, nua_handle_t *nh, sip_t const *sip, tagi_t tags[]);

/* 
 * Logging control functions
 */

/*!
 * \brief Changes the loglevel of a sofia component
 * \param name the sofia component on which to change the loglevel, or "all" to change them all
 * \note Valid components are "all", "default" (sofia's default logger), "tport", "iptsec", "nea", "nta", "nth_client", "nth_server", "nua", "soa", "sresolv", "stun"
 * \return SWITCH_STATUS_SUCCESS or SWITCH_STATUS_FALSE if the component isnt valid, or the level is out of range
 */
switch_status_t sofia_set_loglevel(const char *name, int level);

/*!
 * \brief Gets the loglevel of a sofia component
 * \param name the sofia component on which to change the loglevel
 * \note Valid components are "default" (sofia's default logger), "tport", "iptsec", "nea", "nta", "nth_client", "nth_server", "nua", "soa", "sresolv", "stun"
 * \return the component's loglevel, or -1 if the component isn't valid
 */
int sofia_get_loglevel(const char *name);
sofia_cid_type_t sofia_cid_name2type(const char *name);
void sofia_glue_tech_set_local_sdp(private_object_t *tech_pvt, const char *sdp_str, switch_bool_t dup);
void sofia_glue_set_rtp_stats(private_object_t *tech_pvt);
void sofia_glue_get_addr(msg_t *msg, char *buf, size_t buflen, int *port);
sofia_destination_t* sofia_glue_get_destination(char *data);
void sofia_glue_free_destination(sofia_destination_t *dst);
switch_status_t sofia_glue_send_notify(sofia_profile_t *profile, const char *user, const char *host, const char *event, const char *contenttype, const char *body, const char *o_contact, const char *network_ip);
char *sofia_glue_get_extra_headers(switch_channel_t *channel, const char *prefix);
void sofia_glue_set_extra_headers(switch_channel_t *channel, sip_t const *sip, const char *prefix);
void sofia_info_send_sipfrag(switch_core_session_t *aleg, switch_core_session_t *bleg);
void sofia_update_callee_id(switch_core_session_t *session, sofia_profile_t *profile, sip_t const *sip, switch_bool_t send);
void sofia_send_callee_id(switch_core_session_t *session, const char *name, const char *number);

// Copyright 2018 The Cobalt Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef STARBOARD_SHARED_WIDEVINE_DRM_SYSTEM_WIDEVINE_H_
#define STARBOARD_SHARED_WIDEVINE_DRM_SYSTEM_WIDEVINE_H_

#include <map>
#include <string>
#include <vector>

#include "starboard/common/scoped_ptr.h"
#include "starboard/shared/starboard/drm/drm_system_internal.h"
#include "starboard/shared/starboard/thread_checker.h"
#include "starboard/thread.h"
#include "third_party/starboard/amlogic/shared/ce_cdm/cdm/include/cdm.h"
#include "third_party/starboard/amlogic/shared/ce_cdm/util/include/log.h"
#include "third_party/starboard/amlogic/shared/ce_cdm/util/include/string_conversions.h"
#include "third_party/starboard/amlogic/shared/ce_cdm/oemcrypto/include/OEMCryptoCENC.h"
#include "third_party/starboard/amlogic/shared/aml_av_components.h"

namespace starboard {
namespace shared {
namespace widevine {

// Adapts Widevine's |Content Decryption Module v 3.5| to Starboard's
// |SbDrmSystem|.
//
// All |SbDrmSystemPrivate| methods except Decrypt() must be called from the
// constructor thread.
class DrmSystemWidevine : public SbDrmSystemPrivate,
                          private ::widevine::Cdm::IEventListener {
 public:
  DrmSystemWidevine(
      void* context,
      SbDrmSessionUpdateRequestFunc update_request_callback,
      SbDrmSessionUpdatedFunc session_updated_callback
#if SB_HAS(DRM_KEY_STATUSES)
      ,
      SbDrmSessionKeyStatusesChangedFunc key_statuses_changed_callback
#endif  // SB_HAS(DRM_KEY_STATUSES)
#if SB_API_VERSION >= 10
      ,
      SbDrmServerCertificateUpdatedFunc server_certificate_updated_callback
#endif  // SB_API_VERSION >= 10
#if SB_HAS(DRM_SESSION_CLOSED)
      ,
      SbDrmSessionClosedFunc session_closed_callback
#endif  // SB_HAS(DRM_SESSION_CLOSED)
      ,
      const std::string& company_name,
      const std::string& model_name);

  ~DrmSystemWidevine() override;

  static bool IsKeySystemSupported(const char* key_system);

  // From |SbDrmSystemPrivate|.
  void GenerateSessionUpdateRequest(int ticket,
                                    const char* type,
                                    const void* initialization_data,
                                    int initialization_data_size) override;

  void UpdateSession(int ticket,
                     const void* key,
                     int key_size,
                     const void* sb_drm_session_id,
                     int sb_drm_session_id_size) override;

  void CloseSession(const void* sb_drm_session_id,
                    int sb_drm_session_id_size) override;

  DecryptStatus Decrypt(InputBuffer* buffer) override;

#if SB_API_VERSION >= 10
  // This function is called by the app to explicitly set the server
  // certificate.  For an app that supports this feature, it should call this
  // function before calling any other functions like
  // GenerateSessionUpdateRequest().  So we needn't process pending requests in
  // this function.  Note that it is benign if this function is called in
  // parallel with a server certificate request.
  void UpdateServerCertificate(int ticket,
                               const void* certificate,
                               int certificate_size) override;
#endif  // SB_API_VERSION >= 10

#if defined(COBALT_WIDEVINE_OPTEE)
  using AmlAVCodec = ::starboard::shared::starboard::player::filter::AmlAVCodec;
  AmlAVCodec * decoder_;
  void AttachDecoder(AmlAVCodec * decoder) { decoder_ = decoder;}
  static OEMCryptoResult CopyBuffer(uint8_t *out_buffer,
                                    const uint8_t *data_addr,
                                    size_t data_length);
#endif

 private:
  // Stores the data necessary to call GenerateSessionUpdateRequestInternal().
  struct GenerateSessionUpdateRequestData {
    int ticket;
    ::widevine::Cdm::InitDataType init_data_type;
    std::string initialization_data;
  };

  // An unique id to identify the first SbDrm session id when server
  // certificate is not ready.  This is necessary to send the server
  // certificate request, as EME requires a session id while wvcdm cannot
  // generate a session id before having a server certificate.
  // This works with |first_wvcdm_session_id_| to map the first session id
  // between wvcdm and SbDrm.
  static const char kFirstSbDrmSessionId[];

  void GenerateSessionUpdateRequestInternal(
      int ticket,
      ::widevine::Cdm::InitDataType init_data_type,
      const std::string& initialization_data,
      bool is_first_session);

  // From |cdm::IEventListener|.
  // A message (license request, renewal, etc.) to be dispatched to the
  // application's license server. The response, if successful, should be
  // provided back to the CDM via a call to Cdm::update().
  void onMessage(const std::string& session_id,
                 ::widevine::Cdm::MessageType message_type,
                 const std::string& message) override;
  // There has been a change in the keys in the session or their status.
  void onKeyStatusesChange(const std::string& wvcdm_session_id, bool has_new_usable_key) override;
  // A remove() operation has been completed.
  void onRemoveComplete(const std::string& wvcdm_session_id) override;
  // Called when a deferred action has completed.
  void onDeferredComplete(const std::string& wvcdm_session_id,
                          ::widevine::Cdm::Status result) override;
  // Called when the CDM requires a new device certificate.
  void onDirectIndividualizationRequest(const std::string& wvcdm_session_id,
                                        const std::string& request) override;

  void SetTicket(const std::string& sb_drm_session_id, int ticket);
  int GetAndResetTicket(const std::string& sb_drm_session_id);
  std::string WvdmSessionIdToSbDrmSessionId(
      const std::string& wvcdm_session_id);
  std::string SbDrmSessionIdToWvdmSessionId(const void* sb_drm_session_id,
                                            int sb_drm_session_id_size);

  // Generates a special key message to ask for the server certificate.  When
  // the license server receives the request, it will send back the server
  // certificate.
  void SendServerCertificateRequest(int ticket);
  // When this function is called, the update contains the server certificate.
  // The function parses the special update and pass the server certificate to
  // the cdm.
  // Note that the app shouldn't persist the server certificate across playback
  // or across application instances.
  ::widevine::Cdm::Status ProcessServerCertificateResponse(
      const std::string& response);
  // If server certificate has been set, send all pending requests.
  void TrySendPendingGenerateSessionUpdateRequests();
  void SendSessionUpdateRequest(SbDrmSessionRequestType type,
                                const std::string& sb_drm_session_id,
                                const std::string& message);

  ::starboard::shared::starboard::ThreadChecker thread_checker_;
  void* const context_;
  const SbDrmSessionUpdateRequestFunc session_update_request_callback_;
  const SbDrmSessionUpdatedFunc session_updated_callback_;
#if SB_HAS(DRM_KEY_STATUSES)
  const SbDrmSessionKeyStatusesChangedFunc key_statuses_changed_callback_;
#endif  // SB_HAS(DRM_KEY_STATUSES)
#if SB_API_VERSION >= 10
  const SbDrmServerCertificateUpdatedFunc server_certificate_updated_callback_;
#endif  // SB_API_VERSION >= 10
#if SB_HAS(DRM_SESSION_CLOSED)
  const SbDrmSessionClosedFunc session_closed_callback_;
#endif  // SB_HAS(DRM_SESSION_CLOSED)

  // Store a map from session id generated by the cdm to its associated ticket
  // id.  The ticket is a unique id passed to GenerateSessionUpdateRequest() to
  // allow the caller of GenerateSessionUpdateRequest() to associate the
  // session id with the session related data specified by the ticket, as both
  // of them will be passed via session_update_request_callback_ when it is
  // called for the first time for this paritcular session id.  As this is only
  // necessary for the first time the callback is called on the particular
  // session, every time an entry is used, it will be removed from the map.
  // Note that the first callback is always accessed on the thread specificed
  // by |ticket_thread_id_|.
  std::map<std::string, int> sb_drm_session_id_to_ticket_map_;

  // |ticket_| is only valid on the constructor thread within the duration of
  // call to |GenerateKeyRequest| or |AddKey|, but CDM may invoke host's methods
  // spontaneously from the timer thread. In that case |GetTicket| need to
  // return |kSbDrmTicketInvalid|.
  const SbThreadId ticket_thread_id_;

  std::vector<GenerateSessionUpdateRequestData>
      pending_generate_session_update_requests_;
  std::string first_wvcdm_session_id_;

  scoped_ptr<::widevine::Cdm> cdm_;
#if SB_API_VERSION >= 10
  bool is_server_certificate_set_ = false;
#else   // SB_API_VERSION >= 10
  bool is_server_certificate_set_ = true;
#endif  // SB_API_VERSION >= 10

  volatile bool quitting_ = false;

};

}  // namespace widevine
}  // namespace shared
}  // namespace starboard

// Widevine use boring ssl which shares most of it's symbol with Cobalt's openssl
// To solve this name conflics, widevine has to be loaded by dlopen with RTLD_LOCAL flag
// the symbols in widevine::Cdm and others will be unavailable because of RTLD_LOCAL,
// Here We will get these function pointers explicitly from a export funcion cobalt_widevine_cdm_init
struct CobaltWidevineSymbols {
  decltype(&::wvcdm::InitLogging) InitLogging;
  decltype(&::wvcdm::a2bs_hex) a2bs_hex;
  decltype(&::widevine::Cdm::version) version;
  decltype(&::widevine::Cdm::initialize) initialize;
  decltype(&::widevine::Cdm::create) create;
  decltype(&::OEMCrypto_CopyBuffer) CopyBuffer;
};
extern "C" int cobalt_widevine_cdm_init(struct CobaltWidevineSymbols * symbols);

#endif  // STARBOARD_SHARED_WIDEVINE_DRM_SYSTEM_WIDEVINE_H_

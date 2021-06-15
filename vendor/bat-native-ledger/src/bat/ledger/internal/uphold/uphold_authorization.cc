/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/uphold/uphold_authorization.h"

#include <utility>

#include "base/json/json_reader.h"
#include "base/strings/stringprintf.h"
#include "bat/ledger/global_constants.h"
#include "bat/ledger/internal/common/random_util.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "bat/ledger/internal/logging/event_log_keys.h"
#include "bat/ledger/internal/uphold/uphold_util.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace ledger {
namespace uphold {

UpholdAuthorization::UpholdAuthorization(LedgerImpl* ledger) :
    ledger_(ledger),
    uphold_server_(std::make_unique<endpoint::UpholdServer>(ledger)) {
}

UpholdAuthorization::~UpholdAuthorization() = default;

void UpholdAuthorization::Authorize(
    const base::flat_map<std::string, std::string>& args,
    ledger::ExternalWalletAuthorizationCallback callback) {
  auto wallet = GetWallet(ledger_);
  if (!wallet) {
    BLOG(0, "Wallet is null");
    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  // We're using only 4 states (NOT_CONNECTED, VERIFIED, DISCONNECTED_VERIFIED, PENDING)
  // and we don't want to authorize with Uphold in VERIFIED and PENDING.
  DCHECK(wallet->status == type::WalletStatus::NOT_CONNECTED ||
         wallet->status == type::WalletStatus::DISCONNECTED_VERIFIED);

  const auto current_one_time = wallet->one_time_string;

  // we need to generate new string as soon as authorization is triggered
  wallet->one_time_string = util::GenerateRandomHexString();
  const bool success = ledger_->uphold()->SetWallet(wallet->Clone());

  if (!success) {
    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  auto it = args.find("error_description");
  if (it != args.end()) {
    const std::string message = args.at("error_description");
    BLOG(1, message);
    if (message == "User does not meet minimum requirements") {
      callback(type::Result::NOT_FOUND, {});
      return;
    }

    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  if (args.empty()) {
    BLOG(0, "Arguments are empty");
    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  std::string code;
  it = args.find("code");
  if (it != args.end()) {
    code = args.at("code");
  }

  if (code.empty()) {
    BLOG(0, "Code is empty");
    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  std::string one_time_string;
  it = args.find("state");
  if (it != args.end()) {
    one_time_string = args.at("state");
  }

  if (one_time_string.empty()) {
    BLOG(0, "One time string is empty");
    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  if (current_one_time != one_time_string) {
    BLOG(0, "One time string miss match");
    callback(type::Result::LEDGER_ERROR, {});
    return;
  }

  uphold_server_->post_oauth()->Request(
      code,
      std::bind(&UpholdAuthorization::OnAuthorize, this, _1, _2, callback));
}

void UpholdAuthorization::OnAuthorize(
    const type::Result result,
    const std::string& token,
    ledger::ExternalWalletAuthorizationCallback callback) {
  if (result == type::Result::EXPIRED_TOKEN) {
    BLOG(0, "Expired token");
    callback(type::Result::EXPIRED_TOKEN, {});
    // status == type::WalletStatus::NOT_CONNECTED
    // Theoretically, calling DisconnectWallet() could result in
    // DISCONNECTED_VERIFIED, but only in case the status was VERIFIED (which we
    // know it wasn't - see above).
    return ledger_->uphold()->DisconnectWallet();
  }

  // status == type::WalletStatus::NOT_CONNECTED ||
  // status == type::WalletStatus::DISCONNECTED_VERIFIED

  if (result != type::Result::LEDGER_OK) {
    BLOG(0, "Couldn't get token");
    return callback(type::Result::LEDGER_ERROR, {});
  }

  if (token.empty()) {
    BLOG(0, "Token is empty");
    return callback(type::Result::LEDGER_ERROR, {});
  }

  auto uphold_wallet = GetWallet(ledger_);
  DCHECK(uphold_wallet);
  uphold_wallet->token = token;
  uphold_wallet->status = type::WalletStatus::PENDING;
  ledger_->uphold()->SetWallet(uphold_wallet->Clone());

  // After a login, we want to attempt to relink the user's payment ID to their
  // Uphold wallet address. Clear the flag that will cause relinking to be
  // skipped.
  ledger_->state()->SetAnonTransferChecked(false);

  callback(type::Result::LEDGER_OK, {});
}

}  // namespace uphold
}  // namespace ledger

/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_GEMINI_GEMINI_UTIL_H_
#define BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_GEMINI_GEMINI_UTIL_H_

#include <map>
#include <string>
#include <vector>

#include "bat/ledger/ledger.h"

namespace ledger {
class LedgerImpl;

namespace gemini {

const char kFeeAddressStaging[] = "60c0f891-5f98-4bee-a35d-9f834eaebfc0";
const char kFeeAddressProduction[] = "";
const char kACAddressStaging[] = "";
const char kACAddressProduction[] = "";

std::string GetClientId();

std::string GetClientSecret();

std::string GetUrl();

std::string GetFeeAddress();

std::string GetACAddress();

std::string GetAuthorizeUrl(const std::string& state);

std::string GetAddUrl();

std::string GetWithdrawUrl();

type::ExternalWalletPtr GetWallet(LedgerImpl* ledger);

bool SetWallet(LedgerImpl* ledger, type::ExternalWalletPtr wallet);

std::string GetAccountUrl();

type::ExternalWalletPtr GenerateLinks(type::ExternalWalletPtr wallet);

type::ExternalWalletPtr ResetWallet(type::ExternalWalletPtr wallet);

}  // namespace gemini
}  // namespace ledger

#endif  // BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_GEMINI_GEMINI_UTIL_H_

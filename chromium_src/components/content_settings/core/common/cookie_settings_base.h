/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_COMPONENTS_CONTENT_SETTINGS_CORE_COMMON_COOKIE_SETTINGS_BASE_H_
#define BRAVE_CHROMIUM_SRC_COMPONENTS_CONTENT_SETTINGS_CORE_COMMON_COOKIE_SETTINGS_BASE_H_

// TODO(mario): IsChromiumCookieAccessAllowed() will be needed while Chromium
// has IsCookieAccessAllowed() around to make sure callers do the right thing in
// the meantime (see https://crrev.com/c/2855034 for the CL removing that).

#define IsCookieSessionOnly                                                  \
  ShouldUseEphemeralStorage(                                                 \
      const GURL& url, const GURL& site_for_cookies,                         \
      const absl::optional<url::Origin>& top_frame_origin) const;            \
  bool IsEphemeralCookieAccessAllowed(const GURL& url,                       \
                                      const GURL& first_party_url) const;    \
  bool IsEphemeralCookieAccessAllowed(                                       \
      const GURL& url, const GURL& site_for_cookies,                         \
      const absl::optional<url::Origin>& top_frame_origin) const;            \
  bool IsChromiumFullCookieAccessAllowed(const GURL& url,                    \
                                         const GURL& first_party_url) const; \
  bool IsChromiumFullCookieAccessAllowed(                                    \
      const GURL& url, const GURL& site_for_cookies,                         \
      const absl::optional<url::Origin>& top_frame_origin) const;            \
  bool IsChromiumCookieAccessAllowed(const GURL& url,                        \
                                     const GURL& first_party_url) const;     \
  bool IsChromiumCookieAccessAllowed(                                        \
      const GURL& url, const GURL& site_for_cookies,                         \
      const absl::optional<url::Origin>& top_frame_origin) const;            \
  bool IsCookieSessionOnly

#include "../../../../../../components/content_settings/core/common/cookie_settings_base.h"

#undef IsCookieSessionOnly

#endif  // BRAVE_CHROMIUM_SRC_COMPONENTS_CONTENT_SETTINGS_CORE_COMMON_COOKIE_SETTINGS_BASE_H_

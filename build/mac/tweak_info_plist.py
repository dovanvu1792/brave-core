#!/usr/bin/env python3

# Copyright (c) 2021 The Brave Authors. All rights reserved.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.

#
# Xcode supports build variable substitutions and CPP; sadly, that doesn't work
# because:
#
# 1. Xcode wants to do the Info.plist work before it runs any build phases,
#    this means if we were to generate a .h file for INFOPLIST_PREFIX_HEADER
#    we'd have to put it in another target so it runs in time.
# 2. Xcode also doesn't check to see if the header being used as a prefix for
#    the Info.plist has changed.  So even if we updated it, it's only looking
#    at the modtime of the info.plist to see if that's changed.
#
# So, we work around all of this by making a script build phase that will run
# during the app build, and simply update the info.plist in place.  This way
# by the time the app target is done, the info.plist is correct.
#

import argparse
import plistlib
import sys

def _RemoveKeys(plist, *keys):
    """Removes a varargs of keys from the plist."""
    for key in keys:
        try:
            del plist[key]
        except KeyError:
            pass


def _OverrideVersionKey(plist, brave_version):
    """ `minor.build` version string is used for update.
    When we begin to use the Major version component, Brave version string will
    be `1.0.0` for example and `Minor.Build` (`0.0`) would be used for update
    check. Without modifying these numbers, update will fail as `0.0` is lower
    than `70.121` for example.

    To ensure minor version is higher than existing minor versions, we can
    multiply the major version by 100 and set it to `CFBundleVersion`."""
    version_values = brave_version.split('.')
    if int(version_values[0]) >= 1:
        adjusted_minor = int(version_values[1]) + (100 * int(version_values[0]))
        plist['CFBundleVersion'] = str(adjusted_minor) + '.' + version_values[2]


def Main():
    parser = argparse.ArgumentParser(usage='%(prog)s [options]')
    parser.add_argument('--plist', dest='plist_path', action='store',
        required=True, help='The path of the plist to tweak.')
    parser.add_argument('--output', dest='plist_output', action='store',
        default=None, help='If specified, the path to output ' + \
        'the tweaked plist, rather than overwriting the input.')
    parser.add_argument('--brave_channel', dest='brave_channel', action='store',
        default=None, help='Channel (beta, dev, nightly)')
    parser.add_argument('--brave_product_dir_name', dest='brave_product_dir_name',
        action='store', default=None,
        help='Product directory name')
    parser.add_argument('--brave_eddsa_key', dest='brave_eddsa_key', action='store',
        default=None, help='Public EdDSA key for update')
    parser.add_argument('--brave_version', dest='brave_version', action='store',
        default=None, help='brave version string')
    parser.add_argument('--format', choices=('binary1', 'xml1', 'json'),
        default='xml1', help='Format to use when writing property list '
            '(default: %(default)s)')
    parser.add_argument('--skip_signing', dest='skip_signing', action='store_true')
    args = parser.parse_args()

    # Read the plist into its parsed format.
    with open(args.plist_path, 'rb') as f:
        plist = plistlib.load(f)

    output_path = args.plist_path
    if args.plist_output is not None:
        output_path = args.plist_output

    if args.skip_signing:
        plist['KSChannelID'] = args.brave_channel
    elif 'KSChannelID' in plist:
        # 'KSChannelID' is set at _modify_plists() of modification.py.
        del plist['KSChannelID']

    plist['CrProductDirName'] = args.brave_product_dir_name

    if args.brave_eddsa_key:
        plist['SUPublicEDKey'] = args.brave_eddsa_key

    _OverrideVersionKey(plist, args.brave_version)

    # Explicitly disable profiling
    plist['SUEnableSystemProfiling'] = False

    # Explicitly change notifications from banner to alert
    plist['NSUserNotificationAlertStyle'] = 'alert'

    # Now that all keys have been mutated, rewrite the file.
    with open(output_path, 'wb') as f:
        plist_format = {'binary1': plistlib.FMT_BINARY, 'xml1': plistlib.FMT_XML} # pylint: disable=no-member
        plistlib.dump(plist, f, fmt=plist_format[args.format])


if __name__ == '__main__':
    sys.exit(Main())

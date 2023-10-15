#!/usr/bin/env bash

set -u
set -e

if [[ $# -eq 0 ]]; then
    echo "Usage: update-builtin-roots.sh webrtc-checkout-dir"
    exit 1
fi

THIS_DIR=$(cd -P "$(dirname "$(readlink "${BASH_SOURCE[0]}" || echo "${BASH_SOURCE[0]}")")" && pwd)
WEBRTC_DIR=$1

if [[ ! -d $THIS_DIR/.env ]]; then
    echo "Preparing Python virtualenv..."
    python3 -m venv $THIS_DIR/.env
fi

source $THIS_DIR/.env/bin/activate

pip install asn1crypto

pushd $WEBRTC_DIR

# Cleanup
rm -f ssl_roots.h

# Build the SSL roots file
python tools_webrtc/sslroots/generate_sslroots.py https://curl.se/ca/cacert.pem

# Move it
mv -f ssl_roots.h rtc_base/

# Save the changes
git add rtc_base/ssl_roots.h
git commit -m "Update SSL roots"

popd

deactivate

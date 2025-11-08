#!/usr/bin/env bash
# Headless i18n smoke test
# Usage: ci/i18n_smoke.sh <path-to-phoenix-app>

set -euo pipefail

APP_PATH="${1:?First argument must be path to Phoenix binary}"

QT_QPA_PLATFORM=offscreen "$APP_PATH" --test-i18n --lang=de | tee /tmp/i18n_de.log

grep -q 'QLocale(default)  : de_DE' /tmp/i18n_de.log
grep -q 'load qtbase ("de") => OK' /tmp/i18n_de.log
grep -q 'load phoenix ("de") => OK' /tmp/i18n_de.log

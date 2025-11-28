.PHONY: preflight daily
preflight:
	@echo "✅ Preflight OK (dev-01 removed, use GitHub Actions for CI)"

daily:
	@./.underlord/tasks/daily_startup.sh

SOAK_BUILD_DIR ?= build/soak
SOAK_BIN ?= $(SOAK_BUILD_DIR)/phoenix_app
SOAK_APP_LOG ?= /tmp/phoenix_soak_app.log
SOAK_MEMORY_CSV ?= /tmp/phoenix_soak_memory.csv
SOAK_PARSER ?= python3 scripts/soak/parse_memory.py
SOAK_CMAKE_PREFIX ?= $(CMAKE_PREFIX_PATH)

ifeq ($(OS),Windows_NT)
SOAK_RUNNER := pwsh -File scripts/soak/windows_soak.ps1
SOAK_BIN ?= $(SOAK_BUILD_DIR)/phoenix_app.exe
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
SOAK_RUNNER := zsh scripts/soak/macos_soak.zsh
SOAK_BIN ?= $(SOAK_BUILD_DIR)/Phoenix.app/Contents/MacOS/Phoenix
else
SOAK_RUNNER := bash scripts/soak/linux_soak.sh
endif
endif

.PHONY: soak-build soak-quick soak-1h soak-2h

soak-build:
	@cmake -S . -B $(SOAK_BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo $(if $(SOAK_CMAKE_PREFIX),-DCMAKE_PREFIX_PATH=$(SOAK_CMAKE_PREFIX),)
	@cmake --build $(SOAK_BUILD_DIR) --target phoenix_app

define run_soak
	@echo "[soak] running $(1)-minute soak using $(SOAK_RUNNER)"
	@SOAK_BIN="$(SOAK_BIN)" \
	 SOAK_APP_LOG="$(SOAK_APP_LOG)" \
	 SOAK_CSV="$(SOAK_MEMORY_CSV)" \
	 DURATION_MIN=$(1) \
	 $(SOAK_RUNNER)
	@$(SOAK_PARSER) "$(SOAK_MEMORY_CSV)" --app-log "$(SOAK_APP_LOG)"
endef

soak-quick:
	$(call run_soak,30)

soak-1h:
	$(call run_soak,60)

soak-2h:
	$(call run_soak,120)

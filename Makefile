.PHONY: build install test run side gui gui-frontend gui-bindings gui-run app install-app mac-app

build:
	go build -o bin/sysroad .

install:
	go install .

test:
	go test ./...

run: build
	./bin/sysroad

side: build
	./bin/sysroad side

# ---- desktop app (cmd/sysroad-gui) ----
#
# Needs bun, and on Linux WebKitGTK: webkitgtk-6.0 (Wails' default), or the
# older webkit2gtk-4.1 through the gtk3 tag, picked automatically.

GUI_DIR := cmd/sysroad-gui
GUI_TAGS := gui
ifeq ($(shell uname),Linux)
  ifneq ($(shell pkg-config --exists webkitgtk-6.0 && echo yes),yes)
    GUI_TAGS := gui,gtk3
  endif
endif

gui-frontend:
	cd $(GUI_DIR)/frontend && bun install --frozen-lockfile && bun run build

# Regenerate the TypeScript bindings after changing internal/gui's API.
gui-bindings:
	cd $(GUI_DIR) && wails3 generate bindings -f '-tags $(GUI_TAGS)' -ts -i -names

gui: gui-frontend
	go build -tags $(GUI_TAGS) -o bin/sysroad-gui ./$(GUI_DIR)

gui-run: gui
	./bin/sysroad-gui

# ---- installing the app (per user, no sudo) ----
#
# `make app` (from any checkout) sets up the app's own clone in SRC_DIR on this
# checkout's branch (or BRANCH=...), then builds and installs from it. The app
# updates itself later by pulling that clone and running `make install-app`.

SRC_DIR ?= $(HOME)/.local/share/sysroad-src
BIN_DIR ?= $(HOME)/.local/bin
BRANCH ?= $(shell git rev-parse --abbrev-ref HEAD)
ORIGIN := $(shell git remote get-url origin 2>/dev/null)

app:
	@if [ ! -d "$(SRC_DIR)/.git" ]; then git clone --branch "$(BRANCH)" "$(ORIGIN)" "$(SRC_DIR)"; \
	else git -C "$(SRC_DIR)" fetch --quiet origin && git -C "$(SRC_DIR)" switch -C "$(BRANCH)" "origin/$(BRANCH)"; fi
	$(MAKE) -C "$(SRC_DIR)" install-app

ifeq ($(shell uname),Darwin)
install-app: mac-app
	mkdir -p "$(BIN_DIR)"
	go build -o "$(BIN_DIR)/sysroad" .
else
install-app: gui
	mkdir -p "$(BIN_DIR)" "$(HOME)/.local/share/applications" \
		"$(HOME)/.local/share/icons/hicolor/scalable/apps" "$(HOME)/.local/share/icons/hicolor/512x512/apps"
	go build -o bin/sysroad .
	install -m755 bin/sysroad "$(BIN_DIR)/sysroad"
	install -m755 bin/sysroad-gui "$(BIN_DIR)/sysroad-gui"
	sed 's|^Exec=.*|Exec=$(BIN_DIR)/sysroad-gui|' $(GUI_DIR)/build/sysroad.desktop > "$(HOME)/.local/share/applications/sysroad.desktop"
	install -m644 $(GUI_DIR)/build/sysroad.svg "$(HOME)/.local/share/icons/hicolor/scalable/apps/sysroad.svg"
	install -m644 $(GUI_DIR)/build/sysroad-512.png "$(HOME)/.local/share/icons/hicolor/512x512/apps/sysroad.png"
	-update-desktop-database -q "$(HOME)/.local/share/applications" 2>/dev/null
	@echo "Installed sysroad and sysroad-gui in $(BIN_DIR)"
endif

# macOS: sysroad.app (ad-hoc signed for Apple Silicon) in ~/Applications, so it
# opens from Spotlight and can live in the Dock.
MAC_APP := bin/sysroad.app
MAC_VERSION := $(shell git rev-list --count HEAD 2>/dev/null || echo 0)

mac-app: gui
	rm -rf $(MAC_APP)
	mkdir -p $(MAC_APP)/Contents/MacOS $(MAC_APP)/Contents/Resources
	cp bin/sysroad-gui $(MAC_APP)/Contents/MacOS/sysroad-gui
	cp $(GUI_DIR)/build/sysroad.icns $(MAC_APP)/Contents/Resources/sysroad.icns
	sed 's/@VERSION@/0.$(MAC_VERSION)/' $(GUI_DIR)/build/Info.plist > $(MAC_APP)/Contents/Info.plist
	codesign --force --deep --sign - $(MAC_APP)
	mkdir -p $(HOME)/Applications
	rm -rf $(HOME)/Applications/sysroad.app
	cp -R $(MAC_APP) $(HOME)/Applications/sysroad.app
	@echo "Installed ~/Applications/sysroad.app"

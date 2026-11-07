.PHONY: build install test run side gui gui-frontend gui-bindings gui-run arch-pkg mac-app

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

# Arch: build a package with the launcher entry and icon; makepkg -si installs it.
arch-pkg:
	cd packaging/arch && makepkg -f

# macOS: build sysroad.app (unsigned, ad-hoc signed for Apple Silicon) into
# ~/Applications, so it opens from Spotlight and can live in the Dock.
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

.PHONY: build install test run side gui gui-frontend gui-bindings gui-run

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

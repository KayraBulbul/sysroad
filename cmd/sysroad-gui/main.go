//go:build gui

// sysroad-gui is the desktop app: the course dashboard, lessons with an
// embedded editor, and Run/Submit, on the same engine and data as the TUI.
// Build it with `make gui` (it needs the gui build tag and a built frontend).
package main

import (
	"embed"
	"fmt"
	"log"
	"os"
	"runtime"
	"strconv"

	"github.com/wailsapp/wails/v3/pkg/application"
	"github.com/wailsapp/wails/v3/pkg/events"

	"github.com/KayraBulbul/sysroad/internal/app"
	"github.com/KayraBulbul/sysroad/internal/gui"
)

//go:embed all:frontend/dist
var assets embed.FS

//go:embed build/appicon.png
var icon []byte

// evNav carries the mouse's back (-1) and forward (+1) side buttons on
// Linux, where the web view doesn't see them.
const evNav = "nav"

func init() {
	application.RegisterEvent[int](evNav)
	application.RegisterEvent[gui.Theme](gui.EvTheme)
	application.RegisterEvent[gui.FileEvent](gui.EvFile)
	application.RegisterEvent[gui.ResultEvent](gui.EvResult)
	application.RegisterEvent[gui.UpdateInfo](gui.EvUpdate)
}

func main() {
	gui.UseLoginPath()
	a, err := app.Open(app.DefaultDir())
	if err != nil {
		fmt.Fprintln(os.Stderr, "sysroad-gui:", err)
		os.Exit(1)
	}

	var wails *application.App
	var win *application.WebviewWindow
	svc := gui.New(a,
		func(name string, data any) { wails.Event.Emit(name, data) },
		func(url string) error { return wails.Browser.OpenURL(url) },
	)
	wails = application.New(application.Options{
		Name:        "sysroad",
		Description: "A boot.dev-style systems programming course in C++",
		Icon:        icon,
		// One window: launching it again (the keybind, the Dock) brings
		// the open one forward.
		SingleInstance: &application.SingleInstanceOptions{
			UniqueID: "dev.kayrabulbul.sysroad",
			OnSecondInstanceLaunch: func(application.SecondInstanceData) {
				if win != nil {
					application.InvokeSync(func() {
						win.Show()
						win.UnMinimise()
						win.Focus()
					})
				}
			},
		},
		Services:   []application.Service{application.NewService(svc)},
		Assets:     application.AssetOptions{Handler: application.AssetFileServerFS(assets)},
		Linux:      application.LinuxOptions{ProgramName: "sysroad"},
		Mac:        application.MacOptions{ApplicationShouldTerminateAfterLastWindowClosed: true},
		OnShutdown: func() { gui.Stop(svc) },
	})
	gui.OnQuit(svc, func() { wails.Quit() })
	wails.Event.OnApplicationEvent(events.Common.ApplicationStarted, func(*application.ApplicationEvent) {
		gui.Start(svc)
	})

	bg := svc.Theme().Colors["background"]
	if runtime.GOOS == "darwin" {
		// macOS wires Cmd+C/V/X/A/Z and Cmd+Q through the menu bar, so the
		// web view needs the standard menus to have them at all.
		menu := application.NewMenu()
		menu.AddRole(application.AppMenu)
		menu.AddRole(application.EditMenu)
		menu.AddRole(application.WindowMenu)
		wails.Menu.Set(menu)
	}

	win = wails.Window.NewWithOptions(application.WebviewWindowOptions{
		Title:            "sysroad",
		Width:            1440,
		Height:           900,
		MinWidth:         720,
		MinHeight:        480,
		BackgroundColour: rgb(bg),
		URL:              "/",
		Linux:            application.LinuxWindow{Icon: icon},
		Mac: application.MacWindow{
			InvisibleTitleBarHeight: 38,
			TitleBar:                application.MacTitleBarHiddenInset,
		},
	})

	mouseNav = func(dir int) { wails.Event.Emit(evNav, dir) }
	win.OnWindowEvent(events.Common.WindowRuntimeReady, func(*application.WindowEvent) {
		application.InvokeSync(func() {
			if tuneWebView(win.NativeWindow()) == 0 {
				log.Println("sysroad-gui: couldn't find the web view to set up scrolling and mouse buttons")
			}
		})
	})

	if err := wails.Run(); err != nil {
		log.Fatal(err)
	}
}

// rgb turns "#rrggbb" into the window's background colour, so the window
// doesn't flash white before the page paints.
func rgb(hex string) application.RGBA {
	if len(hex) < 7 {
		return application.NewRGB(25, 25, 29)
	}
	v, err := strconv.ParseUint(hex[1:7], 16, 32)
	if err != nil {
		return application.NewRGB(25, 25, 29)
	}
	return application.NewRGB(uint8(v>>16), uint8(v>>8), uint8(v))
}

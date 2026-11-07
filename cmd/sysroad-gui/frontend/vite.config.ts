import { defineConfig } from "vite";
import { svelte } from "@sveltejs/vite-plugin-svelte";
import wails from "@wailsio/runtime/plugins/vite";

export default defineConfig({
  server: {
    host: "127.0.0.1",
    port: Number(process.env.WAILS_VITE_PORT) || 9245,
    strictPort: true,
  },
  build: {
    target: "es2022",
    // One bundle: everything is local, so splitting only adds round trips.
    chunkSizeWarningLimit: 2000,
    assetsInlineLimit: 0,
  },
  plugins: [svelte(), wails("./bindings")],
});

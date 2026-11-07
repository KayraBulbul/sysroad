# Vigil on the MacBook

sysroad's TUI only uses the terminal's 16 palette colours, so on Omarchy it
follows whatever theme is active. The Mac has no theme system, so set
Ghostty and Neovim to Vigil there and sysroad will match.

```sh
# from a checkout of this repo, on the Mac
mkdir -p ~/.config/ghostty/themes ~/.config/nvim/lua/plugins
cp extras/mac-vigil/vigil.ghostty ~/.config/ghostty/themes/vigil
echo 'theme = vigil' >> ~/.config/ghostty/config
cp extras/mac-vigil/vigil.lua ~/.config/nvim/lua/plugins/vigil.lua   # LazyVim
```

Restart Ghostty (or reload its config with `cmd+shift+,`) and Neovim.

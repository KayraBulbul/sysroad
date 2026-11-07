-- Vigil for LazyVim (copied from ~/.config/omarchy/themes/vigil/neovim.lua).
-- Copy to ~/.config/nvim/lua/plugins/vigil.lua on the Mac.
-- Keep Miasma syntax colors, with warm neutrals replacing olive and green.
return {
  {
    "bjarneo/aether.nvim",
    branch = "v3",
    name = "aether",
    priority = 1000,
    opts = {
      colors = {
        bg = "#19191D",
        dark_bg = "#131316",
        darker_bg = "#0E0E11",
        lighter_bg = "#26252B",

        fg = "#c2c2b0",
        dark_fg = "#555555",
        light_fg = "#8a8a7e",
        bright_fg = "#c2c2b0",
        muted = "#666666",

        red = "#685742",
        yellow = "#b36d43",
        orange = "#8d6242",
        green = "#B87333",
        cyan = "#c9a554",
        blue = "#A79D98",
        magenta = "#bb7744",
        brown = "#463121",

        bright_red = "#685742",
        bright_yellow = "#b36d43",
        bright_green = "#B87333",
        bright_cyan = "#c9a554",
        bright_blue = "#A79D98",
        bright_magenta = "#bb7744",

        accent = "#C65D2E",
        cursor = "#c2c2b0",
        foreground = "#c2c2b0",
        background = "#19191D",
        selection = "#48342F",
        selection_foreground = "#c2c2b0",
        selection_background = "#48342F",
      },
    },
  },
  {
    "LazyVim/LazyVim",
    opts = {
      colorscheme = "aether",
    },
  },
}

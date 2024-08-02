# fancy-border

<div align = center>

<img src="screenshot.png">

<br>

<div align = left>

Replaces rounded corners with chamfered corners. Currently, it functions pretty much the same as borders++, but it also loads in a different border shader. The rendering is done by incorporating a copy of the renderBorder() function from the Hyprland source (src/Hyprland/src/render/OpenGl.cpp) so this plugin has a tendency to break (often) whenever there's a Hyprland or hyprland-plugin update.

currently this works with:
hyprland-git (0.41.2.r154.ab0a3268)
hyprland-plugins-git (r189.4c2cef8)


## Building:

Ensure hyprland and hyprland-plugins is installed:
Use `make` or meson/ninja (preferred for smaller lib size) to build fancy-border.so.
The compile output needs to exist in /usr/lib/hyprland-plugins/ to satisfy dependencies.


## Example Config:
```
exec-once = hyprctl plugin load /usr/lib/hyprland-plugins/fancy-border.so

# remove the default border by setting to 0
# general { border_size = 0 }

# border radius is conrolled by decoration config
# decoration { rounding = 6 }

plugin {
    fancy-border {
        add_borders = 2 # 0 - 9

        # you can add up to 9 borders
        col.border_1 = rgb(888888ee)
        col.border_2 = rgb(e0def4ee)

        border_size_1 = 2
        border_size_2 = 3

        # makes outer edges match rounding of the parent. Turn on / off to better understand. Default = on.
        natural_rounding = yes
    }
}
```

# note

A description of this project.

a note tool build with meson and gnome. etc..

## ToDo List

1. Markup
2. Sync
3. Privillege
4. Theme
5. Multiple file save

经过测试：
meson有能力做好程序的编译和安装两件事情，flatpak滚一边去。
meson setup _build
meson compile -C _build
meson dist -C _build
sudo meson install -C _build
搞定装好后，设置schema也会安装到系统，所以之后就可以进行debug了
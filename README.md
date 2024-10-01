# hyprsquint

A Hyprland plugin for magnifying your screen.

## Build

```bash
# Make build script executable
chmod +x ./build.sh

./build.sh build
```

## Clean

```bash
./build clean
```

## Configuration

The following binds enable cursor zoom with the mouse wheel when the `Control`
button is held, and disables zoom when it is released.

Zoom will reset once `Control` is released.

```
bindti = , code:37, hyprsquint:squint, enable
bindtri = , code:37, hyprsquint:squint, disable
```

___

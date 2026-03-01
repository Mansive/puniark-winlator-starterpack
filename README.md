# PuniArk Winlator Starter Pack

This repo contains resources and instructions on making Winlator and Winlator-based apps ([GameNative](https://github.com/utkarshdalal/GameNative), [GameHub](https://gamehub.xiaoji.com/), [GameHub Lite](https://github.com/Producdevity/gamehub-lite)) compatible with PuniArk. It's not convenient, but at least you only need to perform simple copy-pastes once per game.

## Guide

1. Download the [starter pack zip archive](https://github.com/Mansive/puniark-winlator-starterpack/releases/latest) and extract its contents into your Android device's `Downloads` folder.
2. Open the emulator, select a game, then enter its container
   - GameNative: Select the game, press the top-right icon with the three dots, then `Open container`.
   - On GameHub (Lite): Select the game, press the icon with the three dots, press `PC Game Settings`, then `Enter Container Desktop`. This option may be unavailable if you imported your game instead of downloading through Steam.
3. In the container, navigate to the `D:` drive and open the folder containing the starter pack contents.
4. Run one of the `RUN` scripts.
   - Use `RUN_AUTO.bat` if your game is located in drives `A:` or `X:`. This will always be the case if you're using GameNative. It'll work for GameHub if the game is from your Steam account.
   - Use `RUN_PICKER.bat` to select your game folder. This should be used if the game is stored somewhere other than `A:` or `X:`.
5. While in the container, launch the game and see if PuniArk can connect to it.
6. If PuniArk connects, you can exit the container and launch the game normally.

> [!TIP]
> If you're using GameNative, you can make easier to navigate to the starter pack folder with the following:
> 1. Press your profile picture in the top-right corner
> 2. Navigate `Settings` -> `Modify Default Config` -> `Drives`
> 3. Press the `+` icon at the bottom
> 4. Select any letter that isn't `A`, for example, `F`
> 5. Select the starter pack folder
>
> This will make the starter pack folder appear as drive `F:` in the side menu in the container, reducing the amount of clicks needed to navigate to the folder.

## How does this work?

There needs to be a way for PuniArk to control another app's memory. However, Android has security measures in place to prevent this sort of situation where apps can arbitrarily control other apps. We can bypass them by injecting [`frida-gadget`](https://frida.re/docs/gadget/) into the app, such that whenever the app launches, it also launches a Frida server. PuniArk can then connect to this special server and extract text from the app.

The standard way of injecting `frida-gadget` on Android involves using a tool like [lief](https://lief.re) to modify the app, tricking it into loading the gadget library. Unfortunately, it only works when the library is of the same architecture as the device—an ARM64 Android device—and the ARM64 version of the gadget doesn't work on x86_64 Windows games. Instead, we need to figure out how to get the Windows game to load a x86_64 library. This is where DLL (dynamic-link library) proxying comes in.

Games on Windows need to import libraries or DLLs to work. They first look for the `.dll` file they need in their own folder, and if it isn't there, they'll look in a folder like `C:\Windows\System32`. What we can do is place a `.dll` from the [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) project into the game's folder which does three things:
1. Load `frida-gadget.dll`.
2. Load the real DLL from `System32` if 64-bit and `SysWOW64` if 32-bit.
3. Proxy function calls from the game to the real DLL.

This way, the game can load our proxy DLL and work just fine, while PuniArk can connect to the Frida server and control the game. But just because we place a `punipuni.dll` into the folder doesn't mean the game will load it. How do we know which DLL files the game will import?

Many games commonly import the same set of libraries; for example, the DirectX Direct3D libraries. Direct3D is standard in the gaming industry and allows developers to do fancy 2D and 3D graphics stuff. It also allows us to proxy the `d3d9.dll`, `d3d11.dll`, and `d3d12.dll` files that almost every game imports. The `RUN` scripts in the starter pack first detects if the game is 32-bit or 64-bit, then copies over all three Direct3D DLL files of the correct bitness into the game's folder to maximize import chances. 

It is possible to detect the exact Direct3D DLL file the game needs and copy over just that specific one, but that's too much work for a problem that can be solved by blindly throwing in all three files. Also possible is proxying a library that every single *software* uses such as `version.dll`, but things get weird if the game's launcher opens before the actual game, or two processes launch, or... We can avoid all of those quirks by proxying libraries that only the actual game will use.

Of course, this strategy doesn't work if the game doesn't use Direct3D. They might be using OpenGL or pure software rendering for their graphics. To handle these edge cases, the starter pack still copies over `version.dll`. Because of how Wine works, this catch-all DLL doesn't get imported even if the game needs it, because the game is forced to use Wine's own `version.dll`. You need to add the environment variable `WINEDLLOVERRIDES` with the value `version=n,b`, which tells Wine to prioritize the **n**ative DLL over Wine's **b**uilt-in one. This can be done in the available Windows emulators:
- GameNative 
  1. Select a game, press the top-right three-dots icon, and `Edit container`. Or, on the main menu, press your profile picture icon and then `Modify Default Config`.
  2. Select the `Environment` tab.
  3. Press the `+` icon near the bottom.
  4. Set `Name` to `WINEDLLOVERRIDES` and `Value` to `version=n,b`.
- GameHub (Lite)
  1. Select a game, press the three-dots icon, then press `PC Game Settings`.
  2. Scroll down in the `General` menu until you find `Environment Variables`.
  3. In the `Environment Variables` window, enter in the left box `WINEDLLOVERRIDES` and the right box `version=n,b`.
  4. If this doesn't work then I'm not surprised cause I didn't test GameHub at all.

I've tried making the DLL setup as simple as possible, but there's still usability improvements that can be made.

## Building

See the [build guide](docs/build.md).

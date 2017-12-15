# G-Keyboard API
This small library allows you to manage your G-Keyboard's leds and keypresses.
- Read up to 26 keys pressed at the same time
- Set led intensity for each key

# Compatibility
Only the G610 Keyboard is currently supported...

# Building
This project provides a library (.so) as well as a runnable example.

```bash
make lib
make bin
```

or

```bash
make
```

The runnable (`bin/gkbd`) lights up each key you press. The led then fades away quickly. Every time you press a key, the G logo lights up

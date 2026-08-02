# OpenAE Vamp plugin

[Vamp](https://vamp-plugins.org) audio analysis plugin exposing the [OpenAE](https://openae.io) acoustic emission features.
The plugin can be used in Vamp hosts such as [Sonic Visualiser](https://sonicvisualiser.org), [Audacity](https://www.audacityteam.org) and [Sonic Annotator](https://vamp-plugins.org/sonic-annotator/).

## Installation

Copy the plugin binary into one of the Vamp plugin directories and restart your Vamp host:

**Linux** (`openae.so`)

- `$HOME/vamp`
- `/usr/local/lib/vamp`
- `/usr/lib/vamp`

**macOS** (`openae.dylib`)

- `$HOME/Library/Audio/Plug-Ins/Vamp`
- `/Library/Audio/Plug-Ins/Vamp`

The binary is not notarized by Apple. macOS quarantines downloaded files and will refuse to load the plugin ("cannot be opened because the developer cannot be verified"). Remove the quarantine attribute after extracting:

```sh
xattr -d com.apple.quarantine openae.dylib
```

**Windows** (`openae.dll`)

- `C:\Program Files\Vamp Plugins`

Alternatively, set the `VAMP_PATH` environment variable to a custom plugin directory.

## Links

- Documentation: https://openae.io
- Source code: https://github.com/openae-io/openae-lib
- Issues: https://github.com/openae-io/openae-lib/issues

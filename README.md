# Buoh
Buoh aims to help the comic lovers by providing an easy way of browsing and reading their favourites online comic strips.

* Web: http://buoh.steve-o.org/
* Issue tracker: https://gitlab.gnome.org/GNOME/buoh/issues
* Merge requests: https://gitlab.gnome.org/GNOME/buoh/merge_requests

## Building
For building Buoh, you will need `meson`, `ninja` and `pkgconfig`, as well as the following libraries with their development headers.

* `glib2`
* `gtk3`
* `libxml2`
* `libsoup`

Then you can build and install Buoh like [any other meson project](http://mesonbuild.com/Quick-guide.html#compiling-a-meson-project):

```
meson build --prefix=/usr
ninja -C build
ninja -C build install
```

Alternately, with [Nix package manager](https://nixos.org/nix/), you can just run `nix-shell` in the project directory, and it will drop you into a shell with all the required dependencies.

## License
Buoh is distributed under the terms of the GNU General Public License version 2, or (at your option) any later version. You can find the whole text of the license in the [COPYING](COPYING) file.

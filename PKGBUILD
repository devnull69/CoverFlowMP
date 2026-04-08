pkgname=coverflowmp
pkgver=2.0.4
pkgrel=1
pkgdesc="CoverFlow-based media center with configurator"
arch=('x86_64')
url="https://example.invalid/coverflowmp"
license=('GPL-3.0-or-later')

depends=(
  'ffmpeg'
  'ffmpegthumbnailer'
  'mpv'
  'qt6-5compat'
  'qt6-base'
  'qt6-declarative'
)

makedepends=(
  'cmake'
  'pkgconf'
  'qt6-tools'
)

options=('!emptydirs')

build() {
  local builddir="${srcdir}/build"

  cmake -S "${startdir}" -B "${builddir}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_INSTALL_LIBDIR=lib

  cmake --build "${builddir}"
}

package() {
  local builddir="${srcdir}/build"

  DESTDIR="${pkgdir}" cmake --install "${builddir}"
}


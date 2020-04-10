# Memcached for Windows

[memcached](https://memcached.org/) is a high performance multithreaded event-based key/value cache store intended to be used in a distributed system.

**memcached-windows** is a native Windows port of memcached without using a compatibility layer like Cygwin or Windows Subsystem for Linux. It is instead using [Mingw-w64](http://mingw-w64.org/) to produce native Win32 and Win64 binaries. See [Why native?](https://github.com/jefyt/memcached-windows/wiki/Why-native%3F) wiki for the advantages. Build script is available to build reproducible binaries similar to [cURL](https://curl.haxx.se/windows/) (see [curl-for-win](https://github.com/curl/curl-for-win/)). This means that rebuild can always produce same binaries using same compiler package version. This script is used in CI build.

**memcached-windows** is verified using the same test suite as the official [memcached](https://memcached.org/). All tests **PASSED**!

**memcached-windows** will be regularly merged, built, and tested with upstream/official [memcached](https://memcached.org/)'s latest releases. See [wiki](https://github.com/jefyt/memcached-windows/wiki) for more info.

## Bintray binary package downloads (win32 and win64)

<table>
    <tr>
        <th>Package</th>
        <th>Link</th>
    </tr>
    <tr>
        <td>memcached</td>
        <td><a href='https://bintray.com/jefty/generic/memcached-windows/_latestVersion'><img src='https://api.bintray.com/packages/jefty/generic/memcached-windows/images/download.svg'></a>
        </td>
    </tr>
    <tr>
        <td>libevent (for build only)</td>
        <td><a href='https://bintray.com/jefty/generic/libevent-windows/_latestVersion'><img src='https://api.bintray.com/packages/jefty/generic/libevent-windows/images/download.svg'></a></td>
    </tr>
    <tr>
        <td>boringssl (for build only)</td>
        <td><a href='https://bintray.com/jefty/generic/boringssl-windows/_latestVersion'><img src='https://api.bintray.com/packages/jefty/generic/boringssl-windows/images/download.svg'></a></td>
    </tr>
</table>

## CI build and test logs

* https://ci.appveyor.com/project/jefty/memcached-windows (Source/Uploader of [bintray](https://bintray.com/jefty/generic/memcached-windows/_latestVersion) binaries)
* CI outputs and saves the final archives' hashes and can be compared with [bintray](https://bintray.com/jefty/generic/memcached-windows/_latestVersion)'s published hashes. This is one way to confirm that the release came from the CI build.
* Aside from the hashes, [bintray](https://bintray.com/jefty/generic/memcached-windows/_latestVersion) binaries are also GPG-signed. Verify with [public key](https://bintray.com/user/downloadSubjectPublicKey?username=jefty).

## Environment

At least **Windows 7**

## Running

* Just execute __*memcached.exe*__ (Options are same except the unsupported)
* Just execute __*memcached.exe --help*__ for more info

## Unsupported/Disabled options/features (may support in the future)

* **sasl** (-Y/--auth-file with TLS/SSL enabled is a good alternative)
* **-u/user** (Better use Windows __*runas*__ command, Windows **explorer**'s __*Run as different user*__ context menu, or other Windows built-in tools)
* **-s/unix-socket** ([Mingw-w64](http://mingw-w64.org/) does not currently support **AF_UNIX** even though it is already supported in Windows since **Windows 10 build 1803**. Supporting this natively in Windows 10 **MUST** also need native (not **Cygwin**-emulated) **AF_UNIX**-aware memcached clients. Just use localhost TCP instead!
* **-r/coredumps** ([Mingw-w64](http://mingw-w64.org/) currently doesn't support coredump but check [Runtime and Crash Analysis](https://github.com/jefyt/memcached-windows/wiki/Runtime-and-Crash-Analysis) wiki to know how to achieve same purpose using native Windows crash dumps.)
* **-k/lock-memory** (Windows does not currently support locking of all paged memory)
* **seccomp**

## Bug reports

Feel free to use the issue tracker on github.

**If you are reporting a security bug** please contact a maintainer privately.
We follow responsible disclosure: we handle reports privately, prepare a
patch, allow notifications to vendor lists, then run a fix release and your
bug can be posted publicly with credit in our release notes and commit
history.

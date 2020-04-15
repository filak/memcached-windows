# Memcached for Windows

[memcached](https://memcached.org/) is a high performance multithreaded event-based key/value cache store intended to be used in a distributed system.

**memcached-windows** is a native Windows port of memcached without using a compatibility layer like Cygwin or Windows Subsystem for Linux. It is instead using [Mingw-w64](http://mingw-w64.org/) to produce native Win32 and Win64 binaries. See [Why native?](https://github.com/jefyt/memcached-windows/wiki/Why-native%3F) wiki for the advantages. Build script is available to build reproducible binaries similar to [cURL](https://curl.haxx.se/windows/) (see [curl-for-win](https://github.com/curl/curl-for-win/)). This means that rebuild can always produce same binaries using same compiler package version. This script is used in CI build.

**memcached-windows** is verified using the same test suite as the official [memcached](https://memcached.org/). All tests **PASSED**!

**memcached-windows** will be regularly merged, built, and tested with upstream/official [memcached](https://memcached.org/)'s latest releases. See [wiki](https://github.com/jefyt/memcached-windows/wiki) for more info.

## Binary package downloads (win32 and win64)
<table>
    <tr>
        <td>Bintray</td>
        <td><a href='https://bintray.com/jefty/generic/memcached-windows/_latestVersion'><img src='https://api.bintray.com/packages/jefty/generic/memcached-windows/images/download.svg'></a>
        </td>
    </tr>
    <tr>
        <td>AppVeyor CI</td>
        <td><a href='https://ci.appveyor.com/project/jefty/memcached-windows/build/artifacts'>Artifacts</a>
        </td>
    </tr>
</table>

* CI outputs and saves the final archives' hashes and can be compared with [bintray](https://bintray.com/jefty/generic/memcached-windows/_latestVersion)'s published hashes. This is one way to confirm that the release came from the CI build.
* Aside from the hashes, [bintray](https://bintray.com/jefty/generic/memcached-windows/_latestVersion) binaries are also GPG-signed. Verify with [public key](https://bintray.com/user/downloadSubjectPublicKey?username=jefty).

## Environment

Minimum Requirement: **Windows Vista/Windows Server 2008**
* **NOTE**: **-s/unix-socket** requires at least [Windows 10 version 1803](https://docs.microsoft.com/en-us/windows/whats-new/whats-new-windows-10-version-1803) to work. Run _**sc query afunix**_ to check if OS is supported. Socket creation error will occur for unsupported OS.

## Running

* Just execute __*memcached.exe*__ (Options are same except the unsupported)
* Just execute __*memcached.exe --help*__ for more info

## Unsupported/Disabled options/features (may support in the future)

* **sasl** (-Y/--auth-file with TLS/SSL enabled is a good alternative)
* **-u/user** (Better use Windows __*runas*__ command, Windows **explorer**'s __*Run as different user*__ context menu, or other Windows built-in tools)
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

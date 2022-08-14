# Memcached for Windows

[memcached](https://memcached.org/) is a high performance multithreaded event-based key/value cache store intended to be used in a distributed system.

**memcached-windows** is a native Windows port of memcached without using a compatibility layer like Cygwin or Windows Subsystem for Linux. It is instead using [Mingw-w64](http://mingw-w64.org/) to produce native Windows binaries. See [Why native?](https://github.com/jefyt/memcached-windows/wiki/Why-native%3F) wiki for the advantages. Released binaries are transparently built, tested, and reproducible (see https://ci.appveyor.com/project/jefty/memcached-windows).

**memcached-windows** is verified using the same test suite as the official [memcached](https://memcached.org/). All tests **PASSED**!

**memcached-windows** will be regularly merged, built, and tested with upstream/official [memcached](https://memcached.org/)'s latest releases. See [wiki](https://github.com/jefyt/memcached-windows/wiki) for more info.

## Binary package downloads (win32 and win64)
<table>
    <tr>
        <td>GitHub (AppVeyor CI)</td>
        <td><a href='https://github.com/jefyt/memcached-windows/releases/latest'>Latest</a>
        </td>
    </tr>
    <tr>
        <td>AppVeyor CI</td>
        <td><a href='https://ci.appveyor.com/project/jefty/memcached-windows/build/artifacts'>Artifacts</a>
        </td>
    </tr>
</table>

* CI outputs and saves the final archives' hashes and can be compared with the released hashes. This is one way to confirm binaries' origin.
* Aside from the hashes, [Bintray](https://bintray.com/jefty/generic/memcached-windows/_latestVersion) binaries are also GPG-signed. Verify with [public key](https://bintray.com/user/downloadSubjectPublicKey?username=jefty).

## Environment

Minimum Requirement: **Windows Vista/Windows Server 2008**
* **NOTE**: **-s/unix-socket** requires at least [Windows 10 version 1803](https://en.wikipedia.org/wiki/Windows_10_version_history#Version_1803_(April_2018_Update))/[Windows Server 2016 version 1803](https://en.wikipedia.org/wiki/Windows_Server_2016#Version_1803) to work. Run _**sc query afunix**_ to check if OS is supported. Socket creation error will occur for unsupported OS.

## Running

* Just execute __*memcached.exe*__ (Options are same except the unsupported)
* Just execute __*memcached.exe --help*__ for more info

## Unsupported (may support in the future)

* **sasl** (-Y/--auth-file with TLS/SSL enabled is a good alternative)
* **-u/user** (Better use Windows __*runas*__ command, Windows **explorer**'s __*Run as different user*__ context menu, or other Windows built-in tools)
* **-r/coredumps** ([Mingw-w64](http://mingw-w64.org/) currently doesn't support coredump but check [Runtime and Crash Analysis](https://github.com/jefyt/memcached-windows/wiki/Runtime-and-Crash-Analysis) wiki to know how to achieve same purpose using native Windows crash dumps.)
* **-k/lock-memory** (Windows does not currently support locking of all paged memory)
* **seccomp**

## Bug reports

Feel free to use the issue tracker on github.

**If you are reporting a security bug** please contact a maintainer privately.
We follow responsible disclosure: we handle reports privately, prepare a
patch, allow notifications to vendor lists. Then we push a fix release and your
bug can be posted publicly with credit in our release notes and commit
history.

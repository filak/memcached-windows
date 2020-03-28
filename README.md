# Memcached for Windows

[memcached](https://memcached.org/) is a high performance multithreaded event-based key/value cache store intended to be used in a distributed system.

**memcached-windows** is a native Windows port of memcached without using a compatibility layer like Cygwin or Windows Subsystem for Linux. It is instead using [Mingw-w64](http://mingw-w64.org/) to produce native Win32 and Win64 binaries. Build script is also available to build reproducible binaries similar to [cURL](https://curl.haxx.se/windows/) (see [curl-for-win](https://github.com/curl/curl-for-win/)). This means that rebuild can always produce same binaries using same compiler package version. This script is used in CI build.

Since the ideal target of this project is to be merged upstream, the codes are carefully written not to cause major merge conflicts with the base code. Header inclusions are not modified but instead redirected to local headers with the same name (e.g. **signal.h**). The local header files then include the real files if necessary using **#include_next**. This means that the codes to be built for non-Windows target are virtually same as the original because all new/modified codes are protected by macros (e.g. **DISABLE_UNIX_SOCKET**).

Main logic is same as the official except for the necessary platform-specific changes mostly implemented in separate files (see **mingw** folder).

## Dependencies

* [libevent](https://libevent.org/) (same as the base code)

## Unsupported/Disabled options/features (may support in the future)

* **sasl** (Upstream support since v1.4.3)
* **extstore** (Upstream support since v1.5.4)
* **-u/user** (Can use Windows __*runas*__ command or Windows **explorer**'s __*Run as different user*__ context menu)
* **-s/unix-socket** (Windows does not currently support Unix domain socket) 
* **-k/lock-memory** (Windows does not currently support locking of all paged memory)
* **-r/coredumps** (MinGW currently doesn't support but gdb debugging without coredump is possible)
* **seccomp** (Linux-specific)

## Building on Windows Host (IDE-based, recommended for easier debugging)

* Download and install [Mingw-w64](http://mingw-w64.org/) compiler (see https://sourceforge.net/projects/mingw-w64/files/)
* Download **config.h** from https://bintray.com/jefty/generic/memcached-windows/_latestVersion (e.g. 	
*memcached-x.x.x-win64-mingw.zip/include/config.h*) and save in memcached root directory.
* Build [libevent](https://libevent.org/)'s static library or download pre-built from https://bintray.com/jefty/generic/libevent-windows/_latestVersion
* Build [boringssl](https://boringssl.googlesource.com/boringssl/+/refs/heads/chromium-stable)'s static library or download pre-built from https://bintray.com/jefty/generic/boringssl-windows/_latestVersion
* Download, install, and run [Code::Blocks IDE](http://www.codeblocks.org/downloads)
* Set the installed MinGW as the default compiler of Code::Blocks IDE
* Set the default compiler's **LIBEVENTDIR** custom variables to the location of libevent (dir with include and lib subdirs).
* Set the default compiler's **SSLDIR** custom variables to the location of libevent (dir with include and lib subdirs).
* Open **mingw/memcached.workspace** workspace from **Code::Blocks IDE**
* Build and/or debug using the IDE
* Artifacts: **mingw/bin/Debug**, **mingw/bin/Release**

## Building on Linux Host (configure)

* Install **gcc-mingw-w64 autoconf automake libtool**
* Cross-build [libevent](https://libevent.org/)'s static library or download pre-built from https://bintray.com/jefty/generic/libevent-windows/_latestVersion
* Cross-build [boringssl](https://boringssl.googlesource.com/boringssl/+/refs/heads/chromium-stable)'s static library or download pre-built from https://bintray.com/jefty/generic/boringssl-windows/_latestVersion
* *./autogen.sh*
* *ac_cv_libevent_dir=/path/to/libevent ac_cv_libssl_dir=/path/to/boringssl ./configure --host=(x86_64/i686)-w64-mingw32 --enable-tls --disable-extstore --disable-seccomp --disable-sasl  --disable-sasl-pwdb --disable-coverage --disable-docs*
* *make*
* *make test*

## Building on Linux Host (Reproducible binaries with autotest using Wine)

**NOTE:** This takes time since [libevent](https://libevent.org/) and [boringssl](https://boringssl.googlesource.com/boringssl/+/refs/heads/chromium-stable) will be downloaded and built from source.
* Install **gcc-mingw-w64 autoconf automake cmake libtool curl git gpg python3-pip make libssl-dev zip zstd time jq dos2unix wine64 wine32 perl nasm golang**
* cd **mingw/build/**
* *./_build.sh* or **CRUSHER_TEST=ON** *./_build.sh*
* Setting **CRUSHER_TEST** env automatically runs [mc-crusher](https://github.com/memcached/mc-crusher) test. By default this is limited only to **30 seconds per [mc-crusher](https://github.com/memcached/mc-crusher)/conf/*** with a total duration of around **10 minutes** per build. Set **CRUSHER_DURATION** env to customize.

**Artifacts**:
<table>
    <tr>
        <th>File</th>
        <th>Description</th>
    </tr>
    <tr>
        <td>memcached-VERSION-win32-mingw.tar.xz</td>
        <td>Win32 tar.xz format</td>
    </tr>
    <tr>
        <td>memcached-VERSION-win32-mingw.zip</td>
        <td>Win32 zip format</td>
    </tr>
    <tr>
        <td>memcached-VERSION-win64-mingw.tar.xz</td>
        <td>Win64 tar.xz format</td>
    </tr>
    <tr>
        <td>memcached-VERSION-win64-mingw.zip</td>
        <td>Win64 zip format</td>
    </tr>
    <tr>
        <td>hashes.txt</td>
        <td>SHA256 and SHA512 of the above files</td>
    </tr>
    <tr>
        <td>all-mingw-VERSION.zip</td>
        <td>All of the above files</td>
    </tr>
</table>

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

* https://ci.appveyor.com/project/jefty/memcached-windows (Source of [bintray](https://bintray.com/jefty/generic/memcached-windows/_latestVersion)
* CI logs and saves the final archives' hashes and can be compared with [bintray](https://bintray.com/jefty/generic/memcached-windows/_latestVersion)'s published hashes. This is one way to confirm that the release came from the CI build.
* Aside from the hashes, [bintray](https://bintray.com/jefty/generic/memcached-windows/_latestVersion) binaries are also GPG-signed. Verify with [public key](https://bintray.com/user/downloadSubjectPublicKey?username=jefty).

## Environment

At least **Windows 7**

## Running

* Just execute __*memcached.exe*__ (Options are same except the unsupported)
* Just execute __*memcached.exe --help*__ for more info

## Testing

* memcached's **testapp** is also ported to easily verify that the port is working as expected
* __*make test*__ will only execute __*testapp.exe*__ and __*sizes.exe*__ but not the Perl-based test suite (at least currently)
* [mc-crusher](https://github.com/memcached/mc-crusher) test is also performed.
* Maximum connection limit concurrency testing is also performed using [libMemcached](https://libmemcached.org/libMemcached.html)'s **memcslap**.
* **NOTE:** Since Perl-based test suite is not executed, test is lacking on some areas. **Use at your own risk!**

## Third-party OSS
<table>
    <tr>
        <th>OSS</th>
        <th>Purpose</th>
    </tr>
    <tr>
        <td><a href="https://www.freebsd.org/">freebsd</a></td>
        <td><a href="https://github.com/freebsd/freebsd/blob/master/contrib/file/src/getline.c">getline</a> and <a href="https://github.com/freebsd/freebsd/blob/master/lib/libc/stdlib/getsubopt.c">getsubopt</a> implementation
            <br>
        </td>
    </tr>
    <tr>
        <td><a href="https://github.com/curl/curl-for-win">curl-for-win</a></td>
        <td>Scripts only for cross-build on Linux build hosts to build reproducible binaries</td>
    </tr>
    <tr>
        <td><a href="http://syslog-win32.sourceforge.net/">syslog-win32</a></td>
        <td>Used only in debugging especially when executing multi-process testapp.exe. This is disabled by default.</td>
    </tr>
</table>

## Implementation Notes for Devs

* **-Werror** is intentionally disabled for Windows target but this doesn't mean warnings were not or won't be checked! In Windows host build, a lot of warnings are reported for memcached base code. Some of warnings are **-Wimplicit-fallthrough** and **-Wsign-compare**. These warnings can be ignored and fixing will just cause a lot of merge conflicts with the base code. In Linux host build, a lot of warnings are reported for the new/modified codes mostly for the heavy use of **#include_next (#include_next is a GCC extension)**. These warnings can also be ignored unless other method will be used in porting without much code changes with base code's header inclusions (e.g. #ifdefs, #ifndefs, moving includes).
* __*pipe*__ is implementated as libevent's __*evutil_socketpair*__ because Windows' anonymous pipes won't work with libevent which is used in memcached.
* *Reading/writing* from/to __*pipe*__ **MUST** use __*pipe_read/pipe_write*__ instead of __*read/write*__, otherwise runtime/logic error occurs. In non-Windows build, it's just __*read/write*__.
* *Reading/writing* from/to **socket** **MUST** use __*sock_read/sock_write*__ instead of __*read/write*__, otherwise runtime/logic error occurs. In non-Windows build, it's just __*read/write*__.
* *Closing* **socket** **MUST** use __*sock_close*__ instead of __*close*__, otherwise runtime/logic error occurs. In non-Windows build, it's just __*close*__.
* In dealing with **TLS**, WinSSL_* APIs (e.g. **WinSSL_accept**) **MUST** be used. It just directly calls OpenSSL/BoringSSL APIs counterparts with the setting of errno after the call in case of error. In Windows, OpenSSL/BoringSSL does not automatically set the errno (e.g. **EWOULDBLOCK**) since Windows has its own **WSAGetLastError**. **BUT**, memcached is using the errno (e.g. to retry read).
* Since no __*fork*__ in Windows, **-d/daemon** is implemented as running the same command line in background and **MEMCACHE_DAEMON_ENV** is set so that child/background/daemon process won't repeat the sequence. Main memcached process exits immediately after processing the **-d** option. User can use Windows' service manager as alternative.
* Most importanly, **socket fds** **CAN'T** be directly used as **conns index** in Windows! Socket fd value can even start beyond **1000**! __*conn_list*__ handles this.
* If needs more details regarding Windows-specific changes/implementation, **mingw/src/*** files have header comments for reference.

## Debugging Notes for Devs

* Debugging with an IDE (e.g. **Code::Blocks IDE**) can make life easier especially step-tracing
* To easily debug daemon/testapp background processes, just define **UNISTD_API_LOG** in __*mingw/src/mingw_unistd.c*__. This will create new console for every process created.
* To use syslog logging especially for multi-process/daemon debugging, just define **MINGW_USE_SYSLOG** in __*mingw/include/mingw_logger.h*__. Any Windows-based syslog server can be used as viewer.

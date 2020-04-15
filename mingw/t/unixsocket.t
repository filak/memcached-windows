#!/usr/bin/perl

use strict;
use Test::More;
use FindBin qw($Bin);
use lib "$Bin/lib";
use MemcachedTest;

my $filename = "tmp/memcachetest$$";

if (supports_unix_socket()) {
    plan tests => 3;

    my $server = new_memcached("-s $filename");
    my $sock = $server->sock;
    if(!windows_test()) {
        ok(-S $filename, "creating unix domain socket $filename");
    } else {
        # Windows implements unix socket file as NTFS reparse point. It looks
        # like a regular file for users.
        ok(-e $filename, "creating unix domain socket $filename");
    }

    # set foo (and should get it)
    print $sock "set foo 0 0 6\r\nfooval\r\n";

    is(scalar <$sock>, "STORED\r\n", "stored foo");
    mem_get_is($sock, "foo", "fooval");

    unlink($filename);

    ## Just some basic stuff for now...
} elsif (!$ENV{WINE_TEST}) {
    plan tests => 1;

    eval {
        my $server = new_memcached("-s $filename");
    };
    ok($@, "Died connecting to unsupported unix socket.");
} else {
    plan skip_all => 'Skipping unix socket tests on wine';
    exit 0;
}

#!/usr/bin/perl
use strict;
use FindBin qw($RealBin);
our @files;

BEGIN {
    my $top_srcdir = "$RealBin/../..";
    chdir "$top_srcdir" or die;
    unless (-d "$top_srcdir/.git") {
        use Test::More;
        plan skip_all => "Skipping test because this does not appear to be a memcached git working directory";
        exit 0;
    }

    my @exempted = qw(Makefile.am ChangeLog doc/Makefile.am README README.md compile_commands.json);
    push(@exempted, glob("doc/*.xml"));
    push(@exempted, glob("doc/*.full"));
    push(@exempted, glob("doc/xml2rfc/*.xsl"));
    push(@exempted, glob("doc/xml2rfc/*.dtd"));
    push(@exempted, glob("m4/*backport*m4"));
    push(@exempted, glob("*.orig"));
    push(@exempted, glob(".*.swp"));
    push(@exempted, glob("*.exe"));
    push(@exempted, glob("mingw/**"));              # Exempt Code::Blocks project files
    push(@exempted, glob("mingw/bin/**/**"));       # Exempt Code::Blocks built files
    push(@exempted, glob("mingw/build/*.patch"));   # Exempt mingw/build patch files
    my $mingw_build_dir = "mingw/build";            # Exclude mingw/build files from git result
    my %exempted_hash = map { $_ => 1 } @exempted;

    my @stuff = split /\0/, `git ls-files -z -c -m -o --exclude-standard --exclude=$mingw_build_dir`;
    @files = grep { ! $exempted_hash{$_} } @stuff;

    # We won't find any files if git isn't installed.  If git isn't
    # installed, they're probably not doing any useful development, or
    # at the very least am will clean up whitespace when we receive
    # their patch.
    unless (@files) {
        use Test::More;
        plan skip_all => "Skipping tests probably because you don't have git.";
        exit 0;
    }
}

use Test::More tests => scalar(@files);

foreach my $f (@files) {
    open(my $fh, $f) or die "Cannot open file $f: $!";
    my $before = do { local $/; <$fh>; };
    close ($fh);
    my $after = $before;
    $after =~ s/\t/    /g;
    $after =~ s/ +$//mg;
    $after .= "\n" unless $after =~ /\n$/;
    ok ($after eq $before, "$f (see devtools/clean-whitespace.pl)");
}
